#pragma once
#include <cstdlib>
#include <fcntl.h>
#include <unistd.h>
inline int wake_mute_open() {
  const char *path = std::getenv("DOT_TEST_MUTE_FILE");
  return path ? open(path, O_RDONLY) : -1;
}
inline bool wake_unmuted(int fd) {
  char state;
  return lseek(fd, 0, SEEK_SET) == 0 && read(fd, &state, 1) == 1 && state == '0';
}
