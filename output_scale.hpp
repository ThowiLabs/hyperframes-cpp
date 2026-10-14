#pragma once
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <stdexcept>
#include <string>
#include <vector>

// The rendering layout still uses a 720x1280 design canvas.
// Output pixel dimensions may be scaled to another 9:16 resolution.
// deuda-tecnica: raster scaling does not recalculate CSS layout; use a true
// responsive layout engine when supporting arbitrary aspect ratios.
namespace output {
struct Size {int width=720,height=1280;};

inline Size parseSize(const std::string& value){
 auto pos=value.find('x');
 if(pos==std::string::npos || value.find('x',pos+1)!=std::string::npos)
  throw std::invalid_argument("Resolution must be WIDTHxHEIGHT");
 auto part=[](const std::string& s)->int{
  if(s.empty()||s.size()>5)return -1;
  for(char c:s)if(c<'0'||c>'9')return -1;
  try{return std::stoi(s);}catch(...){return -1;}
 };
 Size result{part(value.substr(0,pos)),part(value.substr(pos+1))};
 if(result.width<180||result.width>1080||result.height<320||result.height>1920||
    result.width%2 || result.height%2 ||
    result.width*16!=result.height*9)
  throw std::invalid_argument("Resolution must be even, 9:16, between 180x320 and 1080x1920");
 return result;
}

struct AxisSample {int first,second,weight;};
inline std::vector<AxisSample> axis(int oldSize,int newSize){
 std::vector<AxisSample> samples;
 samples.reserve(newSize);
 for(int i=0;i<newSize;i++){
  double x=(i+.5)*double(oldSize)/newSize-.5;
  int a=std::max(0,int(std::floor(x))),b=std::min(oldSize-1,a+1);
  if(x<0.){a=0;b=0;x=0.;}
  samples.push_back({a,b,std::clamp(int((x-a)*256+.5),0,256)});
 }
 return samples;
}

// Bilinear interpolation in BGR byte order. Dimensions are checked by caller.
inline std::vector<unsigned char> resize(const std::vector<unsigned char>& input,
 int sourceW,int sourceH,int targetW,int targetH){
 if(sourceW<=0||sourceH<=0||targetW<=0||targetH<=0 ||
    input.size()!=std::size_t(sourceW)*sourceH*3)
  throw std::invalid_argument("Invalid source buffer or canvas size");
 if(sourceW==targetW&&sourceH==targetH)return input;
 auto xx=axis(sourceW,targetW),yy=axis(sourceH,targetH);
 std::vector<unsigned char> out(std::size_t(targetW)*targetH*3);
 for(int y=0;y<targetH;y++){
  const auto& v=yy[y];
  for(int x=0;x<targetW;x++){
   const auto& u=xx[x];
   const std::size_t a=(std::size_t(v.first)*sourceW+u.first)*3;
   const std::size_t b=(std::size_t(v.first)*sourceW+u.second)*3;
   const std::size_t c=(std::size_t(v.second)*sourceW+u.first)*3;
   const std::size_t d=(std::size_t(v.second)*sourceW+u.second)*3;
   auto outIndex=(std::size_t(y)*targetW+x)*3;
   for(int k=0;k<3;k++){
    const int upper=int(input[a+k])*(256-u.weight)+int(input[b+k])*u.weight;
    const int lower=int(input[c+k])*(256-u.weight)+int(input[d+k])*u.weight;
    out[outIndex+k]=static_cast<unsigned char>(
       (upper*(256-v.weight)+lower*v.weight+32768)>>16);
   }
  }
 }
 return out;
}
}
