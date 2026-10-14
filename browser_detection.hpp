#pragma once
#include <cstdlib>
#include <filesystem>
#include <string>
#include <vector>

namespace browser {
namespace fs=std::filesystem;
inline std::string discover() {
 auto check=[](const fs::path& path)->std::string {
  std::error_code err;
  if(fs::is_regular_file(path,err))return path.string();
  return {};
 };
 if(const char* override=std::getenv("HFC_BROWSER");override&&*override){
  return check(fs::path(override));
 }
#if defined(_WIN32)
 std::vector<std::string> names={"chrome.exe","msedge.exe","chromium.exe"};
 const char* dirs[]={
  std::getenv("PROGRAMFILES"),std::getenv("PROGRAMFILES(X86)"),
  std::getenv("LOCALAPPDATA")
 };
 for(const auto* dir:dirs)if(dir){
  for(const auto& suffix:{
    "Google/Chrome/Application/chrome.exe",
    "Microsoft/Edge/Application/msedge.exe",
    "Chromium/Application/chrome.exe"
  }){
   auto result=check(fs::path(dir)/suffix);
   if(!result.empty())return result;
  }
 }
 constexpr char separator=';';
#else
 std::vector<std::string> names={"chromium","chromium-browser","google-chrome-stable","google-chrome",
                                 "microsoft-edge","microsoft-edge-stable","brave-browser"};
 constexpr char separator=':';
#endif
 const auto env=std::getenv("PATH");
 std::string path=env?env:"";
 std::size_t start=0;
 while(start<=path.size()){
  auto end=path.find(separator,start);
  if(end==std::string::npos)end=path.size();
  if(end>start){
   for(const auto& exe:names){
    auto candidate=check(fs::path(path.substr(start,end-start))/exe);
    if(!candidate.empty())return candidate;
   }
  }
  if(end==path.size())break;
  start=end+1;
 }
 return {};
}
}
