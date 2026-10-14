#pragma once

#include <algorithm>
#include <cmath>
#include <stdexcept>
#include <string>
#include <vector>

// Deterministic scene-local animation tracks. Sampling depends only on time,
// so rendering frames out of order produces the same visual state.
namespace motion {
enum class Ease { linear, power1In, power2Out, power3Out, expoOut, backOut, backOut16 };

inline Ease parseEase(const std::string& value) {
 if(value=="none" || value=="linear")return Ease::linear;
 if(value=="power1.in")return Ease::power1In;
 if(value=="power2.out")return Ease::power2Out;
 if(value=="power3.out")return Ease::power3Out;
 if(value=="expo.out")return Ease::expoOut;
 if(value=="back.out(1.2)" || value=="back.out")return Ease::backOut;
 if(value=="back.out(1.6)")return Ease::backOut16;
 throw std::invalid_argument("Unsupported easing curve: "+value);
}

inline double ease(double fraction,Ease curve) noexcept {
 const double t=std::clamp(fraction,0.,1.);
 switch(curve) {
  case Ease::linear: return t;
  case Ease::power1In: return t*t;
  case Ease::power2Out: return 1.-(1.-t)*(1.-t);
  case Ease::power3Out: return 1.-(1.-t)*(1.-t)*(1.-t);
  case Ease::expoOut: return t==1. ? 1. : (t==0. ? 0. : 1.-std::pow(2.,-10.*t));
  case Ease::backOut:
  case Ease::backOut16: { const double x=t-1.; const double c=curve==Ease::backOut16?1.6:1.2; return 1.+(c+1.)*x*x*x+c*x*x; }
 }
 return t;
}

inline double value(double time,double begin,double seconds,double from,double to,Ease curve) {
 if(!std::isfinite(time)||!std::isfinite(begin)||!std::isfinite(seconds)||
    !std::isfinite(from)||!std::isfinite(to)||seconds<=0.)
  throw std::invalid_argument("Invalid animation parameters");
 return from+(to-from)*ease((time-begin)/seconds,curve);
}

struct State {
 double x=0.,y=0.,scale=1.,opacity=1.;
};

inline State mix(const State& a,const State& b,double t) noexcept {
 return {a.x+(b.x-a.x)*t,a.y+(b.y-a.y)*t,
         a.scale+(b.scale-a.scale)*t,
         a.opacity+(b.opacity-a.opacity)*t};
}

struct Clip {
 double begin,seconds;
 State from,to;
 Ease easing;
};

// Non-overlapping clips express a scene-local timeline (independent of FPS).
// A held final value becomes the state before the next clip.
class Track {
 std::vector<Clip> clips_;
 State initial_;
 public:
 explicit Track(State initial={}):initial_(initial){}
 void add(const Clip& clip) {
  if(!std::isfinite(clip.begin)||clip.begin<0.||
     !std::isfinite(clip.seconds)||clip.seconds<=0.||
     !std::isfinite(clip.from.x)||!std::isfinite(clip.to.x)||
     !std::isfinite(clip.from.y)||!std::isfinite(clip.to.y)||
     !std::isfinite(clip.from.opacity)||!std::isfinite(clip.to.opacity)||
     !std::isfinite(clip.from.scale)||!std::isfinite(clip.to.scale))
   throw std::invalid_argument("Invalid animation clip");
  if(!clips_.empty() && clip.begin<clips_.back().begin+clips_.back().seconds)
   throw std::invalid_argument("Overlapping clips are not supported");
  clips_.push_back(clip);
 }
 State at(double seconds) const {
  if(!std::isfinite(seconds))throw std::invalid_argument("Invalid timeline time");
  State state=initial_;
  for(const auto& clip:clips_) {
   if(seconds<clip.begin)break;
   if(seconds<clip.begin+clip.seconds)
    return mix(clip.from,clip.to,ease((seconds-clip.begin)/clip.seconds,clip.easing));
   state=clip.to;
  }
  return state;
 }
};

// Common entrance used by both built-in themes.
inline State enter(double local,double begin,double duration,
                   double shiftX,double shiftY,Ease curve=Ease::power2Out) {
 const State a{shiftX,shiftY,.94,0.},b{0.,0.,1.,1.};
 return mix(a,b,ease((local-begin)/duration,curve));
}
}
