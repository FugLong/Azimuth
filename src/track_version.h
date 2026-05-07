#pragma once

namespace azimuth_version {

struct SemVer {
  int ma = 0;
  int mi = 0;
  int pa = 0;
};

bool parseSemVer(const char* s, SemVer& out);
bool semverLess(const SemVer& a, const SemVer& b);

}  // namespace azimuth_version
