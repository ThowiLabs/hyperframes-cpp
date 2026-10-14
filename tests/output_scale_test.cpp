#include "../output_scale.hpp"
#include <cassert>
#include <iostream>
#include <stdexcept>
#include <vector>
int main(){
 using namespace output;
 const auto a=parseSize("360x640");
 assert(a.width==360&&a.height==640);
 assert(parseSize("720x1280").width==720);
 assert(parseSize("1080x1920").height==1920);
 for(const auto& invalid:{"abc","1x2","0x0","720x720","1111x2000",
                          "360x641","2000x4000","-360x640","720X1280"}){
  bool rejected=false;
  try{(void)parseSize(invalid);}catch(const std::invalid_argument&){rejected=true;}
  assert(rejected);
 }
 std::vector<unsigned char> image{
   0,0,0,  255,0,0,
   0,255,0, 255,255,0
 };
 auto small=resize(image,2,2,1,1);
 assert(small.size()==3);
 assert(small[0]>=127&&small[0]<=128);
 assert(small[1]>=127&&small[1]<=128);
 assert(small[2]==0);
 auto original=resize(image,2,2,2,2);
 assert(original==image);
 bool threw=false;
 try{(void)resize(image,2,2,-1,2);}catch(const std::invalid_argument&){threw=true;}
 assert(threw);
 std::cout<<"Output scaling tests passed\n";
}
