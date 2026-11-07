#include "dot_sounds.hpp"
#include <cassert>
#include <cmath>
#include <cstdlib>
#include <vector>
#include <unistd.h>
int main(){
    unsetenv("DOT_SOUND_PRESET"); assert(dot_sound_preset()==DotSoundPreset::Classic);
    setenv("DOT_SOUND_PRESET","gentle",1); assert(dot_sound_preset()==DotSoundPreset::Gentle);
    setenv("DOT_SOUND_PRESET","silent",1); assert(dot_sound_preset()==DotSoundPreset::Silent);
    setenv("DOT_SOUND_PRESET","unknown",1); assert(dot_sound_preset()==DotSoundPreset::Classic);
    short classic[1920]{},gentle[1920]{};
    ringback_block(10,classic);gentle_ringback_block(10,gentle);
    bool differs=false;int peak=0;
    for(int i=0;i<960;i++){assert(classic[2*i]==classic[2*i+1]);assert(gentle[2*i]==gentle[2*i+1]);differs|=classic[2*i]!=gentle[2*i];peak=std::max(peak,std::abs(int(gentle[2*i])));}
    assert(differs && peak>500 && peak<5000);
    handset_cue_block(true,0,classic);handset_cue_block(false,0,gentle);
    differs=false;for(int i=0;i<1920;i++)differs|=classic[i]!=gentle[i];assert(differs);
    std::vector<short> small={10,10,20,20};
    dot_custom_sound_block(small,0,true,classic);
    assert(classic[0]==10 && classic[2]==20 && classic[4]==10);
    dot_custom_sound_block(small,0,false,classic);
    assert(classic[0]==10 && classic[2]==20 && classic[4]==0);
    char path[]="/tmp/dot-sounds-test-XXXXXX";int fd=mkstemp(path);assert(fd>=0);
    assert(write(fd,small.data(),small.size()*sizeof(short))==ssize_t(small.size()*sizeof(short)));close(fd);
    setenv("DOT_SOUND_CONNECTED",path,1);assert(dot_load_pcm("DOT_SOUND_CONNECTED")==small);
    assert(dot_resolve_pcm("DOT_SOUND_CONNECTED","connected.pcm",DotSoundPreset::Classic)==small);
    unlink(path);assert(dot_load_pcm("DOT_SOUND_CONNECTED").empty());unsetenv("DOT_SOUND_CONNECTED");
    char folder[]="/tmp/dot-sound-pack-XXXXXX";assert(mkdtemp(folder));
    std::string builtin=std::string(folder)+"/connected.pcm";
    FILE *f=fopen(builtin.c_str(),"wb");assert(f);assert(fwrite(small.data(),sizeof(short),small.size(),f)==small.size());fclose(f);
    setenv("DOT_SOUND_PACK_DIR",folder,1);
    assert(dot_resolve_pcm("DOT_SOUND_CONNECTED","connected.pcm",DotSoundPreset::Classic)==small);
    assert(dot_resolve_pcm("DOT_SOUND_CONNECTED","connected.pcm",DotSoundPreset::Gentle).empty());
    unlink(builtin.c_str());rmdir(folder);unsetenv("DOT_SOUND_PACK_DIR");
}
