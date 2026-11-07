#pragma once
#include <fcntl.h>
#include <linux/input.h>
#include <cstdio>
#include <cstring>
#include <sys/ioctl.h>
#include <unistd.h>

inline int wake_mute_open() {
  for (int i = 0; i < 32; ++i) {
    char path[32];
    std::snprintf(path, sizeof(path), "/dev/input/event%d", i);
    int fd = open(path, O_RDONLY | O_CLOEXEC);
    if (fd < 0) continue;
    char name[80] = {};
    unsigned long bits[(KEY_MAX + 8 * sizeof(long)) / (8 * sizeof(long))] = {};
    if (ioctl(fd, EVIOCGNAME(sizeof(name)), name) >= 0 && !std::strcmp(name, "gpio-keys-filter") &&
        ioctl(fd, EVIOCGBIT(EV_KEY, sizeof(bits)), bits) >= 0 &&
        (bits[KEY_MUTE / (8 * sizeof(long))] & (1UL << (KEY_MUTE % (8 * sizeof(long)))))) return fd;
    close(fd);
  }
  return -1;
}

inline bool wake_unmuted(int fd) {
  unsigned long bits[(KEY_MAX + 8 * sizeof(long)) / (8 * sizeof(long))] = {};
  if (ioctl(fd, EVIOCGKEY(sizeof(bits)), bits) < 0) return false;
  return !(bits[KEY_MUTE / (8 * sizeof(long))] & (1UL << (KEY_MUTE % (8 * sizeof(long)))));
}
