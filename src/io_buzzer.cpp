#include "io_buzzer.h"

#include <Arduino.h>

#include "azimuth_hw.h"
#include "board_config.h"

namespace azimuth_io_buzzer {
namespace {
bool gEnabled = false;
uint32_t gToneUntilMs = 0;

struct MelodyNote {
  uint16_t freq_hz;
  uint16_t dur_ms;
};

// Tiny ascending "ding" — C5 E5 G5 C6
static const MelodyNote kFuncTune[] = {
    {523, 55},
    {659, 55},
    {784, 55},
    {1047, 90},
};

static bool gMelodyActive = false;
static uint8_t gMelodyIndex = 0;
static uint32_t gMelodyNoteEndMs = 0;
}  // namespace

void init() {
  gEnabled = azimuth_board::capabilities().hasBuzzer;
  if (!gEnabled) {
    return;
  }
  pinMode(azimuth_hw::kPinBuzzer, OUTPUT);
  digitalWrite(azimuth_hw::kPinBuzzer, LOW);
}

void chirp(uint16_t frequencyHz, uint16_t durationMs) {
  if (!gEnabled) {
    return;
  }
  gMelodyActive = false;
  tone(azimuth_hw::kPinBuzzer, frequencyHz, durationMs);
  gToneUntilMs = millis() + durationMs;
}

void playFuncButtonTune() {
  if (!gEnabled) {
    return;
  }
  noTone(azimuth_hw::kPinBuzzer);
  gMelodyActive = true;
  gMelodyIndex = 0;
  gToneUntilMs = 0;
  const MelodyNote& n = kFuncTune[0];
  tone(azimuth_hw::kPinBuzzer, n.freq_hz);
  gMelodyNoteEndMs = millis() + n.dur_ms;
}

void tick() {
  if (!gEnabled) {
    return;
  }

  if (gMelodyActive) {
    const uint32_t now = millis();
    if (now < gMelodyNoteEndMs) {
      return;
    }
    noTone(azimuth_hw::kPinBuzzer);
    gMelodyIndex++;
    if (gMelodyIndex >= sizeof(kFuncTune) / sizeof(kFuncTune[0])) {
      gMelodyActive = false;
      return;
    }
    const MelodyNote& n = kFuncTune[gMelodyIndex];
    tone(azimuth_hw::kPinBuzzer, n.freq_hz);
    gMelodyNoteEndMs = now + n.dur_ms;
    return;
  }

  if (gToneUntilMs == 0) {
    return;
  }
  if (millis() >= gToneUntilMs) {
    noTone(azimuth_hw::kPinBuzzer);
    gToneUntilMs = 0;
  }
}

}  // namespace azimuth_io_buzzer
