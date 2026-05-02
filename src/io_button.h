#pragma once

namespace azimuth_io_button {

using ButtonPressCallback = void (*)();

void init();
void setPressCallback(ButtonPressCallback cb);
bool isPressed();
void tick();

}  // namespace azimuth_io_button
