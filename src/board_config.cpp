#include "board_config.h"

namespace azimuth_board {

BoardId boardId() {
#if defined(AZIMUTH_BOARD_PCB) && AZIMUTH_BOARD_PCB
  return BoardId::Pcb;
#else
  return BoardId::Diy;
#endif
}

const char* boardName() {
  return (boardId() == BoardId::Pcb) ? "pcb" : "diy";
}

Capabilities capabilities() {
  Capabilities c;
  if (boardId() == BoardId::Pcb) {
    c.hasRgb = true;
    c.hasBuzzer = true;
    c.hasFuncButton = true;
    c.hasBatterySense = true;
  } else {
    c.hasRgb = false;
    c.hasBuzzer = true;
    c.hasFuncButton = true;
    c.hasBatterySense = true;
  }
  return c;
}

}  // namespace azimuth_board
