#pragma once
#include <algorithm>
#include <array>
#include <cmath>
#include <limits>
#include <vector>
using WakeFrame = std::array<float, 40>;
using WakeFeatures = std::vector<WakeFrame>;
inline float wake_distance(const WakeFeatures &a, const WakeFeatures &b) {
  if (a.empty() || b.empty())
    return std::numeric_limits<float>::infinity();
  const float inf = std::numeric_limits<float>::infinity();
  std::vector<float> prev(b.size() + 1, inf), next(b.size() + 1, inf);
  prev[0] = 0;
  for (const auto &x : a) {
    next[0] = inf;
    for (size_t j = 0; j < b.size(); ++j) {
      float dot = 0, xx = 0, yy = 0;
      for (size_t k = 0; k < x.size(); ++k) {
        dot += x[k] * b[j][k];
        xx += x[k] * x[k];
        yy += b[j][k] * b[j][k];
      }
      float similarity = xx > 0 && yy > 0 ? dot / std::sqrt(xx * yy) : 0;
      float cost = 1 - std::clamp(similarity, -1.0f, 1.0f);
      next[j + 1] = cost + std::min({prev[j], prev[j + 1], next[j]});
    }
    prev.swap(next);
  }
  return prev.back() / float(a.size() + b.size());
}
inline bool wake_matches(const WakeFeatures &sample,
                         const std::vector<WakeFeatures> &refs,
                         float threshold) {
  for (const auto &ref : refs)
    if (wake_distance(sample, ref) < threshold)
      return true;
  return false;
}
