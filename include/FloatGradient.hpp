#pragma once
#include <vector>
#include <algorithm>
#include <cmath>

namespace VainSabers {
struct FloatGradientKey {
    float time = 0;
    float value = 0;
    int easing = 0;
};
inline float EvaluateGradient(const std::vector<FloatGradientKey> &keys, float t, float fallback) {
    if (keys.empty())
        return fallback;
    if (t <= keys.front().time)
        return keys.front().value;
    for (size_t i = 1; i < keys.size(); ++i) {
        const auto &prev = keys[i - 1];
        const auto &next = keys[i];
        if (t < next.time) {
            float u = std::clamp((t - prev.time) / (next.time - prev.time), 0.0f, 1.0f);
            switch (next.easing) {
            case 1:
                u *= u;
                break;
            case 2:
                u *= 2 - u;
                break;
            case 3:
                u = u < .5f ? 2 * u * u : -1 + (4 - 2 * u) * u;
                break;
            }
            return prev.value + (next.value - prev.value) * u;
        }
    }
    return keys.back().value;
}
} // namespace VainSabers
