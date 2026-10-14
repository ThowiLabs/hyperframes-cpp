#pragma once
#include <litehtml.h>
#include <cmath>
#include <fstream>
#include <iterator>
#include <memory>
#include <string>
#include <vector>

// A bounded litehtml document_container over the existing C++ raster buffer.
// Images, external CSS and scripts are intentionally not loaded/executed.
// Draw operations produce a 720x1280 BGR image owned by the caller.
namespace htmlnative {
struct FontData {int px=18; bool bold=false;};
class Canvas : public litehtml::document_container {
 int w_,h_;
 FontPainter& painter_;
 std::vector<unsigned char>& image_;
 int textCalls_=0,fillCalls_=0;
 static int n(litehtml::pixel_t x){return static_cast<int>(std::round(x.value()));}
 void blend(int x,int y,int r,int g,int b,int a){
  if(x<0||y<0||x>=w_||y>=h_||a<=0)return;
  auto offset=(std::size_t(y)*w_+x)*3;
  auto alpha=std::min(a,255);
  image_[offset]  =static_cast<unsigned char>((image_[offset]*(255-alpha)+b*alpha+127)/255);
  image_[offset+1]=static_cast<unsigned char>((image_[offset+1]*(255-alpha)+g*alpha+127)/255);
  image_[offset+2]=static_cast<unsigned char>((image_[offset+2]*(255-alpha)+r*alpha+127)/255);
 }
 void fill(int x,int y,int w,int h,litehtml::web_color c){
  if(w<=0||h<=0)return;
  for(int yy=std::max(0,y);yy<std::min(h_,y+h);yy++)
   for(int xx=std::max(0,x);xx<std::min(w_,x+w);xx++)
    blend(xx,yy,c.red,c.green,c.blue,c.alpha);
  ++fillCalls_;
 }
 public:
 Canvas(int w,int h,FontPainter& f,std::vector<unsigned char>& image)
 :w_(w),h_(h),painter_(f),image_(image){}
 int textCalls() const{return textCalls_;}
 int fillCalls() const{return fillCalls_;}
 litehtml::uint_ptr create_font(const litehtml::font_description& d,const litehtml::document*,
                               litehtml::font_metrics* fm) override {
  auto font=std::make_unique<FontData>();
  font->px=std::clamp(n(d.size),8,256);font->bold=d.weight>=600;
  fm->font_size=font->px;
  fm->height=font->px;
  fm->ascent=litehtml::pixel_t(font->px*.78f);
  fm->descent=litehtml::pixel_t(font->px*.22f);
  fm->x_height=litehtml::pixel_t(font->px*.5f);
  fm->ch_width=litehtml::pixel_t(font->px*.53f);
  fm->sub_shift=litehtml::pixel_t(0);
  fm->super_shift=litehtml::pixel_t(0);
  fm->draw_spaces=false;
  return reinterpret_cast<litehtml::uint_ptr>(font.release());
 }
 void delete_font(litehtml::uint_ptr font) override{
  delete reinterpret_cast<FontData*>(font);
 }
 litehtml::pixel_t text_width(const char* value,litehtml::uint_ptr ptr) override {
  auto* f=reinterpret_cast<FontData*>(ptr);
  return litehtml::pixel_t(static_cast<float>(painter_.measure(value?value:"",f->px,f->bold)));
 }
 void draw_text(litehtml::uint_ptr,const char* str,litehtml::uint_ptr ptr,litehtml::web_color c,
                const litehtml::position& pos) override{
  if(!str || !*str)return;
  auto* font=reinterpret_cast<FontData*>(ptr);
  painter_.draw(n(pos.x),n(pos.y),str,font->px,{c.red,c.green,c.blue},font->bold,
                std::max(1,n(pos.width)),float(c.alpha)/255.f);
  ++textCalls_;
 }
 litehtml::pixel_t pt_to_px(float pt) const override{return litehtml::pixel_t(pt*96.f/72.f);}
 litehtml::pixel_t get_default_font_size() const override{return litehtml::pixel_t(16);}
 const char* get_default_font_name() const override{return "DejaVu Sans";}
 void draw_list_marker(litehtml::uint_ptr,const litehtml::list_marker&) override{}
 void load_image(const char*,const char*,bool) override{}
 void get_image_size(const char*,const char*,litehtml::size& size) override{
  size.width=0;size.height=0;
 }
 void draw_image(litehtml::uint_ptr,const litehtml::background_layer&,const std::string&,
                 const std::string&) override{}
 void draw_solid_fill(litehtml::uint_ptr,const litehtml::background_layer& layer,
                      const litehtml::web_color& c) override{
  fill(n(layer.clip_box.x),n(layer.clip_box.y),
       n(layer.clip_box.width),n(layer.clip_box.height),c);
 }
 void draw_linear_gradient(litehtml::uint_ptr,const litehtml::background_layer& layer,
                           const litehtml::background_layer::linear_gradient& grad) override{
  if(grad.color_points.empty())return;
  const int x=n(layer.clip_box.x),y=n(layer.clip_box.y),
            w=n(layer.clip_box.width),h=n(layer.clip_box.height);
  if(w<=0||h<=0)return;
  auto a=grad.color_points.front().color,b=grad.color_points.back().color;
  auto start=grad.start,end=grad.end;
  const double dx=end.x-start.x,dy=end.y-start.y,denom=dx*dx+dy*dy;
  for(int yy=std::max(0,y);yy<std::min(h_,y+h);yy++)
   for(int xx=std::max(0,x);xx<std::min(w_,x+w);xx++){
    double t=denom>0.0001?((xx-start.x)*dx+(yy-start.y)*dy)/denom:0;
    t=std::clamp(t,0.,1.);
    blend(xx,yy,int(a.red*(1-t)+b.red*t),int(a.green*(1-t)+b.green*t),
          int(a.blue*(1-t)+b.blue*t),int(a.alpha*(1-t)+b.alpha*t));
   }
  ++fillCalls_;
 }
 void draw_radial_gradient(litehtml::uint_ptr,const litehtml::background_layer& layer,
                           const litehtml::background_layer::radial_gradient& grad) override{
  if(grad.color_points.empty())return;
  auto a=grad.color_points.front().color,b=grad.color_points.back().color;
  int x=n(layer.clip_box.x),y=n(layer.clip_box.y),w=n(layer.clip_box.width),h=n(layer.clip_box.height);
  double rx=std::max(1.f,grad.radius.x),ry=std::max(1.f,grad.radius.y);
  for(int yy=std::max(0,y);yy<std::min(h_,y+h);yy++)
   for(int xx=std::max(0,x);xx<std::min(w_,x+w);xx++){
    double dx=(xx-grad.position.x)/rx,dy=(yy-grad.position.y)/ry;
    double t=std::clamp(std::sqrt(dx*dx+dy*dy),0.,1.);
    blend(xx,yy,int(a.red*(1-t)+b.red*t),int(a.green*(1-t)+b.green*t),
          int(a.blue*(1-t)+b.blue*t),int(a.alpha*(1-t)+b.alpha*t));
   }
  ++fillCalls_;
 }
 void draw_conic_gradient(litehtml::uint_ptr,const litehtml::background_layer&,
                          const litehtml::background_layer::conic_gradient&) override{}
 void draw_borders(litehtml::uint_ptr,const litehtml::borders& b,
                   const litehtml::position& p,bool) override{
  const int x=n(p.x),y=n(p.y),w=n(p.width),h=n(p.height);
  if(w<=0||h<=0)return;
  const int l=std::max(0,n(b.left.width)),r=std::max(0,n(b.right.width));
  const int t=std::max(0,n(b.top.width)),bt=std::max(0,n(b.bottom.width));
  if(t>0&&b.top.style!=litehtml::border_style_none)fill(x,y,w,t,b.top.color);
  if(bt>0&&b.bottom.style!=litehtml::border_style_none)fill(x,y+h-bt,w,bt,b.bottom.color);
  if(l>0&&b.left.style!=litehtml::border_style_none)fill(x,y,l,h,b.left.color);
  if(r>0&&b.right.style!=litehtml::border_style_none)fill(x+w-r,y,r,h,b.right.color);
 }
 void set_caption(const char*) override{}
 void set_base_url(const char*) override{}
 void link(const std::shared_ptr<litehtml::document>&,const litehtml::element::ptr&) override{}
 void on_anchor_click(const char*,const litehtml::element::ptr&) override{}
 void on_mouse_event(const litehtml::element::ptr&,litehtml::mouse_event) override{}
 void set_cursor(const char*) override{}
 void transform_text(std::string& s,litehtml::text_transform tt) override{
  if(tt==litehtml::text_transform_uppercase)
   for(char& c:s)if(c>='a'&&c<='z')c=char(c-'a'+'A');
  if(tt==litehtml::text_transform_lowercase)
   for(char& c:s)if(c>='A'&&c<='Z')c=char(c-'A'+'a');
 }
 void import_css(std::string& css,const std::string&,std::string&) override{css.clear();}
 void set_clip(const litehtml::position&,const litehtml::border_radiuses&) override{}
 void del_clip() override{}
 void get_viewport(litehtml::position& v) const override{
  v=litehtml::position(0,0,w_,h_);
 }
 litehtml::element::ptr create_element(const char*,const litehtml::string_map&,
                                       const std::shared_ptr<litehtml::document>&) override{return {};}
 void get_media_features(litehtml::media_features& m) const override{
  m.type=litehtml::media_type_screen;
  m.width=w_;m.height=h_;m.device_width=w_;m.device_height=h_;
  m.color=8;m.color_index=0;m.monochrome=0;m.resolution=96;
 }
 void get_language(std::string& language,std::string& culture) const override{
  language="es";culture="MX";
 }
};
}
