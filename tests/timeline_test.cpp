#include "../animation_timeline.hpp"
#include <cassert>
#include <cmath>
#include <iostream>
#include <limits>

static bool near(double a,double b,double eps=1e-9){return std::abs(a-b)<eps;}

int main(){
 using namespace motion;
 for(Ease e:{Ease::linear,Ease::power1In,Ease::power2Out,
             Ease::power3Out,Ease::expoOut,Ease::backOut,Ease::backOut16}){
  assert(near(ease(0.,e),0.));assert(near(ease(1.,e),1.));
  assert(near(ease(-1.,e),0.));assert(near(ease(2.,e),1.));
 }
 assert(parseEase("expo.out")==Ease::expoOut);
 assert(parseEase("back.out(1.2)")==Ease::backOut);
 assert(parseEase("back.out(1.6)")==Ease::backOut16);
 bool threw=false;
 try{(void)parseEase("unknown");}catch(const std::invalid_argument&){threw=true;}
 assert(threw);
 assert(near(value(2.,1.,2.,0.,100.,Ease::linear),50.));
 assert(near(value(0.,1.,2.,0.,100.,Ease::linear),0.));
 assert(near(value(4.,1.,2.,0.,100.,Ease::linear),100.));
 threw=false;try{value(0.,0.,0.,0.,1.,Ease::linear);}catch(const std::invalid_argument&){threw=true;}assert(threw);

 Track track(State{0.,0.,.8,0.});
 track.add({.5,1.,{0.,60.,.8,0.},{0.,0.,1.,1.},Ease::power2Out});
 track.add({2.,.5,{0.,0.,1.,1.},{50.,0.,1.,0.},Ease::power1In});
 assert(near(track.at(0.).opacity,0.));
 assert(near(track.at(1.5).opacity,1.));
 assert(near(track.at(1.7).opacity,1.));
 assert(near(track.at(2.5).opacity,0.));
 assert(near(track.at(2.25).opacity,.75));

 // Exactly the same animation time must produce the same state at 24 or 60 FPS.
 const double a=24./24.,b=60./60.;
 assert(near(enter(a,.15,.85,0.,40.).y,enter(b,.15,.85,0.,40.).y));
 // Sampling in non-monotonic order is deterministic.
 auto at2=track.at(2.25);(void)track.at(0.);assert(near(track.at(2.25).opacity,at2.opacity));
 threw=false;
 try{track.add({2.25,.2,{}, {},Ease::linear});}catch(const std::invalid_argument&){threw=true;}
 assert(threw);
 threw=false;
 try{(void)track.at(std::numeric_limits<double>::quiet_NaN());}catch(const std::invalid_argument&){threw=true;}
 assert(threw);
 std::cout<<"Animation timeline tests passed\n";
}
