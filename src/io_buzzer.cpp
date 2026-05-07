#include "io_buzzer.h"

#include <Arduino.h>
#include <math.h>
#include <esp32-hal-ledc.h>

#include "azimuth_hw.h"
#include "board_config.h"

namespace azimuth_io_buzzer {
namespace {

bool gEnabled = false;
uint32_t gToneUntilMs = 0;
uint8_t gVolumePct = 25;

/**
 * LEDC channel for the buzzer only. Must not share an LEDC timer with RGB PWM:
 * analogWrite on R/G/B takes channels 7,6,5 first; ch 4 shares timer 2 with ch 5 (blue) — using 4
 * corrupts RGB. Channel 0 uses timer 0, independent of 5–7.
 */
constexpr uint8_t kLedcChannel = 0;

struct MelodyNote {
  uint16_t freq_hz;
  uint16_t dur_ms;
};

static const MelodyNote kFuncTune[] = {
    {523, 55},
    {659, 55},
    {784, 55},
    {1047, 90},
};

/** Rising minor pattern — “heads up” without panic. */
static const MelodyNote kThermalWarnTune[] = {
    {587, 70},
    {622, 70},
    {659, 90},
    {622, 55},
};

/** Fast alternating fifths — unmistakable alarm. */
static const MelodyNote kThermalCriticalTune[] = {
    {784, 85},
    {523, 85},
    {784, 85},
    {523, 85},
    {392, 140},
};

static const MelodyNote kBatteryLowTune[] = {
    {698, 90},
    {659, 90},
    {587, 120},
};

static const MelodyNote kBatteryCriticalTune[] = {
    {988, 100},
    {740, 100},
    {988, 100},
    {622, 150},
};

/** Soft descending — “resting”. */
static const MelodyNote kStasisEnterTune[] = {
    {784, 45},
    {698, 45},
    {587, 55},
    {523, 70},
};

/** Ascending wake — “live again”. */
static const MelodyNote kStasisExitTune[] = {
    {523, 40},
    {659, 40},
    {784, 55},
    {988, 75},
};

static const MelodyNote* gMelodyNotes = nullptr;
static uint8_t gMelodyNoteCount = 0;

static bool gMelodyActive = false;
static uint8_t gMelodyIndex = 0;
static uint32_t gMelodyNoteEndMs = 0;

void stopBuzz() {
  if (!gEnabled) {
    return;
  }
  ledcWrite(kLedcChannel, 0);
  ledcDetachPin(azimuth_hw::kPinBuzzer);
}

/** Map portal 0–100 to PWM duty; gamma > 1 spreads perceived change across the slider (piezo is loud at mid duty). */
uint8_t dutyPercentFromUiVolume(uint8_t uiPct) {
  if (uiPct == 0) {
    return 0;
  }
  constexpr float kGamma = 2.35f;
  const float n = powf(static_cast<float>(uiPct) / 100.0f, kGamma);
  float eff = n * 100.0f;
  if (eff < 1.0f) {
    return 1;
  }
  if (eff > 100.0f) {
    return 100;
  }
  return static_cast<uint8_t>(eff + 0.5f);
}

/** Square-wave frequency + duty for volume. Do not use Arduino tone() here (same channel pool). */
void applyToneHz(uint16_t freqHz) {
  if (!gEnabled || freqHz == 0 || gVolumePct == 0) {
    return;
  }
  ledcAttachPin(azimuth_hw::kPinBuzzer, kLedcChannel);
  ledcWriteTone(kLedcChannel, freqHz);
  constexpr uint32_t kBaseDuty = 511;
  const uint8_t dutyPct = dutyPercentFromUiVolume(gVolumePct);
  uint32_t d = (kBaseDuty * static_cast<uint32_t>(dutyPct)) / 100U;
  if (d < 1U && dutyPct > 0U) {
    d = 1U;
  }
  ledcWrite(kLedcChannel, d);
}

void startMelody(const MelodyNote* notes, uint8_t count) {
  if (!gEnabled || gVolumePct == 0 || notes == nullptr || count == 0) {
    return;
  }
  stopBuzz();
  gMelodyNotes = notes;
  gMelodyNoteCount = count;
  gMelodyActive = true;
  gMelodyIndex = 0;
  gToneUntilMs = 0;
  applyToneHz(notes[0].freq_hz);
  gMelodyNoteEndMs = millis() + notes[0].dur_ms;
}

}  // namespace

void init() {
  gEnabled = azimuth_board::capabilities().hasBuzzer;
  if (!gEnabled) {
    return;
  }
  pinMode(azimuth_hw::kPinBuzzer, OUTPUT);
  digitalWrite(azimuth_hw::kPinBuzzer, LOW);
}

void setVolumePercent(uint8_t percent) {
  if (percent > 100) {
    percent = 100;
  }
  gVolumePct = percent;
}

bool isActive() {
  if (!gEnabled) {
    return false;
  }
  return gMelodyActive || gToneUntilMs != 0;
}

void chirp(uint16_t frequencyHz, uint16_t durationMs) {
  if (!gEnabled || gVolumePct == 0) {
    return;
  }
  gMelodyActive = false;
  gMelodyNotes = nullptr;
  gMelodyNoteCount = 0;
  applyToneHz(frequencyHz);
  gToneUntilMs = millis() + durationMs;
}

void playFuncButtonTune() {
  startMelody(kFuncTune, sizeof(kFuncTune) / sizeof(kFuncTune[0]));
}

void playThermalWarnTune() {
  startMelody(kThermalWarnTune, sizeof(kThermalWarnTune) / sizeof(kThermalWarnTune[0]));
}

void playThermalCriticalTune() {
  startMelody(kThermalCriticalTune,
              sizeof(kThermalCriticalTune) / sizeof(kThermalCriticalTune[0]));
}

void playBatteryLowTune() {
  startMelody(kBatteryLowTune, sizeof(kBatteryLowTune) / sizeof(kBatteryLowTune[0]));
}

void playBatteryCriticalTune() {
  startMelody(kBatteryCriticalTune,
              sizeof(kBatteryCriticalTune) / sizeof(kBatteryCriticalTune[0]));
}

void playBatteryPanicPulse() {
  chirp(1900, 140);
}

void playStasisEnterTune() {
  startMelody(kStasisEnterTune, sizeof(kStasisEnterTune) / sizeof(kStasisEnterTune[0]));
}

void playStasisExitTune() {
  startMelody(kStasisExitTune, sizeof(kStasisExitTune) / sizeof(kStasisExitTune[0]));
}

void tick() {
  if (!gEnabled) {
    return;
  }

  if (gMelodyActive && gMelodyNotes != nullptr && gMelodyNoteCount > 0) {
    const uint32_t now = millis();
    if (now < gMelodyNoteEndMs) {
      return;
    }
    stopBuzz();
    gMelodyIndex++;
    if (gMelodyIndex >= gMelodyNoteCount) {
      gMelodyActive = false;
      gMelodyNotes = nullptr;
      gMelodyNoteCount = 0;
      return;
    }
    const MelodyNote& n = gMelodyNotes[gMelodyIndex];
    applyToneHz(n.freq_hz);
    gMelodyNoteEndMs = now + n.dur_ms;
    return;
  }

  if (gToneUntilMs == 0) {
    return;
  }
  if (millis() >= gToneUntilMs) {
    stopBuzz();
    gToneUntilMs = 0;
  }
}

}  // namespace azimuth_io_buzzer
