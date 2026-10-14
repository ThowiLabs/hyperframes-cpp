#pragma once
#include "browser_detection.hpp"
#include "vendor/browser/json.hpp"
#define STB_IMAGE_IMPLEMENTATION
#include "vendor/browser/stb_image.h"
#include <array>
#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <random>
#include <stdexcept>
#include <string>
#include <string_view>
#include <thread>
#include <vector>
#if defined(_WIN32)
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#else
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#endif

// Browser-backed compositor for trusted, local HTML compositions.
// Uses Chrome DevTools Protocol over localhost, with source assets resolved
// by the real browser. Only Chromium-family browsers are supported.
// Chromium executes JavaScript in a dedicated user profile; never load
// untrusted pages using these privileges. deuda-tecnica: sandbox this process.
namespace browser {
using json=nlohmann::json;
namespace fs=std::filesystem;
#if defined(_WIN32)
using Socket=SOCKET;
constexpr Socket invalid=INVALID_SOCKET;
inline void socketClose(Socket s){closesocket(s);}
class SocketSystem {
 WSADATA init_{};
 public:
 SocketSystem(){if(WSAStartup(MAKEWORD(2,2),&init_))throw std::runtime_error("WSAStartup failed");}
 ~SocketSystem(){WSACleanup();}
};
#else
using Socket=int;
constexpr Socket invalid=-1;
inline void socketClose(Socket s){close(s);}
class SocketSystem {};
#endif
inline void sendAll(Socket s,const std::string& data) {
 std::size_t n=0;
 while(n<data.size()){
  const int written=send(s,data.data()+n,static_cast<int>(std::min(size_t(65536),data.size()-n)),0);
  if(written<=0)throw std::runtime_error("Browser socket send failed");
  n+=static_cast<size_t>(written);
 }
}
inline Socket connectLocal(int port){
 Socket s=socket(AF_INET,SOCK_STREAM,0);
 if(s==invalid)throw std::runtime_error("Browser socket creation failed");
 sockaddr_in endpoint{};
 endpoint.sin_family=AF_INET;endpoint.sin_port=htons(static_cast<uint16_t>(port));
 endpoint.sin_addr.s_addr=htonl(INADDR_LOOPBACK);
#if defined(_WIN32)
 DWORD timeout=10000;
 setsockopt(s,SOL_SOCKET,SO_RCVTIMEO,reinterpret_cast<const char*>(&timeout),sizeof(timeout));
 setsockopt(s,SOL_SOCKET,SO_SNDTIMEO,reinterpret_cast<const char*>(&timeout),sizeof(timeout));
#else
 timeval timeout{10,0};
 setsockopt(s,SOL_SOCKET,SO_RCVTIMEO,&timeout,sizeof(timeout));
 setsockopt(s,SOL_SOCKET,SO_SNDTIMEO,&timeout,sizeof(timeout));
#endif
 if(connect(s,reinterpret_cast<sockaddr*>(&endpoint),sizeof(endpoint))<0){
  socketClose(s);throw std::runtime_error("Could not connect to browser DevTools");
 }
 return s;
}
inline std::string readHttp(int port,const std::string& path){
 auto s=connectLocal(port);
 try{
  sendAll(s,"GET "+path+" HTTP/1.1\r\nHost: 127.0.0.1\r\nConnection: close\r\n\r\n");
  std::string result;
  std::array<char,16384> buf{};
  while(true){
   auto n=recv(s,buf.data(),static_cast<int>(buf.size()),0);
   if(n<=0)break;
   result.append(buf.data(),static_cast<size_t>(n));
   if(result.size()>1024*1024)throw std::runtime_error("Unexpected DevTools HTTP response");
  }
  socketClose(s);
  auto pos=result.find("\r\n\r\n");
  if(pos==std::string::npos||result.find("200 OK")==std::string::npos)
   throw std::runtime_error("Invalid DevTools HTTP response");
  return result.substr(pos+4);
 }catch(...){socketClose(s);throw;}
}
inline std::vector<unsigned char> base64(const std::string& in){
 constexpr std::string_view chars="ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
 std::vector<unsigned char> result;result.reserve(in.size()*3/4);
 unsigned buffer=0;int bits=0;
 for(char c:in){
  if(c=='=')break;
  auto p=chars.find(c);
  if(p==std::string_view::npos)throw std::runtime_error("Invalid PNG base64");
  buffer=(buffer<<6)|static_cast<unsigned>(p);
  bits+=6;
  if(bits>=8){bits-=8;result.push_back(static_cast<unsigned char>((buffer>>bits)&255u));}
 }
 return result;
}
class BrowserProcess{
 fs::path profile_;
#if defined(_WIN32)
 PROCESS_INFORMATION proc_{};
#else
 pid_t pid_=-1;
#endif
 public:
 BrowserProcess(const std::string& executable) {
  const auto name=std::to_string(static_cast<unsigned long>(
       std::chrono::steady_clock::now().time_since_epoch().count()));
  profile_=fs::temp_directory_path()/("hfc-browser-"+name);
  fs::create_directories(profile_);
  std::error_code permissionsError;
  fs::permissions(profile_,fs::perms::owner_all,
                  fs::perm_options::replace,permissionsError);
  const std::vector<std::string> args={
     executable,"--headless=new","--no-first-run","--no-default-browser-check",
     "--disable-background-networking","--disable-extensions","--disable-dev-shm-usage",
     "--hide-scrollbars","--force-device-scale-factor=1","--remote-debugging-port=0",
     "--window-size=720,1280",
     "--user-data-dir="+profile_.string(),"about:blank"
  };
#if defined(_WIN32)
  std::string cmd;
  for(const auto& arg:args){
   if(!cmd.empty())cmd+=' ';
   cmd+='"'+arg+'"';
  }
  STARTUPINFOA info{};info.cb=sizeof(info);
  if(!CreateProcessA(executable.c_str(),cmd.data(),nullptr,nullptr,FALSE,
    CREATE_NO_WINDOW,nullptr,nullptr,&info,&proc_))
   throw std::runtime_error("Could not launch detected browser");
#else
  pid_=fork();
  if(pid_<0)throw std::runtime_error("Unable to fork browser");
  if(pid_==0){
   std::vector<char*> argv;argv.reserve(args.size()+2);
   for(const auto& a:args)argv.push_back(const_cast<char*>(a.c_str()));
   argv.push_back(nullptr);
   // Containers normally run as root; Chromium requires --no-sandbox there.
   if(geteuid()==0)argv.insert(argv.end()-1,const_cast<char*>("--no-sandbox"));
   execv(executable.c_str(),argv.data());
   _exit(127);
  }
#endif
 }
 int port() const{
  const auto path=profile_/"DevToolsActivePort";
  for(int i=0;i<120;i++){
   if(fs::exists(path)){
    std::ifstream f(path);int port=0;f>>port;
    if(port>1024&&port<65536)return port;
   }
   std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }
  throw std::runtime_error("Browser failed to expose DevTools port");
 }
 ~BrowserProcess(){
#if defined(_WIN32)
  if(proc_.hProcess){
   TerminateProcess(proc_.hProcess,0);
   WaitForSingleObject(proc_.hProcess,2000);
   CloseHandle(proc_.hThread);CloseHandle(proc_.hProcess);
  }
#else
  if(pid_>0){
   kill(pid_,SIGTERM);
   int status=0;waitpid(pid_,&status,0);
  }
#endif
  std::error_code ec;
  fs::remove_all(profile_,ec);
 }
};
class Devtools {
 SocketSystem init_;
 Socket fd_=invalid;
 std::string pending_;
 int messageId_=0;
 static std::string websocketPath(const std::string& url,int port) {
  const auto prefix="ws://127.0.0.1:"+std::to_string(port);
  const auto pos=url.find("/devtools/");
  if(pos==std::string::npos)throw std::runtime_error("Unsupported browser DevTools WebSocket URL");
  return url.substr(pos);
 }
 void recvMore(){
  std::array<char,32768> buf{};
  int n=recv(fd_,buf.data(),static_cast<int>(buf.size()),0);
  if(n<=0)throw std::runtime_error("Browser CDP disconnected");
  pending_.append(buf.data(),static_cast<size_t>(n));
  if(pending_.size()>24*1024*1024)throw std::runtime_error("Oversized CDP response");
 }
 std::string exact(std::size_t len){
  while(pending_.size()<len)recvMore();
  auto chunk=pending_.substr(0,len);
  pending_.erase(0,len);
  return chunk;
 }
 void sendFrame(const std::string& payload,uint8_t opcode=1){
  std::string frame;
  frame.push_back(static_cast<char>(0x80|opcode));
  auto n=payload.size();
  if(n<126)frame.push_back(static_cast<char>(0x80|n));
  else if(n<=65535){
   frame.push_back(static_cast<char>(0x80|126));
   frame.push_back(static_cast<char>((n>>8)&255));
   frame.push_back(static_cast<char>(n&255));
  }else{
   frame.push_back(static_cast<char>(0x80|127));
   for(int i=7;i>=0;i--)frame.push_back(static_cast<char>((uint64_t(n)>>(i*8))&255));
  }
  std::array<unsigned char,4> mask{};
  std::random_device rng;
  for(auto& b:mask){b=static_cast<unsigned char>(rng());frame.push_back(static_cast<char>(b));}
  for(std::size_t i=0;i<n;i++)frame.push_back(static_cast<char>(payload[i]^mask[i%4]));
  sendAll(fd_,frame);
 }
 std::string recvFrame(){
  for(;;){
   const auto hdr=exact(2);
   const auto b0=static_cast<unsigned char>(hdr[0]),b1=static_cast<unsigned char>(hdr[1]);
   unsigned long long length=b1&127;
   if(length==126){
    auto ex=exact(2);length=(static_cast<unsigned char>(ex[0])<<8)|static_cast<unsigned char>(ex[1]);
   }else if(length==127){
    auto ex=exact(8);length=0;
    for(unsigned char byte:ex)length=length*256+byte;
   }
   if(length>16*1024*1024)throw std::runtime_error("Unexpected oversized WebSocket response");
   std::array<unsigned char,4> mask{};
   if(b1&128){
    auto bytes=exact(4);
    for(int i=0;i<4;i++)mask[i]=static_cast<unsigned char>(bytes[i]);
   }
   std::string value=exact(static_cast<size_t>(length));
   if(b1&128)for(size_t i=0;i<value.size();i++)value[i]=char(static_cast<unsigned char>(value[i])^mask[i%4]);
   if((b0&15)==9){sendFrame(value,10);continue;}
   if((b0&15)==8)throw std::runtime_error("Browser closed WebSocket");
   if((b0&15)!=1 && (b0&15)!=0)continue;
   if((b0&128)==0)throw std::runtime_error("Fragmented WebSocket frames are not yet supported");
   return value;
  }
 }
 public:
 explicit Devtools(int port) {
  auto pages=json::parse(readHttp(port,"/json"));
  std::string url;
  for(auto& page:pages){
   if(page.value("type","")=="page"&&page.contains("webSocketDebuggerUrl")){
    url=page["webSocketDebuggerUrl"].get<std::string>();break;
   }
  }
  if(url.empty())throw std::runtime_error("No Chrome page target was created");
  fd_=connectLocal(port);
  auto path=websocketPath(url,port);
  sendAll(fd_,"GET "+path+" HTTP/1.1\r\nHost: 127.0.0.1:"+std::to_string(port)+
       "\r\nUpgrade: websocket\r\nConnection: Upgrade\r\nSec-WebSocket-Key: dGhlIHNhbXBsZSBub25jZQ==\r\nSec-WebSocket-Version: 13\r\n\r\n");
  while(pending_.find("\r\n\r\n")==std::string::npos)recvMore();
  auto pos=pending_.find("\r\n\r\n");
  if(pending_.find("HTTP/1.1 101 ")==std::string::npos)
   throw std::runtime_error("Browser WebSocket upgrade rejected: "+pending_.substr(0,420));
  pending_.erase(0,pos+4);
 }
 ~Devtools(){if(fd_!=invalid)socketClose(fd_);}
 json command(const std::string& method,const json& params=json::object()){
  const int id=++messageId_;
  sendFrame(json({{"id",id},{"method",method},{"params",params}}).dump());
  for(int events=0;events<2000;events++){
   auto message=json::parse(recvFrame());
   if(message.value("id",-1)!=id)continue;
   if(message.contains("error"))throw std::runtime_error(
     "Browser CDP error: "+message["error"].dump().substr(0,200));
   return message.value("result",json::object());
  }
  throw std::runtime_error("Too many browser events without a CDP response");
 }
};
class Capture{
 std::unique_ptr<BrowserProcess> child_;
 std::unique_ptr<Devtools> session_;
 int width_,height_;
 public:
 Capture(const std::string& filename,int width=720,int height=1280):width_(width),height_(height){
  auto executable=discover();
  if(executable.empty())throw std::runtime_error(
    "No Chromium-family browser detected. Install Chrome, Chromium, Edge, or set HFC_BROWSER to its executable.");
  std::cerr<<"Browser detected: "<<executable<<"\n";
  child_=std::make_unique<BrowserProcess>(executable);
  session_=std::make_unique<Devtools>(child_->port());
  session_->command("Page.enable");
  session_->command("Runtime.enable");
  session_->command("Emulation.setDeviceMetricsOverride",
     {{"width",width_},{"height",height_},{"deviceScaleFactor",1},{"mobile",false}});
  auto path=fs::absolute(filename).generic_u8string();
#if defined(_WIN32)
  if(path.size()>1&&path[1]==':')path="/"+path;
#endif
  std::string url="file://";
  for(unsigned char c:path){
   if(c==' '||c=='#'||c=='%'||c=='?'||c=='&'){
    constexpr char hex[]="0123456789ABCDEF";
    url+='%';url+=hex[c>>4];url+=hex[c&15];
   }else url+=static_cast<char>(c);
  }
  auto nav=session_->command("Page.navigate",{{"url",url}});
  if(nav.contains("errorText"))throw std::runtime_error("Could not navigate browser to HTML file");
  bool ready=false;
  for(int i=0;i<120;i++){
   auto res=session_->command("Runtime.evaluate",{
    {"expression","document.readyState === 'complete' && !!(window.__timelines && window.__timelines.main)"},
    {"returnByValue",true}});
   if(res.contains("result")&&res["result"].value("value",false)){ready=true;break;}
   std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }
  if(!ready)throw std::runtime_error(
    "Browser page did not expose window.__timelines.main. Check that GSAP assets exist next to HTML.");
 }
 void render(double seconds,std::vector<unsigned char>& bgr){
  if(seconds<0||!std::isfinite(seconds))throw std::invalid_argument("Invalid timeline time");
  auto code="(()=>{const t="+std::to_string(seconds)+
    "; const elements=[...document.querySelectorAll('.scene')];"+
    " let active=elements.findIndex(el=>{const start=Number(el.dataset.start||0);"+
    " const duration=Number(el.dataset.duration||0); return t>=start && t<start+duration;});"+
    " if(active<0)active=elements.length-1;"+
    " elements.forEach((el,i)=>{el.style.visibility=i===active?'visible':'hidden';});"+
    " window.__timelines.main.pause();window.__timelines.main.seek(t,false);return active})()";
  auto evaluated=session_->command("Runtime.evaluate",{{"expression",code},{"returnByValue",true}});
  if(evaluated.contains("exceptionDetails"))throw std::runtime_error("GSAP seek failed in browser");
  auto screenshot=session_->command("Page.captureScreenshot",{
    {"format","jpeg"},{"quality",93},{"fromSurface",true},{"captureBeyondViewport",false}});
  auto encoded=base64(screenshot.at("data").get<std::string>());
  int w=0,h=0,c=0;
  stbi_uc* pixels=stbi_load_from_memory(encoded.data(),static_cast<int>(encoded.size()),&w,&h,&c,3);
  if(!pixels)throw std::runtime_error("Unable to decode browser PNG screenshot");
  if(w!=width_||h!=height_){stbi_image_free(pixels);throw std::runtime_error(
       "Browser returned unexpected screenshot size "+std::to_string(w)+"x"+std::to_string(h));}
  bgr.resize(size_t(width_)*height_*3);
  for(size_t i=0;i<bgr.size();i+=3){
   bgr[i]=pixels[i+2];bgr[i+1]=pixels[i+1];bgr[i+2]=pixels[i];
  }
  stbi_image_free(pixels);
 }
};
}
