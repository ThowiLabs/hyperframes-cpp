#pragma once
// Tema "Noticiero digital": segunda presentacion del mismo HTML.
// Geometria, jerarquia, transiciones y cintillos son nativos en C++17.
inline void newsLine(int x0,int y0,int x1,int y1,RGB c,int thick=2){
 const int n=std::max(std::abs(x1-x0),std::abs(y1-y0));
 if(!n){rect(x0,y0,thick,thick,c);return;}
 for(int i=0;i<=n;i++){
  int x=x0+(x1-x0)*i/n,y=y0+(y1-y0)*i/n;
  rect(x-thick/2,y-thick/2,thick,thick,c);
 }
}
inline void newsOutline(int x,int y,int w,int h,RGB c,int thick=2){
 rect(x,y,w,thick,c);rect(x,y+h-thick,w,thick,c);
 rect(x,y,thick,h,c);rect(x+w-thick,y,thick,h,c);
}
inline RGB newsMix(RGB a,RGB b,float t){
 t=std::clamp(t,0.f,1.f);
 return {static_cast<unsigned char>(a.r*(1-t)+b.r*t),
         static_cast<unsigned char>(a.g*(1-t)+b.g*t),
         static_cast<unsigned char>(a.b*(1-t)+b.b*t)};
}
inline void newsBase(int scene,double t){
 static std::vector<std::vector<unsigned char>> cached(6);
 const RGB navy{8,18,33},blue{9,39,67},teal{16,91,112};
 if(cached[std::min(scene,5)].empty()){
  auto& dst=cached[std::min(scene,5)];
  dst.resize(size_t(W)*H*3);
  for(int y=0;y<H;y++){
   for(int x=0;x<W;x++){
    float yy=float(y)/H,xx=float(x)/W;
    RGB c=newsMix(navy,blue,.16f+.45f*yy+.15f*xx);
    const float dx=(x-545.f)/520.f,dy=(y-510.f)/780.f;
    const float glow=.25f*std::max(0.f,1.f-dx*dx-dy*dy);
    c=newsMix(c,teal,glow);
    if(y<125||y>1175)c=newsMix(c,navy,.55f);
    if(y>=160 && y<=1075 && (y%75==0))c=newsMix(c,{44,100,124},.14f);
    if(x%104==0 && y>170 && y<1090)c=newsMix(c,{44,100,124},.12f);
    auto p=(size_t(y)*W+x)*3;
    dst[p]=c.b;dst[p+1]=c.g;dst[p+2]=c.r;
   }
  }
 }
 pixels=cached[std::min(scene,5)];
 // Líneas editoriales animadas, sutiles.
 RGB line{26,78,103};
 const int x=520+int(65*std::sin(t*.24));
 newsLine(x,164,x-180,1060,line,1);
 newsLine(x+74,164,x-106,1060,{15,60,84},1);
 for(int i=0;i<7;i++){
  const int xx=520+i*24,yy=190+i*19;
  rect(xx,yy,4,4,{25,101,125});
 }
}
inline float newsFade(double sec,double begin,double len){
 return float(motion::value(sec,begin,len,0.,1.,motion::Ease::expoOut));
}
inline void newsHeadline(FontPainter& p,const hf::Scene& s,double local){
 int count=static_cast<int>(s.lines.size());
 int pt=count<=2?68:65;
 for(const auto& line:s.lines){
  if(line.size()>21)pt=58;
 }
 if(count>=4)pt=51;
 const int gap=std::max(69,int(pt*1.27));
 const int top=(count==2?304:278);
 for(int i=0;i<count;i++){
  auto entrance=motion::enter(local,.23+i*.17,.55,38.,15.,motion::Ease::expoOut);
  p.draw(58+int(entrance.x),top+i*gap+int(entrance.y),
         s.lines[i],pt,{250,253,255},true,605,float(entrance.opacity));
 }
}
inline void newsTextWrap(FontPainter& p,const std::string& value,int x,int y,
                         int pt,int maxwidth,RGB color,float alpha){
 std::string line;int row=0;size_t at=0;
 while(at<value.size()&&row<3){
  auto end=value.find(' ',at);
  if(end==std::string::npos)end=value.size();
  std::string word=value.substr(at,end-at),candidate=line.empty()?word:line+" "+word;
  if(!line.empty()&&p.measure(candidate,pt,false)>maxwidth){
   p.draw(x,y+row*(pt+9),line,pt,color,false,maxwidth,alpha);
   row++;line=word;
  }else line=candidate;
  at=end+1;
 }
 if(!line.empty()&&row<3)p.draw(x,y+row*(pt+9),line,pt,color,false,maxwidth,alpha);
}
inline void newsVisual(FontPainter& p,const hf::Scene& s,size_t i,double local,double t){
 const RGB cyan{71,222,245},red{244,66,70},white{244,249,253},muted{165,197,216},ink{9,22,36};
 const int py=663,ph=423;
 rect(49,py+10,622,ph,{3,12,23});
 rect(50,py,620,ph,{11,34,55});
 newsOutline(50,py,620,ph,{45,104,130},2);
 rect(50,py,620,7,cyan);
 rect(70,py+28,191,32,{15,67,87});
 p.draw(84,py+34,"EN DETALLE",17,cyan,true,160);
 p.draw(547,py+31,std::to_string(i+1)+" / "+std::to_string(6),17,muted,true,91);
 const float a=newsFade(local,.85,.65);
 // Lineas de escaneo en el grafico activo.
 int beam=76+int(std::fmod(std::max(0.,local)*115.,535.));
 rect(beam,py+81,2,ph-101,{21,63,82});
 if(s.kind=="meme-face"){
  rect(88,py+94,213,51,red);
  p.draw(113,py+103,"¿RUMOR?",30,white,true,180,a);
  rect(346,py+94,275,51,{14,107,114});
  p.draw(366,py+106,"EN CONTEXTO",23,white,true,230,a);
  circle(354,py+273,104,{252,194,67},true);
  circle(315,py+246,9,ink,true);circle(393,py+246,9,ink,true);
  rect(324,py+310,60,8,ink);
  rect(110,py+214,145,72,{24,70,98});newsOutline(110,py+214,145,72,cyan,2);
  p.draw(128,py+230,"MEME",30,white,true,115,a);
  newsLine(255,py+250,275,py+265,cyan,3);
  p.draw(87,py+368,"¿CUÁL ES EL CONTEXTO?",20,muted,false,525,a);
 }else if(s.kind=="dateart"){
  p.draw(86,py+96,"DATOS",77,white,true,470,a);
  p.draw(89,py+196,"INFO",106,cyan,true,450,a);
  rect(90,py+332,520,3,{42,114,137});
  for(int k=0;k<8;k++){
   int bh=16+int(21*std::sin(t*.7+k));
   rect(101+k*67,py+332-bh,21,bh,k<5?cyan:red);
  }
  p.draw(91,py+354,"CONTEXTO  /  DATOS",20,white,true,510,a);
 }else if(s.kind=="symbols"){
  std::string labels[]={"IDENTIDAD","DOMINIOS","DOCUMENTOS"};
  for(int k=0;k<3;k++){
   const int yy=py+94+k*96;
   rect(89,yy,536,77,{15,54,75});
   newsOutline(89,yy,536,77,{29,101,126},2);
   rect(89,yy,9,77,k==0?red:cyan);
   p.draw(110,yy+16,"0"+std::to_string(k+1),40,cyan,true,75,a);
   p.draw(195,yy+24,labels[k],28,white,true,400,a);
  }
  p.draw(89,py+386,"ANÁLISIS EN PROFUNDIDAD",16,muted,true,500,a);
 }else if(s.kind=="ai-art"){
  rect(91,py+87,245,237,{12,60,82});
  newsOutline(91,py+87,245,237,cyan,2);
  p.draw(126,py+116,"IA",103,white,true,180,a);
  p.draw(118,py+268,"VIDEO FALSO",21,cyan,true,200,a);
  for(int k=0;k<31;k++){
   int xx=365+k*8;
   int hh=12+int(74*std::abs(std::sin(t*4.+k*.5))*std::abs(std::sin(k*.37+2.)));
   rect(xx,py+194-hh/2,4,hh,k%6==0?red:cyan);
  }
  p.draw(362,py+279,"VOZ CLONADA",23,white,true,240,a);
  p.draw(96,py+365,"FRAUDE DIGITAL",23,muted,true,510,a);
 }else if(s.kind=="debate"){
  rect(82,py+92,557,95,{12,62,82});
  rect(82,py+92,9,95,cyan);
  p.draw(114,py+112,"PERSPECTIVA UNO",31,white,true,490,a);
  p.draw(320,py+205,"VS",40,red,true,100,a);
  rect(82,py+271,557,95,{73,31,47});
  rect(82,py+271,9,95,red);
  p.draw(114,py+291,"PERSPECTIVA DOS",28,white,true,485,a);
 }else if(s.kind=="conclusion"){
  rect(87,py+96,550,95,{17,104,103});
  p.draw(126,py+112,"MEME",60,white,true,470,a);
  rect(87,py+211,550,99,{89,32,51});
  p.draw(128,py+224,"CONTEXTO REAL",48,white,true,480,a);
  newsLine(91,py+323,640,py+323,cyan,3);
  p.draw(111,py+345,"EL CONTEXTO IMPORTA",21,muted,true,495,a);
 }else{
  p.draw(90,py+200,s.highlight,28,white,true,520,a);
 }
}
inline void renderNews(int fr,int fps,const std::vector<hf::Scene>& scenes,double duration){
 static FontPainter fp;
 const double t=double(fr)/fps;
 size_t i=0;while(i+1<scenes.size()&&t>=scenes[i+1].start)++i;
 const auto& s=scenes[i];const double local=t-s.start;
 const RGB cyan{71,222,245},red{244,66,70},white{244,249,253},muted{168,193,208},ink{9,22,36};
 newsBase(int(i),t);
 // Cabezal con marca editorial ficticia y fecha.
 rect(0,0,W,14,red);
 rect(48,37,59,58,red);
 fp.draw(67,46,"F",34,white,true,40);
 fp.draw(125,40,"HYPERFRAMES C++",30,white,true,383);
 fp.draw(126,80,"COMPOSITOR  /  DIGITAL",13,muted,true,410);
 fp.draw(579,46,"MOTOR C++",17,white,true,110);
 rect(47,122,626,2,{47,102,131});
 // Cintillo periodístico. "Análisis" evita afirmar noticias de última hora.
 rect(49,155,296,43,red);
 fp.draw(66,164,"INFORME ESPECIAL",21,white,true,266);
 rect(358,155,311,43,{18,60,86});
 fp.draw(370,165,"EN CONTEXTO",18,cyan,true,281);
 fp.draw(54,221,"CONTENIDO  /  ANALISIS",20,cyan,true,570);
 newsHeadline(fp,s,local);
 int contentBottom=int(s.lines.size())<=2?518:522;
 newsTextWrap(fp,s.sub,58,contentBottom+14,24,603,muted,newsFade(local,.75,.6));
 // Separador animado, más flexible que presentación de tarjetas estáticas.
 rect(49,624,622,3,{41,104,136});
 rect(49,624,int(622*std::clamp((local+.2)/s.duration,0.,1.)),3,red);
 newsVisual(fp,s,i,local,t);
 // Lower-third permanente, con titular procedente del HTML.
 rect(0,1096,720,70,{5,20,37});
 rect(49,1109,166,46,red);
 fp.draw(62,1119,"EL DATO",25,white,true,136);
 rect(217,1109,451,46,{18,61,84});
 fp.draw(234,1120,s.highlight,23,white,true,411,newsFade(local,1.2,.5));
 // Ticker en movimiento y barra de avance global.
 rect(0,1177,720,56,{14,47,65});
 rect(0,1177,720,3,cyan);
 const std::string ticker="CONTENIDO DIGITAL  ·  INFORMACION EN CONTEXTO  ·  HYPERFRAMES C++  ·  ";
 float tickerWidth=fp.measure(ticker,19,false);
 int tickerX=int(20-std::fmod(t*44.,double(tickerWidth+20)));
 fp.draw(tickerX,1194,ticker,19,white,false,-1,.86f);
 fp.draw(tickerX+int(tickerWidth)+32,1194,ticker,19,white,false,-1,.86f);
 rect(0,1243,720,37,ink);
 fp.draw(48,1251,"HYPERFRAMES C++",16,cyan,true,220);
 fp.draw(569,1251,std::to_string(i+1)+"/"+std::to_string(scenes.size())+"  SECCIONES",13,white,true,141);
 rect(0,H-6,int(W*t/duration),6,red);
 // Cortinilla roja breve en cada cambio de escena.
 if(local<.27){
  int width=int(W*(1.-local/.27));
  rect(W-width,0,width,4,red);
 }
}
