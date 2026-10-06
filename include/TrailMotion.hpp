#pragma once

namespace VainSabers {
inline constexpr float TrailMotionWindow = 0.02f;

// Activation must measure the same interval for ribbon and tip trails,
// independently of the number of mesh samples and the configured trail length.
template <class Tracker> constexpr float TrailMotionSpeedSquared(Tracker &tracker) {
    const auto now = tracker.GetPoseAgo(0.0f).position;
    const auto prev = tracker.GetPoseAgo(TrailMotionWindow).position;
    const float dx = (now.x - prev.x) / TrailMotionWindow;
    const float dy = (now.y - prev.y) / TrailMotionWindow;
    const float dz = (now.z - prev.z) / TrailMotionWindow;
    return dx * dx + dy * dy + dz * dz;
}
} // namespace VainSabers
