// Dot Connect stop-word detector. Uses the TensorFlow Lite Micro frontend.
// Feature configuration follows pymicro-features 2.0.2 (Apache-2.0); see NOTICE.md.
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <deque>
#include <dlfcn.h>
#include <vector>
#include <string>
extern "C" {
#include "tensorflow/lite/experimental/microfrontend/lib/frontend.h"
#include "tensorflow/lite/experimental/microfrontend/lib/frontend_util.h"
}
struct Quant {float scale; int32_t zero;};
template<class T> T symbol(void *lib,const char *name){auto p=dlsym(lib,name);if(!p){fprintf(stderr,"missing model API\n");exit(2);}return reinterpret_cast<T>(p);}
int main(int argc,char **argv){
 if(argc!=3){fprintf(stderr,"usage: detector model input\n");return 2;}
 FILE *file=!strcmp(argv[2],"-")?stdin:fopen(argv[2],"rb");if(!file)return 2;
 const char *dir=getenv("ALISA_STOP_DIR");if(!dir||!*dir){fprintf(stderr,"ALISA_STOP_DIR is required\n");return 2;}
 std::string library=std::string(dir)+"/libtensorflowlite_c.so";
 void *lib=dlopen(library.c_str(),RTLD_NOW|RTLD_LOCAL);if(!lib){fprintf(stderr,"model library unavailable\n");return 2;}
 auto load=symbol<void*(*)(const char*)>(lib,"TfLiteModelCreateFromFile");
 auto create=symbol<void*(*)(const void*,const void*)>(lib,"TfLiteInterpreterCreate");
 auto alloc=symbol<int(*)(void*)>(lib,"TfLiteInterpreterAllocateTensors");
 auto input=symbol<void*(*)(void*,int)>(lib,"TfLiteInterpreterGetInputTensor");
 auto output=symbol<const void*(*)(const void*,int)>(lib,"TfLiteInterpreterGetOutputTensor");
 auto dim=symbol<int(*)(const void*,int)>(lib,"TfLiteTensorDim");
 auto type=symbol<int(*)(const void*)>(lib,"TfLiteTensorType");
 auto bytes=symbol<size_t(*)(const void*)>(lib,"TfLiteTensorByteSize");
 auto quant=symbol<Quant(*)(const void*)>(lib,"TfLiteTensorQuantizationParams");
 auto write_tensor=symbol<int(*)(void*,const void*,size_t)>(lib,"TfLiteTensorCopyFromBuffer");
 auto read_tensor=symbol<int(*)(const void*,void*,size_t)>(lib,"TfLiteTensorCopyToBuffer");
 auto invoke=symbol<int(*)(void*)>(lib,"TfLiteInterpreterInvoke");
 void *model=load(argv[1]);if(!model)return 3;void *interp=create(model,nullptr);if(!interp||alloc(interp))return 3;
 void *in=input(interp,0);const void *out=output(interp,0);if(!in||!out)return 3;
 auto iq=quant(in),oq=quant(out);int stride=dim(in,1),it=type(in),ot=type(out);
 if(stride<1||stride>20||iq.scale<=0||oq.scale<=0||(it!=3&&it!=9)||(ot!=3&&ot!=9)||bytes(in)!=size_t(stride*40)||bytes(out)!=1)return 4;
 FrontendConfig cfg{};FrontendFillConfigWithDefaults(&cfg);cfg.window.size_ms=30;cfg.window.step_size_ms=10;
 cfg.filterbank.num_channels=40;cfg.filterbank.lower_band_limit=125.0f;cfg.filterbank.upper_band_limit=7500.0f;
 cfg.noise_reduction.smoothing_bits=10;cfg.noise_reduction.even_smoothing=.025f;cfg.noise_reduction.odd_smoothing=.06f;cfg.noise_reduction.min_signal_remaining=.05f;
 cfg.pcan_gain_control.enable_pcan=1;cfg.pcan_gain_control.strength=.95f;cfg.pcan_gain_control.offset=80.0f;cfg.pcan_gain_control.gain_bits=21;
 cfg.log_scale.enable_log=1;cfg.log_scale.scale_shift=6;
 FrontendState state{};if(!FrontendPopulateState(&cfg,&state,16000))return 5;
 std::vector<uint8_t> features;features.reserve(stride*40);std::deque<float> scores;
 int16_t pcm[160];float best=0;bool detected=false;
 if(file==stdin){puts("ready");fflush(stdout);}
 while(fread(pcm,sizeof(int16_t),160,file)==160){
  size_t used=0;auto f=FrontendProcessSamples(&state,pcm,160,&used);if(!f.size)continue;
  if(f.size!=40||used!=160)return 6;
  for(size_t n=0;n<f.size;n++){
   int q=int(std::lrint(f.values[n]*.0390625f/iq.scale+iq.zero));
   q=it==9?std::clamp(q,-128,127):std::clamp(q,0,255);features.push_back(uint8_t(q));
  }
  if(features.size()<size_t(stride*40))continue;
  if(write_tensor(in,features.data(),features.size())||invoke(interp))return 7;features.clear();
  uint8_t raw=0;if(read_tensor(out,&raw,1))return 7;
  float score=((ot==9?int(int8_t(raw)):int(raw))-oq.zero)*oq.scale;scores.push_back(score);if(scores.size()>5)scores.pop_front();
  if(scores.size()==5){float mean=0;for(float x:scores)mean+=x;mean/=5;best=std::max(best,mean);if(mean>.5f){detected=true;if(file==stdin){puts("stop");fflush(stdout);break;}}}
 }
 if(file!=stdin){printf("detected=%d max_probability=%.3f\n",detected,best);fclose(file);}
 FrontendFreeStateContents(&state);return 0;
}
