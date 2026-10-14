#pragma once
// AVI BI_RLE8: codigo intra-frame con paleta 3:3:2.
// No necesita bibliotecas de compresion ni codificadores externos.
inline unsigned char rleIndex(const unsigned char* bgr){
 const int r=(int(bgr[2])*7+127)/255;
 const int g=(int(bgr[1])*7+127)/255;
 const int b=(int(bgr[0])*3+127)/255;
 return static_cast<unsigned char>((r<<5)|(g<<2)|b);
}
inline std::vector<unsigned char> encodeRle8(const std::vector<unsigned char>& rgb){
 std::vector<unsigned char> encoded;
 encoded.reserve(size_t(W)*H/3);
 std::array<unsigned char,W> row{};
 for(int y=H-1;y>=0;--y){
  for(int x=0;x<W;x++)row[x]=rleIndex(&rgb[(size_t(y)*W+x)*3]);
  int x=0;
  while(x<W){
   int repeated=1;
   while(x+repeated<W && repeated<255 && row[x+repeated]==row[x])++repeated;
   if(repeated>=3){
    encoded.push_back(static_cast<unsigned char>(repeated));
    encoded.push_back(row[x]);x+=repeated;
   }else{
    const int start=x;
    // Recoge colores individuales hasta un proximo run de 3 o mas.
    while(x<W && x-start<255){
     int run=1;
     while(x+run<W && run<3 && row[x+run]==row[x])++run;
     if(run>=3)break;
     x+=run;
    }
    int count=x-start;
    if(count==0)throw std::runtime_error("Error interno RLE");
    if(count>=3){
     encoded.push_back(0);
     encoded.push_back(static_cast<unsigned char>(count));
     for(int k=start;k<x;k++)encoded.push_back(row[k]);
     if(count&1)encoded.push_back(0);
    }else{
     for(int k=start;k<x;k++){encoded.push_back(1);encoded.push_back(row[k]);}
    }
   }
  }
  encoded.push_back(0);encoded.push_back(0); // end-of-line
 }
 encoded.push_back(0);encoded.push_back(1); // end-of-bitmap
 return encoded;
}
inline void writeRlePalette(std::ostream& o){
 for(int i=0;i<256;i++){
  const int r=(i>>5)&7,g=(i>>2)&7,b=i&3;
  o.put(char((b*255+1)/3));o.put(char((g*255+3)/7));
  o.put(char((r*255+3)/7));o.put(0);
 }
}
