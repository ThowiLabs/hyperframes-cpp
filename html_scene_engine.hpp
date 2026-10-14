#pragma once
#include <litehtml.h>
#include <fstream>
#include <iterator>
#include <memory>
#include <string>
#include <vector>

#include "html_litehtml.hpp"

// Native HTML/CSS rendering mode: each scene is laid out by litehtml and
// rendered by the built-in font/pixel backend, once at initialization.
// The timeline selects scenes; GSAP/JS expressions are NOT executed.
// deuda-tecnica: cached snapshots do not animate arbitrary DOM properties;
// add CSS transforms/GSAP keyframe evaluation with a bounded DOM model.
class HtmlSceneEngine {
 std::vector<std::vector<unsigned char>> frames_;
 double duration_=0;
 static std::string read(const std::string& file){
  std::ifstream f(file,std::ios::binary);
  if(!f)throw std::runtime_error("Could not open HTML source");
  std::string html((std::istreambuf_iterator<char>(f)),std::istreambuf_iterator<char>());
  if(html.size()>4*1024*1024)throw std::runtime_error("HTML exceeds 4 MB");
  return html;
 }
 public:
 HtmlSceneEngine(const std::string& file,const std::vector<hf::Scene>& scenes){
  static FontPainter font;
  const auto html=read(file);
  duration_=scenes.back().start+scenes.back().duration;
  if(scenes.size()>64)throw std::runtime_error("Too many scenes for HTML raster cache");
  frames_.reserve(scenes.size());
  for(size_t i=0;i<scenes.size();i++){
   std::fill(pixels.begin(),pixels.end(),0);
   htmlnative::Canvas container(W,H,font,pixels);
   auto selector=".scene-"+std::to_string(i+1);
   const std::string rule=".scene {display:none !important} "+selector+
                          "{display:block !important}";
   auto doc=litehtml::document::createFromString(
      html,&container,litehtml::master_css,rule);
   if(!doc)throw std::runtime_error("litehtml failed to parse HTML");
   doc->render(W);
   doc->draw(0,0,0,nullptr);
   if(container.textCalls()==0)throw std::runtime_error("No text in HTML scene "+selector);
   frames_.push_back(pixels);
  }
 }
 void frame(int fr,int fps,const std::vector<hf::Scene>& scenes){
  const double time=double(fr)/fps;
  size_t index=0;
  while(index+1<scenes.size() && time>=scenes[index+1].start)++index;
  const double local=time-scenes[index].start;
  const auto& current=frames_[index];
  // Slide/wipe between each HTML CSS snapshot, preserving scene timing.
  if(local<.4 && index>0){
   const auto& previous=frames_[index-1];
   const int split=int(W*std::clamp(local/.4,0.,1.));
   for(int y=0;y<H;y++){
    auto row=std::size_t(y)*W*3;
    std::copy(current.begin()+row,current.begin()+row+std::size_t(split)*3,pixels.begin()+row);
    std::copy(previous.begin()+row+std::size_t(split)*3,previous.begin()+row+std::size_t(W)*3,
              pixels.begin()+row+std::size_t(split)*3);
   }
  }else pixels=current;
  const int progress=int(W*time/duration_);
  for(int y=H-7;y<H;y++)for(int x=0;x<progress;x++){
   const auto i=(std::size_t(y)*W+x)*3;
   pixels[i]=105; pixels[i+1]=214; pixels[i+2]=255;
  }
 }
};
