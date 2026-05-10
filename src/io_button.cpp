#include "io_button.h"

#include <Arduino.h>

#include "azimuth_hw.h"
#include "board_config.h"

namespace azimuth_io_button {
namespace {

bool gEnabled = false;
TapCallback gSingleCb = nullptr;
TapCallback gDoubleCb = nullptr;
TapCallback gLongPressCb = nullptr;

bool gStablePressed = false;
bool gLastRawPressed = false;
uint32_t gLastDebounceMs = 0;
constexpr uint32_t kDebounceMs = 25;

/** Second release must arrive within this window after the first release (same convention as UI double-click). */
constexpr uint32_t kDoubleTapGapMs = 400;
/** Hold this long → fire long-press once, swallow the next release. */
constexpr uint32_t kLongPressMs = 2000;

bool gAwaitingSecondTap = false;
uint32_t gFirstReleaseMs = 0;
uint32_t gSingleTapDeadlineMs = 0;
uint32_t gPressStartedMs = 0;
bool gLongPressFired = false;

void flushPendingSingleTap() {
  if (gSingleTapDeadlineMs == 0 || !gAwaitingSecondTap) {
    return;
  }
  if ((int32_t)(millis() - gSingleTapDeadlineMs) < 0) {
    return;
  }
  if (gSingleCb) {
    gSingleCb();
  }
  gAwaitingSecondTap = false;
  gSingleTapDeadlineMs = 0;
  gFirstReleaseMs = 0;
}

void onStableRelease() {
  // Long-press already consumed this gesture — don't fire single/double on the
  // same release, otherwise users get an unwanted pause toggle right after OTA
  // arms.
  if (gLongPressFired) {
    gLongPressFired = false;
    gAwaitingSecondTap = false;
    gFirstReleaseMs = 0;
    gSingleTapDeadlineMs = 0;
    return;
  }

  const uint32_t now = millis();
  if (gAwaitingSecondTap && gFirstReleaseMs != 0 &&
      (now - gFirstReleaseMs) < kDoubleTapGapMs) {
    if (gDoubleCb) {
      gDoubleCb();
    }
    gAwaitingSecondTap = false;
    gSingleTapDeadlineMs = 0;
    gFirstReleaseMs = 0;
    return;
  }

  gFirstReleaseMs = now;
  gAwaitingSecondTap = true;
  gSingleTapDeadlineMs = now + kDoubleTapGapMs;
}

}  // namespace

void init() {
  gEnabled = azimuth_board::capabilities().hasFuncButton;
  if (!gEnabled) {
    return;
  }
  pinMode(azimuth_hw::kPinButtonFunc, INPUT_PULLUP);
  gLastRawPressed = (digitalRead(azimuth_hw::kPinButtonFunc) == LOW);
  gStablePressed = gLastRawPressed;
  gAwaitingSecondTap = false;
  gFirstReleaseMs = 0;
  gSingleTapDeadlineMs = 0;
  gPressStartedMs = gStablePressed ? millis() : 0;
  gLongPressFired = false;
}

void setTapCallbacks(TapCallback singleTap, TapCallback doubleTap) {
  gSingleCb = singleTap;
  gDoubleCb = doubleTap;
}

void setLongPressCallback(TapCallback cb) {
  gLongPressCb = cb;
}

bool isPressed() {
  return gEnabled && gStablePressed;
}

void tick() {
  if (!gEnabled) {
    return;
  }

  flushPendingSingleTap();

  // Long-press is "still held N ms after stable press"; check before debouncing
  // so it fires even if the user keeps holding past the loop's idle path.
  if (gStablePressed && !gLongPressFired && gPressStartedMs != 0 &&
      (millis() - gPressStartedMs) >= kLongPressMs) {
    gLongPressFired = true;
    if (gLongPressCb) {
      gLongPressCb();
    }
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

  const bool wasPressed = gStablePressed;
  gStablePressed = rawPressed;

  if (!wasPressed && gStablePressed) {
    gPressStartedMs = millis();
  }
  if (wasPressed && !gStablePressed) {
    onStableRelease();
    gPressStartedMs = 0;
  }
}

}  // namespace azimuth_io_button
