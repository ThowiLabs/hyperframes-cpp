#pragma once
#include <map>
#define STB_TRUETYPE_IMPLEMENTATION
#include "vendor/stb/stb_truetype.h"
#include "font_embedded.hpp"

// DejaVu Sans original (se declara en el CSS). Fuente incrustada en binario,
// con rasterización alpha TrueType sin bibliotecas compartidas.
class FontPainter {
 struct Glyph {
  int width=0,height=0,offsetX=0,offsetY=0,advance=0;
  std::vector<unsigned char> alpha;
 };
 stbtt_fontinfo fonts_[2]{};
 std::map<uint64_t,Glyph> cache_;
 static uint32_t next(const std::string& str,size_t& i) {
  unsigned char a=static_cast<unsigned char>(str[i++]);
  if(a<128)return a;
  int needed=(a>=0xf0 && a<=0xf4)?3:(a>=0xe0&&a<=0xef)?2:(a>=0xc2&&a<=0xdf)?1:0;
  if(!needed)return 0xfffd;
  uint32_t val=a & (needed==3?7:needed==2?15:31);
  for(int j=0;j<needed;j++){
   if(i>=str.size())return 0xfffd;
   unsigned char b=static_cast<unsigned char>(str[i]);
   if((b&0xc0)!=0x80)return 0xfffd;
   ++i;val=(val<<6)|(b&63);
  }
  return val;
 }
 const Glyph& glyph(int font,int size,uint32_t cp) {
  const uint64_t key=(uint64_t(font)<<56)|(uint64_t(size)<<32)|cp;
  auto old=cache_.find(key);if(old!=cache_.end())return old->second;
  auto& f=fonts_[font];
  auto scale=stbtt_ScaleForPixelHeight(&f,float(size));
  Glyph g;
  int advance, lsb;
  stbtt_GetCodepointHMetrics(&f,int(cp),&advance,&lsb);
  g.advance=int(std::round(advance*scale));
  unsigned char* bitmap=stbtt_GetCodepointBitmap(&f,scale,scale,int(cp),&g.width,&g.height,&g.offsetX,&g.offsetY);
  if(bitmap && g.width>0 && g.height>0 && g.width*g.height<1000000){
   g.alpha.assign(bitmap,bitmap+size_t(g.width)*g.height);
  }
  if(bitmap)stbtt_FreeBitmap(bitmap,nullptr);
  auto [it,inserted]=cache_.emplace(key,std::move(g));
  return it->second;
 }
 public:
 FontPainter(){
  if(!stbtt_InitFont(&fonts_[0],bundled::regular,0)||
     !stbtt_InitFont(&fonts_[1],bundled::bold,0))
    throw std::runtime_error("Error al cargar fuentes incrustadas");
 }
 float measure(const std::string& str,int px,bool isBold) {
  const int font=isBold?1:0;float width=0;size_t i=0;uint32_t prev=0;
  const auto scale=stbtt_ScaleForPixelHeight(&fonts_[font],float(px));
  while(i<str.size()){
   uint32_t cp=next(str,i);
   if(prev)width+=stbtt_GetCodepointKernAdvance(&fonts_[font],int(prev),int(cp))*scale;
   width+=glyph(font,px,cp).advance;prev=cp;
  }
  return width;
 }
 // y = borde superior de la línea, igual que top CSS
 void draw(int x,int y,const std::string& str,int px,RGB color,bool bold=true,int maxWidth=-1,float opacity=1.f){
  if(str.empty())return;
  if(maxWidth>0){
   while(px>8 && measure(str,px,bold)>maxWidth)px--;
  }
  const int font=bold?1:0;
  int asc,descent,gap;stbtt_GetFontVMetrics(&fonts_[font],&asc,&descent,&gap);
  auto scale=stbtt_ScaleForPixelHeight(&fonts_[font],float(px));
  const int baseline=y+int(asc*scale+.5);
  float pen=float(x);uint32_t prev=0;size_t i=0;
  while(i<str.size()){
   uint32_t cp=next(str,i);
   if(prev)pen+=stbtt_GetCodepointKernAdvance(&fonts_[font],int(prev),int(cp))*scale;
   const auto& gl=glyph(font,px,cp);
   int left=int(std::round(pen))+gl.offsetX,top=baseline+gl.offsetY;
   for(int gy=0;gy<gl.height;gy++){
    int py=top+gy;if(py<0||py>=H)continue;
    for(int gx=0;gx<gl.width;gx++){
     int px2=left+gx;if(px2<0||px2>=W)continue;
     const auto a=unsigned(std::clamp(gl.alpha[size_t(gy)*gl.width+gx]*opacity,0.f,255.f));
     if(!a)continue;
     auto off=(size_t(py)*W+px2)*3;
     pixels[off]=static_cast<unsigned char>((pixels[off]*(255-a)+color.b*a+127)/255);
     pixels[off+1]=static_cast<unsigned char>((pixels[off+1]*(255-a)+color.g*a+127)/255);
     pixels[off+2]=static_cast<unsigned char>((pixels[off+2]*(255-a)+color.r*a+127)/255);
    }
   }
   pen+=gl.advance;prev=cp;
  }
 }
};
