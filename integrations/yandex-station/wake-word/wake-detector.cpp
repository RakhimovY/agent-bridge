// Optional, experimental wake-word detector. See README.md before use.
// Frontend parameters follow pymicro-features 2.0.2 (Apache-2.0); see
// NOTICE.md.
#include "../../../shared/wake/match.hpp"
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <deque>
#include <fcntl.h>
#include <fstream>
#include <poll.h>
#include <signal.h>
#include <sstream>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/un.h>
#include <sys/wait.h>
#include <unistd.h>
extern "C" {
#include "tensorflow/lite/experimental/microfrontend/lib/frontend.h"
#include "tensorflow/lite/experimental/microfrontend/lib/frontend_util.h"
}
struct SocketLease {
  int fd = -1;
  const char *path = nullptr;
  struct stat owner{};
  bool bound = false;
  ~SocketLease() {
    if (fd >= 0)
      close(fd);
    struct stat now{};
    if (bound && path && !lstat(path, &now) && now.st_ino == owner.st_ino &&
        now.st_dev == owner.st_dev)
      unlink(path);
  }
};
static volatile sig_atomic_t running = 1;
static void stop(int) { running = 0; }
static WakeFeatures load(const char *path) {
  WakeFeatures out;
  FILE *f = fopen(path, "r");
  if (!f)
    return out;
  WakeFrame row{};
  for (;;) {
    int n = 0;
    for (; n < 40; n++)
      if (fscanf(f, n ? ",%f" : "%f", &row[n]) != 1)
        break;
    if (n != 40)
      break;
    out.push_back(row);
  }
  fclose(f);
  return out;
}
static bool mic_allowed() {
  const char *p = getenv("DOT_WAKE_MUTE_FILE");
  if (!p)
    return false;
  std::ifstream f(p);
  std::string s;
  while (std::getline(f, s))
    if (s.find("|mute-switch ") != std::string::npos) {
      std::istringstream tokens(s);
      while (tokens >> s)
        if (s == "hi")
          return true;
      return false;
    }
  return false;
}
static WakeFeatures features(const std::vector<int16_t> &pcm) {
  std::vector<int> peaks(pcm.size() / 160);
  int overall = 0;
  for (size_t i = 0; i < peaks.size(); i++)
    for (int j = 0; j < 160; j++)
      peaks[i] = std::max(peaks[i], std::abs(int(pcm[i * 160 + j])));
  for (int x : peaks)
    overall = std::max(overall, x);
  int cut = std::max(100, int(overall * .05f));
  size_t first = peaks.size(), last = 0;
  for (size_t i = 0; i < peaks.size(); i++)
    if (peaks[i] > cut) {
      first = std::min(first, i);
      last = i;
    }
  if (first == peaks.size())
    return {};
  first = first > 8 ? first - 8 : 0;
  last = std::min(peaks.size(), last + 8);
  FrontendConfig cfg{};
  FrontendFillConfigWithDefaults(&cfg);
  cfg.window.size_ms = 30;
  cfg.window.step_size_ms = 10;
  cfg.filterbank.num_channels = 40;
  cfg.filterbank.lower_band_limit = 125;
  cfg.filterbank.upper_band_limit = 7500;
  cfg.noise_reduction.smoothing_bits = 10;
  cfg.noise_reduction.even_smoothing = .025f;
  cfg.noise_reduction.odd_smoothing = .06f;
  cfg.noise_reduction.min_signal_remaining = .05f;
  cfg.pcan_gain_control.enable_pcan = 1;
  cfg.pcan_gain_control.strength = .95f;
  cfg.pcan_gain_control.offset = 80;
  cfg.pcan_gain_control.gain_bits = 21;
  cfg.log_scale.enable_log = 1;
  cfg.log_scale.scale_shift = 6;
  FrontendState state{};
  if (!FrontendPopulateState(&cfg, &state, 16000))
    return {};
  WakeFeatures out;
  for (size_t i = first; i < last; i++) {
    size_t used = 0;
    auto f = FrontendProcessSamples(&state, pcm.data() + i * 160, 160, &used);
    if (!f.size)
      continue;
    if (f.size != 40 || used != 160)
      break;
    float mean = 0, norm = 0;
    WakeFrame row{};
    for (size_t k = 0; k < 40; k++)
      mean += f.values[k];
    mean /= 40;
    for (size_t k = 0; k < 40; k++) {
      row[k] = f.values[k] - mean;
      norm += row[k] * row[k];
    }
    norm = std::sqrt(norm);
    if (norm < 1e-9f)
      norm = 1;
    for (float &x : row)
      x /= norm;
    out.push_back(row);
  }
  FrontendFreeStateContents(&state);
  return out;
}
int main(int argc, char **argv) {
  if (argc == 3 && !strcmp(argv[1], "features")) {
    FILE *f = fopen(argv[2], "rb");
    if (!f)
      return 1;
    std::vector<int16_t> pcm;
    int16_t block[160];
    size_t n;
    while ((n = fread(block, sizeof(int16_t), 160, f)) == 160) {
      pcm.insert(pcm.end(), block, block + n);
      if (pcm.size() > 160000) {
        fclose(f);
        return 1;
      }
    }
    fclose(f);
    auto out = features(pcm);
    if (out.empty())
      return 1;
    for (const auto &row : out) {
      for (int j = 0; j < 40; j++)
        printf("%s%.7g", j ? "," : "", row[j]);
      puts("");
    }
    return 0;
  }
  if (argc == 5 && !strcmp(argv[1], "compare")) {
    auto a = load(argv[2]), b = load(argv[3]);
    if (a.empty() || b.empty())
      return 2;
    float d = wake_distance(a, b);
    float threshold = strtof(argv[4], nullptr);
    printf("distance=%.6f match=%d\n", d, d < threshold);
    return 0;
  }
  bool enroll = argc == 4 && !strcmp(argv[1], "enroll");
  bool listen = argc >= 6 && !strcmp(argv[1], "listen");
  if (!enroll && !listen)
    return 1;
  --argc;
  ++argv;
  std::vector<WakeFeatures> refs;
  float cutoff = 0;
  if (listen) {
    auto ref = load(argv[2]);
    if (ref.empty())
      return 2;
    refs.push_back(ref);
    for (int i = 5; i < argc; i++) {
      auto extra = load(argv[i]);
      if (extra.empty())
        return 2;
      refs.push_back(extra);
    }
    cutoff = strtof(argv[3], nullptr);
    if (!std::isfinite(cutoff) || cutoff <= 0 || cutoff >= 1)
      return 2;
  }
  SocketLease lease;
  int &fd = lease.fd;
  fd = socket(AF_UNIX, SOCK_DGRAM, 0);
  if (fd < 0)
    return 3;
  sockaddr_un address{};
  address.sun_family = AF_UNIX;
  if (strlen(argv[1]) >= sizeof(address.sun_path))
    return 3;
  strcpy(address.sun_path, argv[1]);
  if (bind(fd, (sockaddr *)&address, sizeof(address)))
    return 3;
  lease.path = argv[1];
  if (lstat(argv[1], &lease.owner))
    return 3;
  lease.bound = true;
  struct stat current{};
  signal(SIGINT, stop);
  signal(SIGTERM, stop);
  if (enroll) {
    puts("ready");
    fflush(stdout);
  }
  std::deque<int16_t> preroll;
  std::vector<int16_t> speech;
  unsigned silence = 0;
  bool active = false;
  while (running) {
    pollfd p{fd, POLLIN, 0};
    int ready = poll(&p, 1, 500);
    if (!running)
      break;
    if (lstat(argv[1], &current) || current.st_ino != lease.owner.st_ino ||
        current.st_dev != lease.owner.st_dev)
      break;
    if (ready <= 0)
      continue;
    int16_t frame[160];
    ssize_t n = recv(fd, frame, sizeof(frame), 0);
    if (n != sizeof(frame))
      continue;
    if (!mic_allowed()) {
      preroll.clear();
      speech.clear();
      active = false;
      silence = 0;
      continue;
    }
    int peak = 0;
    for (auto v : frame)
      peak = std::max(peak, std::abs(int(v)));
    if (!active) {
      for (auto v : frame)
        preroll.push_back(v);
      while (preroll.size() > 1280)
        preroll.pop_front();
      if (peak > 60) {
        active = true;
        speech.assign(preroll.begin(), preroll.end());
        preroll.clear();
        silence = 0;
      }
      continue;
    }
    speech.insert(speech.end(), frame, frame + 160);
    if (peak > 60)
      silence = 0;
    else
      silence++;
    if (speech.size() > 80000) {
      speech.clear();
      active = false;
      silence = 0;
      continue;
    }
    if (silence >= 30) {
      auto f = features(speech);
      if (enroll && !f.empty()) {
        int out = open(argv[2], O_WRONLY | O_CREAT | O_EXCL, 0600);
        if (out < 0)
          return 4;
        FILE *stream = fdopen(out, "w");
        if (!stream) {
          close(out);
          return 4;
        }
        for (const auto &row : f) {
          for (int j = 0; j < 40; j++)
            fprintf(stream, "%s%.7g", j ? "," : "", row[j]);
          fputc('\n', stream);
        }
        if (fclose(stream))
          return 4;
        puts("saved");
        fflush(stdout);
        running = 0;
      }
      if (listen && wake_matches(f, refs, cutoff)) {
        pid_t child = fork();
        if (child == 0) {
          execl(argv[4], argv[4], "open", (char *)nullptr);
          _exit(127);
        }
        if (child > 0)
          waitpid(child, nullptr, 0);
      }
      speech.clear();
      active = false;
      silence = 0;
    }
  }
  return 0;
}
