#pragma once
#include <algorithm>
#include <cmath>
namespace VainSabers::PCUI {
// Matches PC ControllerYawDragHandler. A tap opens direct input on release;
// releasing after a controller drag does not open the keypad.
struct NumberGesture {
    bool pressed = false, dragged = false;
    float yaw = 0, offset = 0, startValue = 0, pressedAt = 0;
    constexpr void Begin(float angle, float value, float time) {
        pressed = true;
        dragged = false;
        yaw = angle;
        offset = 0;
        startValue = value;
        pressedAt = time;
    }
    constexpr bool Move(float angle, float &effective) {
        if (!pressed)
            return false;
        float delta = angle - yaw;
        if (delta > 180)
            delta -= 360;
        if (delta < -180)
            delta += 360;
        if (!dragged) {
            if (delta >= -2 && delta <= 2)
                return false;
            dragged = true;
            offset = delta;
        }
        effective = delta - offset;
        return true;
    }
    constexpr bool Release(float) {
        bool open = pressed && !dragged;
        pressed = false;
        return open;
    }
    constexpr void Cancel() {
        pressed = false;
    }
};
inline constexpr float NumberStep(float step) {
    return std::max(.001f, step);
}
inline constexpr int NumberDecimals(float step) {
    step = NumberStep(step);
    return step >= 1 ? 0 : step >= .1f ? 1 : step >= .01f ? 2 : 3;
}
inline float SnapNumber(float value, float min, float max, float step) {
    return std::clamp(std::round(std::clamp(value, min, max) / NumberStep(step)) * NumberStep(step), min, max);
}
} // namespace VainSabers::PCUI
