# Prototype V0.1 Initial Testing and Insights

## Scope
This note captures first-article bring-up observations for the `Azimuth_Design` custom PCB (`ESP32-C3-WROOM-02`) and immediate actions to get prototype units flashable and testable.

## Current Observations
- No shorts found during continuity check.
- Board powers without obvious overcurrent or heating.
- Buzzer repeats a deterministic rhythm and charge LED flashes in sync.
- Mac enumerates device as Espressif native USB:
  - `USB JTAG/serial debug unit`
  - `/dev/cu.usbmodem1101`
  - VID/PID `0x303A/0x1001`

Interpretation: the MCU and USB PHY are alive. This is not a dead-board signature.

## Most Likely Root Cause (V0.1)
Boot/download strap sensitivity on `GPIO8/GPIO9` during reset:
- `GPIO9` is a strapping pin and also connected to IMU `H_MISO`.
- `GPIO8` is a strapping pin and also connected to IMU `H_SCK`.
- If either line is in a bad state at reset sample time, download mode entry can fail or be inconsistent.

Additional design behavior that can increase variability:
- `RST1` controls LDO `EN` (power-cycles full 3V3 domain), instead of directly resetting MCU `EN`.
- This means MCU and IMU ramp together on reset, increasing chance of transient strap interactions.

## Why This Can Look Worse Than It Is
- Flash entry can fail while runtime hardware is still mostly correct.
- A board can be hard to program but still run normally once a valid image is in flash.
- Rhythmic buzzer/LED can be caused by repeated reboot/boot-state cycling.

## Immediate Bring-Up Procedure (Current Hardware)
1. Connect USB and confirm enumeration (`/dev/cu.usbmodem*` appears).
2. Force download straps during reset:
   - Hold `GPIO9` low.
   - Ensure `GPIO8` is high.
   - Toggle reset/power (for this rev, reset behavior is tied to regulator enable path).
3. Keep `GPIO9` low briefly through reset edge, then attempt flashing.
4. If flash succeeds once, power-cycle and test normal boot repeatedly without forcing straps.
5. If normal boot is stable, proceed with firmware functional tests.

## Known Good/Expected USB Identity
Seeing Espressif `USB JTAG/serial debug unit` on macOS is expected for ESP32-C3 native USB in this state and is useful for Web Serial/Web flasher flow.

## Firmware/Flasher Actions Planned
- Keep `DIY` flow as default and unchanged.
- Add explicit `Custom PCB` option in web flasher UI.
- Publish manifests/artifacts for both:
  - `azimuth_main_diy`
  - `azimuth_main_pcb`
- Add recovery instructions specific to custom PCB:
  - `GPIO9 low` + reset
  - keep `GPIO8` high

## V0.2 Design Improvements (No Jumper Bring-Up Goal)
- Add dedicated BOOT control for `GPIO9` (button or test pad).
- Add dedicated MCU reset button on `EN` (separate from full-rail power-cycle behavior).
- Add deterministic strap biasing so peripherals cannot dominate `GPIO8/GPIO9` at reset.
- Add stronger IMU CS default inactive behavior during reset window.
- Consider delayed IMU enable/reset so MCU strap sampling completes before IMU drives SPI-related nets.
- Optional: add auto-program/reset circuitry for one-click flashing workflow.

## Acceptance Criteria for V0.1 Prototype Success
- Board reliably enters download mode with forced strap method.
- Firmware flashes successfully.
- Board boots into expected setup AP flow after flash.
- Basic runtime behavior is stable across multiple reboots.
