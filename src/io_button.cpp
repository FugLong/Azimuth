#include "io_button.h"

#include <Arduino.h>

#include "azimuth_hw.h"
#include "board_config.h"

namespace azimuth_io_button {
namespace {
bool gEnabled = false;
ButtonPressCallback gCallback = nullptr;
bool gStablePressed = false;
bool gLastRawPressed = false;
uint32_t gLastDebounceMs = 0;
constexpr uint32_t kDebounceMs = 25;
}  // namespace

void init() {
  gEnabled = azimuth_board::capabilities().hasFuncButton;
  if (!gEnabled) {
    return;
  }
  pinMode(azimuth_hw::kPinButtonFunc, INPUT_PULLUP);
  gLastRawPressed = (digitalRead(azimuth_hw::kPinButtonFunc) == LOW);
  gStablePressed = gLastRawPressed;
}

void setPressCallback(ButtonPressCallback cb) {
  gCallback = cb;
}

bool isPressed() {
  return gEnabled && gStablePressed;
}

void tick() {
  if (!gEnabled) {
    return;
  }

  const bool rawPressed = (digitalRead(azimuth_hw::kPinButtonFunc) == LOW);
  if (rawPressed != gLastRawPressed) {
    gLastRawPressed = rawPressed;
    gLastDebounceMs = millis();
    return;
  }

  if (millis() - gLastDebounceMs < kDebounceMs) {
    return;
  }

  if (gStablePressed == rawPressed) {
    return;
  }

  gStablePressed = rawPressed;
  if (gStablePressed && gCallback) {
    gCallback();
  }
}

}  // namespace azimuth_io_button
