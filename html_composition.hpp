#pragma once
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <fstream>
#include <iterator>
#include <stdexcept>
#include <string>
#include <vector>
namespace hf {
struct Scene {double start=0,duration=0; unsigned char r=130,g=210,b=220;std::string tag,sub,highlight,kind;std::vector<std::string> lines;};

inline std::string attr(const std::string& tag,const std::string& key){
 const auto pos=tag.find(key+"=\"");
 if(pos==std::string::npos)return "";
 const auto start=pos+key.size()+2,end=tag.find('"',start);
 if(end==std::string::npos)throw std::runtime_error("Atributo HTML mal formado");
 return tag.substr(start,end-start);
}
inline bool hasClass(const std::string& tag,const std::string& cls){
 const auto classes=" "+attr(tag,"class")+" ";
 return classes.find(" "+cls+" ")!=std::string::npos;
}
inline std::string getText(const std::string& fragment){
 std::string out;bool inside=false;
 for(char c:fragment){
  if(c=='<'){inside=true;continue;}
  if(c=='>'){inside=false;continue;}
  if(!inside)out+=c;
 }
 const std::pair<const char*,const char*> substitutions[]={{"&amp;","&"},{"&lt;","<"},{"&gt;",">"},{"&quot;","\""},{"&nbsp;"," "},{"&#39;","'"}};
 for(const auto& [from,to]:substitutions){
  size_t i=0;
  while((i=out.find(from,i))!=std::string::npos){out.replace(i,strlen(from),to);i+=strlen(to);}
 }
 size_t a=out.find_first_not_of(" \t\n\r");
 if(a==std::string::npos)return "";
 size_t b=out.find_last_not_of(" \t\n\r");
 return out.substr(a,b-a+1);
}
inline std::string divBody(const std::string& doc,size_t openingEnd){
 const auto first=openingEnd+1;auto pos=first;int depth=1;
 while(depth){
  auto op=doc.find("<div",pos),cl=doc.find("</div",pos);
  if(cl==std::string::npos)throw std::runtime_error("Elemento div sin cierre");
  if(op!=std::string::npos&&op<cl){++depth;pos=doc.find('>',op);if(pos==std::string::npos)throw std::runtime_error("Etiqueta incompleta");++pos;}
  else{--depth;if(!depth)return doc.substr(first,cl-first);pos=doc.find('>',cl);if(pos==std::string::npos)throw std::runtime_error("Cierre incompleto");++pos;}
 }
 return "";
}
inline std::vector<std::string> textByClass(const std::string& body,const std::string& cls){
 std::vector<std::string> out;size_t at=0;
 while((at=body.find("<div",at))!=std::string::npos){
  const auto end=body.find('>',at);
  if(end==std::string::npos||end-at>2048)throw std::runtime_error("Div HTML incorrecto");
  if(hasClass(body.substr(at,end-at+1),cls))out.push_back(getText(divBody(body,end)));
  at=end+1;
 }
 return out;
}
inline std::string first(const std::string& body,const std::string& cls){
 auto v=textByClass(body,cls);return v.empty()?"":v.front();
}
inline int nibble(char c){
 if(c>='0'&&c<='9')return c-'0';
 if(c>='a'&&c<='f')return c-'a'+10;
 if(c>='A'&&c<='F')return c-'A'+10;
 throw std::runtime_error("Color hexadecimal incorrecto");
}

inline std::vector<Scene> load(const std::string& filename){
 std::ifstream f(filename,std::ios::binary);
 if(!f)throw std::runtime_error("No se encuentra el HTML: "+filename);
 const std::string doc((std::istreambuf_iterator<char>(f)),std::istreambuf_iterator<char>());
 if(doc.size()>4*1024*1024)throw std::runtime_error("HTML demasiado grande (maximo 4 MB)");
 std::vector<Scene> scenes;size_t at=0;
 while((at=doc.find("<section",at))!=std::string::npos){
  const auto begin=doc.find('>',at);
  if(begin==std::string::npos)throw std::runtime_error("Seccion HTML mal formada");
  const auto head=doc.substr(at,begin-at+1);
  if(!hasClass(head,"scene")){at=begin+1;continue;}
  const auto end=doc.find("</section>",begin);
  if(end==std::string::npos)throw std::runtime_error("Falta cierre de section");
  if(scenes.size()>=64)throw std::runtime_error("Mas de 64 escenas");
  Scene s;
  try{s.start=std::stod(attr(head,"data-start"));s.duration=std::stod(attr(head,"data-duration"));}
  catch(...){throw std::runtime_error("data-start o data-duration incorrecto");}
  if(!std::isfinite(s.start)||!std::isfinite(s.duration)||s.start<0||s.duration<=0||s.duration>300)
   throw std::runtime_error("Inicio o duracion invalida");
  const double expected=scenes.empty()?0:scenes.back().start+scenes.back().duration;
  if(std::abs(s.start-expected)>.12)throw std::runtime_error("Las escenas no son contiguas");
  auto style=attr(head,"style");size_t pos=style.find("--a:");
  if(pos!=std::string::npos){
   pos=style.find('#',pos);
   if(pos==std::string::npos||pos+7>style.size())throw std::runtime_error("Color de escena invalido");
   s.r=static_cast<unsigned char>((nibble(style[pos+1])<<4)|nibble(style[pos+2]));
   s.g=static_cast<unsigned char>((nibble(style[pos+3])<<4)|nibble(style[pos+4]));
   s.b=static_cast<unsigned char>((nibble(style[pos+5])<<4)|nibble(style[pos+6]));
  }
  const auto body=doc.substr(begin+1,end-begin-1);
  s.lines=textByClass(body,"wordline");
  s.tag=first(body,"kicker");
  s.sub=first(body,"sub");
  s.highlight=first(body,"barlabel");
  for(const auto& name:{"meme-face","dateart","symbols","ai-art","debate","conclusion"})
   if(body.find(std::string("class=\"")+name)!=std::string::npos){s.kind=name;break;}
  if(s.lines.empty()||s.lines.size()>8)throw std::runtime_error("Escena requiere entre 1 y 8 lineas wordline");
  scenes.push_back(std::move(s));at=end+10;
 }
 if(scenes.empty())throw std::runtime_error("HTML no contiene section class=scene");
 if(scenes.back().start+scenes.back().duration>300)throw std::runtime_error("Video mayor de cinco minutos");
 return scenes;
}

// Convierte letras UTF-8 espanolas al alfabeto bitmap. Conserva el texto
// original de las etiquetas importadas; la transliteracion es visual.
inline std::string bitmap(const std::string& s){
 std::string out;out.reserve(s.size());
 for(size_t i=0;i<s.size();++i){
  unsigned char ch=static_cast<unsigned char>(s[i]);
  if(ch<128){
   if(ch>='a'&&ch<='z')out+=char(ch-32);
   else if((ch>='A'&&ch<='Z')||(ch>='0'&&ch<='9')||ch==' '||ch=='?'||ch=='.')out+=char(ch);
   else if(ch=='-'||ch=='/'||ch==':'||ch=='!')out+=' ';
  }else if(i+1<s.size()){
   unsigned char b=static_cast<unsigned char>(s[i+1]);
   if(ch==0xc3){
    if(b==0x91||b==0xb1)out+='N';
    else if(b==0x81||b==0xa1||b==0x80||b==0xa0)out+='A';
    else if(b==0x89||b==0xa9)out+='E';
    else if(b==0x8d||b==0xad)out+='I';
    else if(b==0x93||b==0xb3)out+='O';
    else if(b==0x9a||b==0xba||b==0x9c||b==0xbc)out+='U';
    ++i;
   }else if(ch==0xc2){if(b==0xbf)out+='?';++i;}
  }
 }
 return out;
}
} // namespace hf
