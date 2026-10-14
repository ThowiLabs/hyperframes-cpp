#include "../css_properties.hpp"
#include <cassert>
#include <iostream>
#include <stdexcept>
int main(){
 const auto body=std::string(
  "<style>.headline{font-size:68px;margin-top:50px;}"
  ".scene-2 .headline{font-size:76px}"
  ".headline{font-size:44px}</style>");
 const auto css=css::stylesheet(body);
 assert(css::pixels(css,".headline","font-size",20)==44);
 assert(css::pixels(css,".scene-2 .headline","font-size",20)==76);
 assert(css::pixels(css,".headline","margin-top",0)==50);
 assert(css::pixels(css,".headline","nonesuch",71)==71);
 assert(css::pixels(".x{width:100%;}",".x","width",44)==44);
 assert(css::pixels(".x{width:8000px;}",".x","width",44)==44);
 bool threw=false;
 try{(void)css::stylesheet("<style>.x{width:10px}");}
 catch(const std::invalid_argument&){threw=true;}
 assert(threw);
 std::cout<<"CSS property tests passed\n";
}
