#include <cassert>

#include "../src/track_version.h"

using azimuth_version::SemVer;
using azimuth_version::parseSemVer;
using azimuth_version::semverLess;

int main() {
  SemVer v{};
  assert(parseSemVer("1", v));
  assert(v.ma == 1 && v.mi == 0 && v.pa == 0);

  assert(parseSemVer("1.2", v));
  assert(v.ma == 1 && v.mi == 2 && v.pa == 0);

  assert(parseSemVer("v1.2.3", v));
  assert(v.ma == 1 && v.mi == 2 && v.pa == 3);

  assert(!parseSemVer("", v));
  assert(!parseSemVer(nullptr, v));

  const SemVer a{1, 2, 3};
  const SemVer b{1, 3, 0};
  const SemVer c{2, 0, 0};
  assert(semverLess(a, b));
  assert(semverLess(b, c));
  assert(!semverLess(c, a));
  assert(!semverLess(a, a));

  return 0;
}
