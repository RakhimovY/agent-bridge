#pragma once
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <fstream>
#include <string>
#include <vector>

// Portable 48 kHz signed 16-bit stereo cue generators and per-event PCM overrides.
enum class DotSoundPreset {Classic,Gentle,Silent};
inline DotSoundPreset dot_sound_preset(){
    const char *s=std::getenv("DOT_SOUND_PRESET");
    if(s&&std::string(s)=="gentle")return DotSoundPreset::Gentle;
    if(s&&std::string(s)=="silent")return DotSoundPreset::Silent;
    return DotSoundPreset::Classic;
}
inline void ringback_block(unsigned int block,short *samples){
    constexpr double pi=3.141592653589793;unsigned int cycle=block%300;
    for(int i=0;i<960;++i){double t=(cycle*960+i)/48000.0;
        double envelope=cycle<100?std::clamp(std::min(t/.015,(2.0-t)/.015),0.0,1.0):0.0;
        short v=static_cast<short>(2200*envelope*(std::sin(2*pi*440*t)+std::sin(2*pi*480*t)));
        samples[i*2]=samples[i*2+1]=v;}
}
inline void gentle_ringback_block(unsigned int block,short *samples){
    constexpr double pi=3.141592653589793;unsigned int cycle=block%75;
    for(int i=0;i<960;++i){double t=(cycle*960+i)/48000.0;
        double envelope=cycle<15?std::clamp(std::min(t/.025,(.3-t)/.04),0.0,1.0):0.0;
        short v=static_cast<short>(1400*envelope*std::sin(2*pi*660*t));samples[i*2]=samples[i*2+1]=v;}
}
inline void handset_cue_block(bool opening,int block,short *samples){
    constexpr double pi=3.141592653589793;
    for(int i=0;i<960;++i){unsigned int n=static_cast<unsigned int>(block*960+i);double t=n/48000.0;
        uint32_t bits=(n+1)*747796405u+2891336453u;bits=((bits>>((bits>>28)+4))^bits)*277803737u;bits=(bits>>22)^bits;
        double noise=(double(bits&65535)/32767.5)-1.0;
        auto click=[&](double start,double hz,double level){double age=t-start;if(age<0||age>=.065)return 0.0;
            return level*std::exp(-age*58)*std::min(age/.001,1.0)*(std::sin(2*pi*hz*age)+.38*noise);};
        double v=opening?click(0,560,2400)+click(.080,1050,1800):click(0,1050,1900)+click(.072,420,2600);
        short sample=static_cast<short>(std::clamp(v,-9000.0,9000.0));samples[i*2]=samples[i*2+1]=sample;}
}
inline void gentle_cue_block(bool opening,int block,short *samples){
    constexpr double pi=3.141592653589793;double hz=(block<5)==opening?587:880;
    for(int i=0;i<960;++i){double t=(block%5*960+i)/48000.0;
        double env=std::clamp(std::min(t/.008,(.1-t)/.015),0.0,1.0);
        short v=static_cast<short>(1500*env*std::sin(2*pi*hz*t));samples[i*2]=samples[i*2+1]=v;}
}
inline std::vector<short> dot_read_pcm(const char *path){
    if(!path||!*path)return {};
    std::ifstream f(path,std::ios::binary|std::ios::ate);if(!f)return {};
    auto n=f.tellg();if(n<=0||n>std::streamoff(30*48000*4)||n%4)return {};
    std::vector<short> pcm(size_t(n)/sizeof(short));f.seekg(0);if(!f.read(reinterpret_cast<char*>(pcm.data()),n))return {};return pcm;
}
inline std::vector<short> dot_load_pcm(const char *variable){return dot_read_pcm(std::getenv(variable));}
inline std::vector<short> dot_resolve_pcm(const char *variable,const char *filename,DotSoundPreset preset){
    auto pcm=dot_load_pcm(variable);if(!pcm.empty()||preset!=DotSoundPreset::Classic)return pcm;
    const char *dir=std::getenv("DOT_SOUND_PACK_DIR");if(!dir||!*dir)return {};
    return dot_read_pcm((std::string(dir)+"/"+filename).c_str());
}
inline void dot_custom_sound_block(const std::vector<short>&pcm,size_t block,bool loop,short *samples){
    for(size_t i=0;i<1920;++i){size_t at=block*1920+i;samples[i]=pcm.empty()?0:loop?pcm[at%pcm.size()]:at<pcm.size()?pcm[at]:0;}
}
