#pragma once
#include <array>
#include <algorithm>
#include <cstddef>

namespace VainSabers {
// Absolute timestamps avoid counting Unity deltaTime more than once when several
// renderers ask for the current pose in the same frame.
template <class PoseT, size_t Capacity> struct TimedPoseHistory {
    struct Entry {
        PoseT pose{};
        double time = 0;
    };
    std::array<Entry, Capacity> buffer{};
    size_t head = 0;
    size_t count = 0;
    int lastFrame = -1;

    constexpr void Clear() {
        head = count = 0;
        lastFrame = -1;
    }
    constexpr const Entry &Get(size_t index) const {
        return buffer[(head + Capacity - 1 - index) % Capacity];
    }
    constexpr void Record(const PoseT &pose, double time, int frame) {
        if (count && time < Get(0).time)
            Clear();
        if (count && frame == lastFrame) {
            // Refresh the controller after Update without inventing elapsed time.
            buffer[(head + Capacity - 1) % Capacity].pose = pose;
            return;
        }
        buffer[head] = {pose, time};
        head = (head + 1) % Capacity;
        count = std::min(count + 1, Capacity);
        lastFrame = frame;
    }
    constexpr PoseT Sample(double time) const {
        if (!count)
            return {};
        if (time >= Get(0).time)
            return Get(0).pose;
        for (size_t i = 0; i + 1 < count; ++i) {
            const auto &newer = Get(i);
            const auto &older = Get(i + 1);
            if (time >= older.time) {
                const double span = newer.time - older.time;
                const float t = span > 0 ? static_cast<float>((newer.time - time) / span) : 0;
                return newer.pose.LerpTo(older.pose, std::clamp(t, 0.0f, 1.0f));
            }
        }
        return Get(count - 1).pose;
    }
};
} // namespace VainSabers
