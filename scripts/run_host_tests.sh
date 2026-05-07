#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
OUT_DIR="${ROOT}/.tmp-host-tests"
mkdir -p "${OUT_DIR}"

g++ -std=c++17 -Wall -Wextra -pedantic -Isrc \
  tests/track_config_validation_test.cpp \
  src/track_config_validation.cpp \
  -o "${OUT_DIR}/track_config_validation_test"

g++ -std=c++17 -Wall -Wextra -pedantic -Isrc \
  tests/track_config_plan_test.cpp \
  src/track_config_plan.cpp \
  src/track_config_validation.cpp \
  -o "${OUT_DIR}/track_config_plan_test"

g++ -std=c++17 -Wall -Wextra -pedantic -Isrc \
  tests/track_version_test.cpp \
  src/track_version.cpp \
  -o "${OUT_DIR}/track_version_test"

"${OUT_DIR}/track_config_validation_test"
"${OUT_DIR}/track_config_plan_test"
"${OUT_DIR}/track_version_test"

echo "Host tests passed."
