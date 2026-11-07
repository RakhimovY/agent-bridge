#include "match.hpp"
#include <cassert>
#include <cmath>
int main() {
  WakeFrame a{}, b{}, c{};
  a[0] = 1;
  b[1] = 1;
  c[0] = 1;
  c[1] = 1;
  assert(std::abs(wake_distance({a, b}, {a, b})) < 0.0001f);
  assert(wake_distance({a}, {b}) > .49f);
  assert(wake_distance({a, a, b}, {a, b}) < .0001f);
  assert(wake_distance({}, {a}) > 1.0f);
  assert(wake_matches({a, b}, {{a, b}}, .05f));
  assert(!wake_matches({a, b}, {{b, a}}, .05f));
}
