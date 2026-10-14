#pragma once
#include <algorithm>
#include <cctype>
#include <stdexcept>
#include <string>

// Narrow CSS numeric-property reader. This is NOT a CSS layout engine.
// A full CSS implementation (litehtml) is a separate planned integration.
namespace css {
inline std::string trim(const std::string& s){
 const auto a=s.find_first_not_of(" \t\r\n");
 if(a==std::string::npos)return "";
 const auto b=s.find_last_not_of(" \t\r\n");
 return s.substr(a,b-a+1);
}
inline std::string stylesheet(const std::string& doc){
 const auto a=doc.find("<style>");
 if(a==std::string::npos)return "";
 const auto b=doc.find("</style>",a+7);
 if(b==std::string::npos)throw std::invalid_argument("Unclosed CSS style element");
 return doc.substr(a+7,b-a-7);
}
inline int pixels(const std::string& css,const std::string& selector,
                  const std::string& property,int fallback,int maximum=800){
 std::size_t pos=0;int result=fallback;
 while(pos<css.size()){
  auto open=css.find('{',pos);
  if(open==std::string::npos)break;
  auto close=css.find('}',open+1);
  if(close==std::string::npos)throw std::invalid_argument("Unclosed CSS rule");
  auto sel=trim(css.substr(pos,open-pos));
  // Exact selector match; complex selectors and media queries are not supported.
  if(sel==selector){
   const auto rules=css.substr(open+1,close-open-1);
   std::size_t offset=0;
   while(offset<rules.size()){
    auto semicolon=rules.find(';',offset);
    if(semicolon==std::string::npos)semicolon=rules.size();
    auto item=rules.substr(offset,semicolon-offset);
    auto colon=item.find(':');
    if(colon!=std::string::npos&&trim(item.substr(0,colon))==property){
     auto value=trim(item.substr(colon+1));
     if(value.size()>=3&&value.compare(value.size()-2,2,"px")==0){
      auto number=value.substr(0,value.size()-2);
      if(!number.empty()&&number.size()<=5&&
         std::all_of(number.begin(),number.end(),
                     [](unsigned char c){return std::isdigit(c)!=0;})){
       const int candidate=std::stoi(number);
       if(candidate>=1&&candidate<=maximum)result=candidate;
      }
     }
    }
    offset=semicolon+1;
   }
  }
  pos=close+1;
 }
 return result;
}
}
