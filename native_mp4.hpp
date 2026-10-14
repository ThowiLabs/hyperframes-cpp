#pragma once
#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <cmath>
#include <memory>
#include <stdexcept>
#include <vector>

#define H264E_MAX_THREADS 0
#include "minih264e.h"
#include "minimp4.h"
#include <common/include/voAAC.h>
#include <common/include/cmnMemory.h>

// In-process H.264/AAC writer. Codec sources compile as C and are linked
// statically; no FFmpeg, Chromium, or command-line encoder is executed.
// minih264 is experimental; distribution of H.264/AAC may implicate patents.
class NativeMp4Writer {
 int width_,height_,fps_,audioId_=-1;
 FILE* file_=nullptr;
 MP4E_mux_t* mux_=nullptr;
 mp4_h26x_writer_t writer_{};
 H264E_persist_t* encoder_=nullptr;
 H264E_scratch_t* scratch_=nullptr;
 std::vector<uint8_t> persistent_,scratchMem_,yuv_;
 VO_AUDIO_CODECAPI audioApi_{};
 VO_MEM_OPERATOR memoryOp_{};
 VO_HANDLE audioHandle_=nullptr;
 static int writeCallback(int64_t offset,const void* data,size_t count,void* handle){
  auto* f=static_cast<FILE*>(handle);
  if(offset<0||offset>2147483647 || std::fseek(f,static_cast<long>(offset),SEEK_SET))return 1;
  return std::fwrite(data,1,count,f)!=count;
 }
 static uint8_t* ptr(std::vector<uint8_t>& storage){
  const auto address=reinterpret_cast<uintptr_t>(storage.data());
  return reinterpret_cast<uint8_t*>((address+63)&~uintptr_t(63));
 }
 static int clamp(int x){return std::max(0,std::min(255,x));}
 void convert(const std::vector<uint8_t>& bgr){
  if(bgr.size()!=size_t(width_)*height_*3)
   throw std::runtime_error("MP4 frame has wrong dimensions");
  auto* y=yuv_.data(),*u=y+width_*height_,*v=u+width_*height_/4;
  for(int j=0;j<height_;j++)for(int i=0;i<width_;i++){
   const auto k=(size_t(j)*width_+i)*3;
   const int b=bgr[k],g=bgr[k+1],r=bgr[k+2];
   y[j*width_+i]=uint8_t(clamp(((66*r+129*g+25*b+128)>>8)+16));
  }
  for(int j=0;j<height_;j+=2)for(int i=0;i<width_;i+=2){
   int sr=0,sg=0,sb=0;
   for(int yy=0;yy<2;yy++)for(int xx=0;xx<2;xx++){
    auto k=(size_t(j+yy)*width_+i+xx)*3;
    sb+=bgr[k];sg+=bgr[k+1];sr+=bgr[k+2];
   }
   sr/=4;sg/=4;sb/=4;
   auto off=(j/2)*(width_/2)+i/2;
   u[off]=uint8_t(clamp(((-38*sr-74*sg+112*sb+128)>>8)+128));
   v[off]=uint8_t(clamp(((112*sr-94*sg-18*sb+128)>>8)+128));
  }
 }
 public:
 NativeMp4Writer(const char* path,int w,int h,int fps):width_(w),height_(h),fps_(fps){
  if(w%16||h%16||w<16||h<16||fps<1||fps>60)
   throw std::invalid_argument("H264 requires 16-pixel-aligned width and height and 1..60 FPS");
  file_=std::fopen(path,"wb");if(!file_)throw std::runtime_error("Unable to open MP4 output");
  mux_=MP4E_open(0,0,file_,writeCallback);
  if(!mux_)throw std::runtime_error("Unable to create MP4 muxer");
  if(mp4_h26x_write_init(&writer_,mux_,w,h,0)!=MP4E_STATUS_OK)
   throw std::runtime_error("Unable to initialize H264 track");
  H264E_create_param_t p{};
  p.width=w;p.height=h;p.gop=std::max(1,fps*2);
  p.const_input_flag=1;p.num_layers=1;
  int mem=0,tmp=0;
  if(H264E_sizeof(&p,&mem,&tmp)||mem<=0||tmp<=0)
   throw std::runtime_error("H264 memory sizing failed");
  persistent_.resize(size_t(mem)+64);scratchMem_.resize(size_t(tmp)+64);
  encoder_=reinterpret_cast<H264E_persist_t*>(ptr(persistent_));
  scratch_=reinterpret_cast<H264E_scratch_t*>(ptr(scratchMem_));
  if(H264E_init(encoder_,&p))throw std::runtime_error("H264 initialization failed");
  yuv_.resize(size_t(w)*h*3/2);
  memoryOp_.Alloc=cmnMemAlloc;memoryOp_.Copy=cmnMemCopy;
  memoryOp_.Free=cmnMemFree;memoryOp_.Set=cmnMemSet;memoryOp_.Check=cmnMemCheck;
  VO_CODEC_INIT_USERDATA user{};
  user.memflag=VO_IMF_USERMEMOPERATOR;user.memData=&memoryOp_;
  if(voGetAACEncAPI(&audioApi_)!=VO_ERR_NONE||
     audioApi_.Init(&audioHandle_,VO_AUDIO_CodingAAC,&user)!=VO_ERR_NONE)
   throw std::runtime_error("AAC initialization failed");
  AACENC_PARAM aac{};aac.sampleRate=24000;aac.bitRate=48000;
  aac.nChannels=1;aac.adtsUsed=0;
  if(audioApi_.SetParam(audioHandle_,VO_PID_AAC_ENCPARAM,&aac)!=VO_ERR_NONE)
   throw std::runtime_error("AAC configuration failed");
  MP4E_track_t track{};track.track_media_kind=e_audio;
  track.object_type_indication=MP4_OBJECT_TYPE_AUDIO_ISO_IEC_14496_3;
  track.language[0]='u';track.language[1]='n';track.language[2]='d';
  track.time_scale=24000;track.default_duration=1024;track.u.a.channelcount=1;
  audioId_=MP4E_add_track(mux_,&track);
  const uint8_t dsi[]={0x13,0x08}; // MPEG-4 AAC LC, 24000 Hz, mono.
  if(audioId_<0 || MP4E_set_dsi(mux_,audioId_,dsi,2)!=MP4E_STATUS_OK)
   throw std::runtime_error("AAC MP4 track initialization failed");
 }
 NativeMp4Writer(const NativeMp4Writer&)=delete;
 NativeMp4Writer& operator=(const NativeMp4Writer&)=delete;
 ~NativeMp4Writer(){
  if(audioHandle_)audioApi_.Uninit(audioHandle_);
  if(mux_){mp4_h26x_write_close(&writer_);MP4E_close(mux_);}
  if(file_)std::fclose(file_);
 }
 void addVideo(const std::vector<unsigned char>& bgr){
  convert(bgr);
  H264E_io_yuv_t io{};
  io.yuv[0]=yuv_.data();io.stride[0]=width_;
  io.yuv[1]=yuv_.data()+width_*height_;io.stride[1]=width_/2;
  io.yuv[2]=yuv_.data()+width_*height_*5/4;io.stride[2]=width_/2;
  H264E_run_param_t run{};run.qp_min=run.qp_max=31;run.encode_speed=5;
  uint8_t* coded=nullptr;int bytes=0;
  if(H264E_encode(encoder_,scratch_,&run,&io,&coded,&bytes)||bytes<=0)
   throw std::runtime_error("H264 frame encoding failed");
  // The encoded frame is Annex B with one or more NAL units.
  int start=0;
  for(int i=3;i<=bytes;i++){
   bool next=i==bytes;
   if(!next && i+3<bytes){
    next=(coded[i]==0&&coded[i+1]==0&&coded[i+2]==1) ||
         (coded[i]==0&&coded[i+1]==0&&coded[i+2]==0&&coded[i+3]==1);
   }
   if(next&&i>start){
    if(mp4_h26x_write_nal(&writer_,coded+start,i-start,90000/fps_)!=MP4E_STATUS_OK)
     throw std::runtime_error("Failed to mux H264 packet");
    start=i;
   }
  }
 }
 void addAudio(const std::vector<int16_t>& pcm){
  const size_t count=(pcm.size()+1023)/1024;
  std::vector<int16_t> block(1024);
  for(size_t f=0;f<count;f++){
   std::fill(block.begin(),block.end(),0);
   auto from=f*1024;
   if(from<pcm.size())std::copy_n(pcm.begin()+from,std::min(size_t(1024),pcm.size()-from),block.begin());
   VO_CODECBUFFER in{},out{};VO_AUDIO_OUTPUTINFO info{};
   in.Buffer=reinterpret_cast<uint8_t*>(block.data());
   in.Length=2048;
   std::vector<uint8_t> coded(20480);
   out.Buffer=coded.data();out.Length=static_cast<VO_U32>(coded.size());
   if(audioApi_.SetInputData(audioHandle_,&in)!=VO_ERR_NONE ||
      audioApi_.GetOutputData(audioHandle_,&out,&info)!=VO_ERR_NONE)
    throw std::runtime_error("AAC frame encoding failed");
   const int sampleDuration=int(f==count-1 ? pcm.size()-f*1024 : 1024);
   if(out.Length && MP4E_put_sample(mux_,audioId_,coded.data(),out.Length,sampleDuration,MP4E_SAMPLE_DEFAULT)!=MP4E_STATUS_OK)
    throw std::runtime_error("Failed to mux AAC packet");
  }
 }
};
