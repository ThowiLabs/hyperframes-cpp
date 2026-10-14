#pragma once
// Elementos de la composición faceless de HyperFrames, dibujados en C++.
// Medidas y paleta basadas en el CSS original de index.html (720x1280).
inline RGB blendColor(RGB a,RGB b,float t){
 auto b1=[&](int x,int y){return static_cast<unsigned char>(std::clamp(int(x*(1.f-t)+y*t),0,255));};
 return {b1(a.r,b.r),b1(a.g,b.g),b1(a.b,b.b)};
}
inline void borderRect(int x,int y,int w,int h,RGB c,int thickness=2){
 rect(x,y,w,thickness,c);rect(x,y+h-thickness,w,thickness,c);
 rect(x,y,thickness,h,c);rect(x+w-thickness,y,thickness,h,c);
}
inline void backgroundFor(size_t scene,RGB accent){
 static std::vector<std::vector<unsigned char>> cache;
 if(cache.size()<=scene)cache.resize(scene+1);
 if(cache[scene].empty()){
  static const RGB top[6]={{21,43,58},{16,28,42},{24,34,52},{14,32,52},{31,32,48},{18,30,36}};
  static const RGB bottom[6]={{7,16,25},{16,42,54},{5,17,28},{23,33,62},{11,24,36},{18,32,28}};
  auto& result=cache[scene];result.resize(size_t(W)*H*3);
  for(int y=0;y<H;++y){
   auto base=blendColor(top[std::min(scene,size_t(5))],bottom[std::min(scene,size_t(5))],float(y)/H);
   for(int x=0;x<W;++x){
    float dx=float(x-660)/530.f,dy=float(y-240)/520.f;
    const float glow=0.17f*std::max(0.f,1.f-dx*dx-dy*dy);
    RGB p=blendColor(base,accent,glow);
    if(x%38==0||y%38==0)p=blendColor(p,{205,219,229},0.095f);
    auto ix=(size_t(y)*W+x)*3;result[ix]=p.b;result[ix+1]=p.g;result[ix+2]=p.r;
   }
  }
 }
 pixels=cache[scene];
}
inline float fadeIn(double t,double start,double duration){
 return float(motion::value(t,start,duration,0.,1.,motion::Ease::power2Out));
}
inline void subLines(FontPainter& fp,const std::string& content,int x,int y,int px,int width,RGB color,float opacity){
 std::string current;
 size_t i=0;
 int shown=0;
 while(i<content.size()&&shown<3){
  auto sep=content.find(' ',i);
  if(sep==std::string::npos)sep=content.size();
  auto word=content.substr(i,sep-i);
  auto next=current.empty()?word:current+" "+word;
  if(!current.empty()&&fp.measure(next,px,false)>width){
   fp.draw(x,y+shown*(px+10),current,px,color,false,width,opacity);
   ++shown;current=word;
  }else current=next;
  i=sep+1;
 }
 if(!current.empty()&&shown<3)fp.draw(x,y+shown*(px+10),current,px,color,false,width,opacity);
}
inline void render(int fr,int fps,const std::vector<hf::Scene>& scenes,double duration){
 static FontPainter painter;
 const double t=double(fr)/fps;
 size_t idx=0;
 while(idx+1<scenes.size()&&t>=scenes[idx+1].start)++idx;
 const auto& s=scenes[idx];
 const double local=t-s.start;
 const RGB accent{s.r,s.g,s.b},white{255,255,255},dark{15,32,41};
 backgroundFor(idx,accent);
 // Movimientos orbitales suaves de la composición original.
 circle(585+int(38*std::sin(t*.28)),465,215,blendColor(accent,{11,25,36},.57f));
 circle(-122+int(35*std::sin(t*.3)),945,126,blendColor(accent,{11,25,36},.60f));
 circle(540,100,240,blendColor(accent,{21,33,43},.77f));
 // Encabezado original: top:45px, left:65px.
 painter.draw(65,45,"HYPERFRAMES C++  /  VISUAL",15,{213,224,224},true,510,.82f);
 const auto count=std::to_string(idx+1)+std::string(" / ")+std::to_string(scenes.size());
 painter.draw(565,40,count,26,accent,true,115,1);
 // Kicker: top:210px, height:52px, border-color: --a.
 const float kickerFade=fadeIn(local,.12,.55);
 const int x=s.layout.upperLeft,y=s.layout.upperTop;
 const int tagWidth=std::min(W-x-48,int(painter.measure(s.tag,s.layout.kickerFont,true))+53);
 rect(x,y,tagWidth,s.layout.kickerHeight,{5,21,31});
 borderRect(x,y,tagWidth,s.layout.kickerHeight,accent,1);
 circle(x+21,y+s.layout.kickerHeight/2,6,accent,true);
 painter.draw(x+40,y+16,s.tag,s.layout.kickerFont,accent,true,tagWidth-54,kickerFade);
 // Títulos a partir de top ~312, siguiendo font-size CSS original.
 int headlinePx=s.layout.headlineFont;
 const int baseTop=s.layout.upperTop+s.layout.kickerHeight+s.layout.headlineMargin;
 const int spacing=int(headlinePx*1.08);
 for(size_t i=0;i<s.lines.size();++i){
  auto entrance=motion::enter(local,.4+i*.16,.8,0.,64.,motion::Ease::power3Out);
  painter.draw(s.layout.upperLeft+int(entrance.x),
               baseTop+int(i)*spacing+int(entrance.y),
               s.lines[i],headlinePx,white,true,
               W-s.layout.upperLeft-70,float(entrance.opacity));
 }
 // Subtítulos: después de última línea, margen de ~42.
 const int subY=baseTop+int(s.lines.size())*spacing+s.layout.subMargin;
 subLines(painter,s.sub,s.layout.upperLeft,std::min(subY,682),
          s.layout.subFont,570,{218,229,228},fadeIn(local,1.15,.7));
 // Arte específico de cada sección del HTML.
 const int cy=905+int(14*std::sin(t*.5));
 const float extraFade=fadeIn(local,1.55,.85);
 const auto faceColor=blendColor(accent,{9,26,38},1.f-extraFade);
 if(s.kind=="meme-face"){
  circle(360,cy,123,faceColor,true);
  circle(315,cy-34,15,dark,true);circle(407,cy-34,15,dark,true);
  rect(330,cy+31,69,14,dark);
  rect(460,cy-130,110,68,{246,247,245});
  painter.draw(482,cy-122,"¿?",49,{18,32,42},true,88,extraFade);
 }else if(s.kind=="dateart"){
  borderRect(65,775,590,252,faceColor,4);
  painter.draw(89,805,"INFO",150,faceColor,true,548,extraFade);
  rect(80,1004,540,5,faceColor);
  rect(509,795,130,47,{8,42,57});painter.draw(519,805,"DATOS",16,white,true,114,extraFade);
 }else if(s.kind=="symbols"){
  const std::string tags[]={"01  IDENTIDAD","02  DOMINIOS","03  DOCUMENTOS"};
  for(int i=0;i<3;i++){
   const int yy=793+i*78;
   rect(72,yy,570,68,{13,36,54});borderRect(72,yy,570,68,{73,106,128},2);
   painter.draw(90,yy+17,tags[i],28,i==0?accent:white,true,535,extraFade);
  }
 }else if(s.kind=="ai-art"){
  circle(360,cy,141,faceColor);
  circle(360,cy,125,blendColor(faceColor,{9,26,38},.72f));
  painter.draw(275,cy-62,"IA",125,faceColor,true,190,extraFade);
  rect(225,cy+120,270,46,faceColor);
  painter.draw(240,cy+130,"¿VIDEO REAL?",16,dark,true,245,extraFade);
 }else if(s.kind=="debate"){
  borderRect(65,792,590,82,{102,183,182},3);
  painter.draw(82,818,"CONTEXTO: UN PUNTO DE VISTA",25,white,true,550,extraFade);
  painter.draw(315,886,"VS.",39,faceColor,true,100,extraFade);
  borderRect(65,950,590,82,faceColor,3);
  painter.draw(82,973,"ANALISIS: OTRO PUNTO DE VISTA",22,white,true,550,extraFade);
 }else if(s.kind=="conclusion"){
  rect(72,854,265,94,faceColor);rect(425,854,231,94,white);
  painter.draw(83,870,"MEME",52,dark,true,240,extraFade);
  painter.draw(349,870,"≠",60,white,true,61,extraFade);
  painter.draw(436,870,"DELITO",40,dark,true,205,extraFade);
 }
 // Etiqueta y pie. CSS: barlabel, bottom-meta.
 rect(65,1112,590,71,{6,21,33});rect(65,1112,9,71,accent);
 borderRect(65,1112,590,71,blendColor(accent,{5,23,34},.50f),1);
 painter.draw(88,1131,s.highlight,19,white,true,550,fadeIn(local,1.85,.65));
 painter.draw(67,1211,"CONTENIDO DIGITAL",12,{174,194,197},true,386,.93f);
 painter.draw(552,1211,"VIDEO C++",12,{174,194,197},true,110,.93f);
 rect(0,H-9,int(W*t/duration),9,accent);
}
