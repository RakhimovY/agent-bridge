// Experimental local keyword listener. It emits a signal; it never starts a call.
#include <sherpa-onnx/c-api/c-api.h>
#include <dot/wake_mute.h>
#include <algorithm>
#include <csignal>
#include <cstdlib>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <deque>
#include <string>
#include <vector>
#include <poll.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/un.h>
#include <unistd.h>

static volatile sig_atomic_t running = 1;
static volatile sig_atomic_t bind_permitted = 1;
static void stop(int) { running = 0; }
static void resume(int) { bind_permitted = 1; }

int main(int argc, char **argv) {
  // socket followed by one or more groups: keywords, encoder, decoder, joiner, tokens
  if (argc < 7 || (argc - 2) % 5) return 2;
  const char *prewarm = std::getenv("DOT_WAKE_PREWARM");
  if (prewarm && !std::strcmp(prewarm, "1")) bind_permitted = 0;
  std::signal(SIGTERM, stop);
  std::signal(SIGINT, stop);
  std::signal(SIGUSR1, resume);
  int mute_fd = wake_mute_open();
  if (mute_fd < 0) { std::fputs("Mute input unavailable\n", stderr); return 5; }
  std::vector<const SherpaOnnxKeywordSpotter *> spotters;
  for (int i = 2; i < argc; i += 5) {
    SherpaOnnxKeywordSpotterConfig c{};
    c.feat_config.sample_rate = 16000;
    c.feat_config.feature_dim = 80;
    c.model_config.transducer.encoder = argv[i + 1];
    c.model_config.transducer.decoder = argv[i + 2];
    c.model_config.transducer.joiner = argv[i + 3];
    c.model_config.tokens = argv[i + 4];
    c.model_config.num_threads = 1;
    c.model_config.provider = "cpu";
    c.model_config.model_type = "zipformer2";
    c.max_active_paths = 4;
    c.num_trailing_blanks = 1;
    c.keywords_score = 1;
    c.keywords_threshold = 0.25;
    c.keywords_file = argv[i];
    const auto *spotter = SherpaOnnxCreateKeywordSpotter(&c);
    if (!spotter) return 3;
    spotters.push_back(spotter);
  }
  if (!bind_permitted) {
    puts("WARM");
    fflush(stdout);
    while (running && !bind_permitted) usleep(100000);
  }
  if (!running) return 1;
  int fd = socket(AF_UNIX, SOCK_DGRAM, 0);
  sockaddr_un address{};
  address.sun_family = AF_UNIX;
  if (fd < 0 || strlen(argv[1]) >= sizeof(address.sun_path)) return 4;
  strcpy(address.sun_path, argv[1]);
  if (bind(fd, reinterpret_cast<sockaddr *>(&address), sizeof(address))) return 4;
  struct stat owner{}, current{};
  if (lstat(argv[1], &owner)) return 4;
  puts("READY");
  fflush(stdout);
  std::deque<int16_t> preroll;
  std::vector<int16_t> voice;
  int silence = 0;
  int16_t frame[160], packet[160];
  size_t filled = 0;
  bool matched = false;
  std::string matched_key;
  while (running && !matched) {
    pollfd input{fd, POLLIN, 0};
    int ready = poll(&input, 1, 500);
    if (lstat(argv[1], &current) || current.st_ino != owner.st_ino || current.st_dev != owner.st_dev) break;
    if (ready <= 0) continue;
    ssize_t n = recv(fd, packet, sizeof(packet), 0);
    if (n <= 0) break;
    if (n % sizeof(int16_t)) continue;
    if (!wake_unmuted(mute_fd)) { preroll.clear(); voice.clear(); filled = 0; silence = 0; continue; }
    for (size_t j = 0; j < static_cast<size_t>(n) / sizeof(int16_t); ++j) {
      frame[filled++] = packet[j];
      if (filled < 160) continue;
      filled = 0;
      int peak = 0;
      for (auto sample : frame) peak = std::max(peak, std::abs(int(sample)));
      if (voice.empty()) {
        for (auto sample : frame) preroll.push_back(sample);
        while (preroll.size() > 1280) preroll.pop_front();
        if (peak > 60) { voice.assign(preroll.begin(), preroll.end()); preroll.clear(); }
        continue;
      }
      voice.insert(voice.end(), frame, frame + 160);
      silence = peak > 60 ? 0 : silence + 1;
      if (silence < 30 && voice.size() < 80000) continue;
      std::vector<float> samples;
      samples.reserve(voice.size() + 16000);
      for (auto sample : voice) samples.push_back(sample / 32768.0f);
      samples.resize(samples.size() + 16000, 0);
      for (auto *spotter : spotters) {
        const auto *stream = SherpaOnnxCreateKeywordStream(spotter);
        SherpaOnnxOnlineStreamAcceptWaveform(stream, 16000, samples.data(), samples.size());
        SherpaOnnxOnlineStreamInputFinished(stream);
        while (SherpaOnnxIsKeywordStreamReady(spotter, stream)) {
          SherpaOnnxDecodeKeywordStream(spotter, stream);
          const auto *result = SherpaOnnxGetKeywordResult(spotter, stream);
          if (result && result->keyword) {
            std::string key(result->keyword);
            if (key.size() >= 2 && key.size() <= 12 && key[0] == 'K' &&
                std::all_of(key.begin() + 1, key.end(), [](char c) { return c >= '0' && c <= '9'; })) {
              matched_key = key;
              matched = true;
            }
          }
          if (result) SherpaOnnxDestroyKeywordResult(result);
          if (matched) break;
        }
        SherpaOnnxDestroyOnlineStream(stream);
        if (matched) break;
      }
      voice.clear();
      silence = 0;
      if (matched) break;
    }
  }
  close(fd);
  bool owns_socket = !lstat(argv[1], &current) && current.st_ino == owner.st_ino && current.st_dev == owner.st_dev;
  if (owns_socket) unlink(argv[1]);
  for (auto *spotter : spotters) SherpaOnnxDestroyKeywordSpotter(spotter);
  bool unmuted = wake_unmuted(mute_fd);
  close(mute_fd);
  if (matched && running && owns_socket && unmuted) { std::printf("MATCH %s\n", matched_key.c_str()); return 0; }
  return 1;
}
