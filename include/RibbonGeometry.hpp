#pragma once
#include <algorithm>
#include <cmath>

namespace VainSabers {
// The same interpolated, orthonormal basis as the PC history shader. Build real
// vertices on Quest so shader history uniforms cannot collapse the whole mesh.
template <typename V> V RibbonNormalize(V v, V fallback) {
    float n = v.x * v.x + v.y * v.y + v.z * v.z;
    if (n < .00000001f)
        return fallback;
    float scale = 1.f / std::sqrt(n);
    return {v.x * scale, v.y * scale, v.z * scale};
}
template <typename V> V RibbonLerp(V a, V b, float t) {
    return {std::lerp(a.x, b.x, t), std::lerp(a.y, b.y, t), std::lerp(a.z, b.z, t)};
}
template <typename V> V RibbonCross(V a, V b) {
    return {a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x};
}
template <typename V> V RibbonTip(V position, V forward, V up, V offset) {
    forward = RibbonNormalize(forward, V{0, 0, 1});
    up = RibbonNormalize(up, V{0, 1, 0});
    V right = RibbonNormalize(RibbonCross(up, forward), V{1, 0, 0});
    up = RibbonNormalize(RibbonCross(forward, right), V{0, 1, 0});
    return {position.x + right.x * offset.x + up.x * offset.y + forward.x * offset.z,
            position.y + right.y * offset.x + up.y * offset.y + forward.y * offset.z,
            position.z + right.z * offset.x + up.z * offset.y + forward.z * offset.z};
}
template <typename V, typename M> V RibbonLocalPoint(V point, const M &worldToLocal) {
    return {worldToLocal.m00 * point.x + worldToLocal.m01 * point.y + worldToLocal.m02 * point.z + worldToLocal.m03,
            worldToLocal.m10 * point.x + worldToLocal.m11 * point.y + worldToLocal.m12 * point.z + worldToLocal.m13,
            worldToLocal.m20 * point.x + worldToLocal.m21 * point.y + worldToLocal.m22 * point.z + worldToLocal.m23};
}
} // namespace VainSabers
