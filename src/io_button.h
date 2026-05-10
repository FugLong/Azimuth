#pragma once

namespace azimuth_io_button {

using TapCallback = void (*)();

void init();
/** Single tap on release; double tap = second release within ~400 ms of the first. Either cb may be null. */
void setTapCallbacks(TapCallback singleTap, TapCallback doubleTap);
/**
 * Long press fires once when the button has been held continuously for ~2 s, before
 * release. The release is then swallowed (no single-tap is dispatched for the same
 * press). `cb` may be null.
 */
void setLongPressCallback(TapCallback cb);
bool isPressed();
void tick();

}  // namespace azimuth_io_button
