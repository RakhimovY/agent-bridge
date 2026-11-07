#pragma once
#include <cstdint>
#include <cstring>
#include <cstdio>
#include <unistd.h>
struct SherpaOnnxOnlineTransducerModelConfig { const char *encoder, *decoder, *joiner; };
struct SherpaOnnxOnlineModelConfig { SherpaOnnxOnlineTransducerModelConfig transducer; const char *tokens; int num_threads; const char *provider, *model_type; };
struct SherpaOnnxFeatureConfig { int sample_rate, feature_dim; };
struct SherpaOnnxKeywordSpotterConfig { SherpaOnnxFeatureConfig feat_config; SherpaOnnxOnlineModelConfig model_config; int max_active_paths, num_trailing_blanks; float keywords_score, keywords_threshold; const char *keywords_file; };
struct SherpaOnnxKeywordSpotter { bool enabled, slow; };
struct SherpaOnnxOnlineStream { bool enabled, slow, match=false, ready=true; };
struct SherpaOnnxKeywordResult { const char *keyword; };
inline const SherpaOnnxKeywordSpotter *SherpaOnnxCreateKeywordSpotter(const SherpaOnnxKeywordSpotterConfig *c) { return new SherpaOnnxKeywordSpotter{std::strcmp(c->keywords_file, "negative") != 0, std::strcmp(c->keywords_file, "slow") == 0}; }
inline void SherpaOnnxDestroyKeywordSpotter(const SherpaOnnxKeywordSpotter *p) { delete p; }
inline const SherpaOnnxOnlineStream *SherpaOnnxCreateKeywordStream(const SherpaOnnxKeywordSpotter *p) { return new SherpaOnnxOnlineStream{p->enabled,p->slow}; }
inline void SherpaOnnxDestroyOnlineStream(const SherpaOnnxOnlineStream *p) { delete p; }
inline void SherpaOnnxOnlineStreamAcceptWaveform(const SherpaOnnxOnlineStream *p, int, const float *a, int n) { for(int i=0;i<n;i++) if(p->enabled && a[i]>0.24f) const_cast<SherpaOnnxOnlineStream*>(p)->match=true; }
inline void SherpaOnnxOnlineStreamInputFinished(const SherpaOnnxOnlineStream *) {}
inline int SherpaOnnxIsKeywordStreamReady(const SherpaOnnxKeywordSpotter *, const SherpaOnnxOnlineStream *p) { return p->ready; }
inline void SherpaOnnxDecodeKeywordStream(const SherpaOnnxKeywordSpotter *, const SherpaOnnxOnlineStream *p) { if(p->slow){std::fputs("DECODING\n",stderr);std::fflush(stderr);sleep(1);} const_cast<SherpaOnnxOnlineStream*>(p)->ready=false; }
inline const SherpaOnnxKeywordResult *SherpaOnnxGetKeywordResult(const SherpaOnnxKeywordSpotter *, const SherpaOnnxOnlineStream *p) { return new SherpaOnnxKeywordResult{p->match?"K0":""}; }
inline void SherpaOnnxDestroyKeywordResult(const SherpaOnnxKeywordResult *p) { delete p; }
