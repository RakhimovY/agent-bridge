#include "connecting_capture.hpp"
#include <sys/socket.h>
#include <sys/un.h>
#include <atomic>
#include <chrono>
#include <cstdio>
#include <cstring>
#include <thread>
#include <unistd.h>
#include <sys/stat.h>
int main(){
 std::string path="/tmp/ab-ring-"+std::to_string(getpid());std::atomic<bool> mute{true}, cancel{false};
 int fd=socket(AF_UNIX,SOCK_DGRAM,0);sockaddr_un a{};a.sun_family=AF_UNIX;std::strcpy(a.sun_path,path.c_str());
 int16_t pcm[160];for(auto &v:pcm)v=3210;
 auto send=[&]{return sendto(fd,pcm,sizeof pcm,0,(sockaddr*)&a,sizeof a);};
 int replacement=-1;
 {
  ConnectingCapture capture(path,[&]{return !mute.load();},[&](const int16_t *v,size_t n){for(size_t i=0;i<n;i++)if(v[i]==3210)cancel=true;});
  for(int i=0;i<10;i++){send();std::this_thread::sleep_for(std::chrono::milliseconds(20));}
  if(cancel){std::fputs("FAIL: captured muted audio\n",stderr);return 1;}
  mute=false;
  for(int i=0;i<30&&!cancel;i++){send();std::this_thread::sleep_for(std::chrono::milliseconds(20));}
  if(!cancel){std::fputs("FAIL: no stop signal while connection pending\n",stderr);return 1;}
  unlink(path.c_str());replacement=socket(AF_UNIX,SOCK_DGRAM,0);
  if(bind(replacement,(sockaddr*)&a,sizeof a)){std::fputs("FAIL: could not replace socket\n",stderr);return 1;}
 }
 struct stat info{};if(lstat(path.c_str(),&info)){std::fputs("FAIL: handoff removed new microphone socket\n",stderr);return 1;}
 close(replacement);close(fd);unlink(path.c_str());std::puts("PASS: muted input, stop while connecting, socket handoff");
}
