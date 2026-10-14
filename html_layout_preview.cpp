#include <algorithm>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <iterator>
#include <stdexcept>
#include <string>
#include <vector>

constexpr int W=720,H=1280;
struct RGB{unsigned char r,g,b;};
std::vector<unsigned char> pixels(std::size_t(W)*H*3);
#include "font_renderer.hpp"
#include "html_litehtml.hpp"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#if defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
#endif
#include "vendor/stb/stb_image_write.h"
#if defined(__GNUC__)
#pragma GCC diagnostic pop
#endif

int main(int argc,char** argv) {
 try {
  if(argc!=3 && argc!=4){std::cerr<<"Usage: html-layout-preview input.html output.png [scene-number]\n";return 2;}
  std::ifstream input(argv[1],std::ios::binary);
  if(!input)throw std::runtime_error("Unable to read input HTML");
  std::string html((std::istreambuf_iterator<char>(input)),std::istreambuf_iterator<char>());
  if(html.size()>4*1024*1024)throw std::runtime_error("HTML file exceeds 4MB");
  std::fill(pixels.begin(),pixels.end(),0);
  FontPainter fonts;
  htmlnative::Canvas canvas(W,H,fonts,pixels);
  std::string overrides;
  if(argc==4){
   auto index=std::stoi(argv[3]);
   if(index<1||index>64)throw std::runtime_error("Scene number must be 1..64");
   overrides=".scene { display:none !important } .scene-"+std::to_string(index)+
             " { display:block !important }";
  }
  auto document=litehtml::document::createFromString(html,&canvas,litehtml::master_css,overrides);
  if(!document)throw std::runtime_error("litehtml failed to parse input");
  auto calculatedHeight=document->render(W);
  document->draw(0,0,0,nullptr);
  std::vector<unsigned char> rgb(pixels.size());
  for(size_t i=0;i<rgb.size();i+=3){
   rgb[i]=pixels[i+2];rgb[i+1]=pixels[i+1];rgb[i+2]=pixels[i];
  }
  if(!stbi_write_png(argv[2],W,H,3,rgb.data(),W*3))
   throw std::runtime_error("PNG output failed");
  std::cout<<"HTML/CSS rendered: "<<W<<"x"<<H
           <<" computed_height="<<calculatedHeight.value()
           <<" text_calls="<<canvas.textCalls()<<" fills="<<canvas.fillCalls()<<"\n";
  if(canvas.textCalls()==0 || canvas.fillCalls()==0)
   throw std::runtime_error("No text or backgrounds drawn: unsupported input?");
  return 0;
 }catch(const std::exception& e) {
  std::cerr<<"Layout preview error: "<<e.what()<<"\n";return 1;
 }
}
