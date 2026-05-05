#pragma once

namespace azimuth_io_button {

using TapCallback = void (*)();

void init();
/** Single tap on release; double tap = second release within ~400 ms of the first. Either cb may be null. */
void setTapCallbacks(TapCallback singleTap, TapCallback doubleTap);
bool isPressed();
void tick();

}  // namespace azimuth_io_button
