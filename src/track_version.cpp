#include "track_version.h"

#include <cstdio>

namespace azimuth_version {

bool parseSemVer(const char* s, SemVer& out) {
  out = {};
  if (!s || !*s) {
    return false;
  }
  if (s[0] == 'v' || s[0] == 'V') {
    ++s;
  }
  const int n = std::sscanf(s, "%d.%d.%d", &out.ma, &out.mi, &out.pa);
  if (n == 1) {
    out.mi = 0;
    out.pa = 0;
  } else if (n == 2) {
    out.pa = 0;
  }
  return n >= 1;
}

bool semverLess(const SemVer& a, const SemVer& b) {
  if (a.ma != b.ma) {
    return a.ma < b.ma;
  }
  if (a.mi != b.mi) {
    return a.mi < b.mi;
  }
  return a.pa < b.pa;
}

}  // namespace azimuth_version
