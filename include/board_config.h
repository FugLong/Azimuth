#pragma once

namespace azimuth_board {

enum class BoardId {
  Diy = 0,
  Pcb = 1,
};

struct Capabilities {
  bool hasRgb = false;
  bool hasBuzzer = false;
  bool hasFuncButton = false;
  bool hasBatterySense = false;
};

BoardId boardId();
const char* boardName();
Capabilities capabilities();

}  // namespace azimuth_board
