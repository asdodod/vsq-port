#include "PcNumberGesture.hpp"
using VainSabers::PCUI::NumberGesture;
constexpr bool NumberGestureRegression() {
    NumberGesture g;
    float delta = 0;
    g.Begin(10, 16, 0);
    if (g.Move(11, delta) || !g.Release(.1f))
        return false;
    g.Begin(10, 16, 0);
    if (g.Move(12, delta))
        return false;
    if (!g.Move(13, delta) || delta != 0 || !g.Move(23, delta) || delta != 10)
        return false;
    if (g.Release(.1f) || g.Move(30, delta))
        return false;
    g.Begin(0, 1, 0);
    if (!g.Move(-3, delta) || !g.Move(-13, delta) || delta != -10 || g.Release(1))
        return false;
    // Once dragged, returning to the initial direction must not open input.
    g.Begin(0, 1, 0);
    g.Move(3, delta);
    g.Move(0, delta);
    if (g.Release(1))
        return false;
    // Crossing the 180-degree yaw boundary remains a small positive motion.
    g.Begin(179, 1, 0);
    if (!g.Move(-178, delta) || delta != 0 || !g.Move(-168, delta) || delta != 10 || g.Release(1))
        return false;
    g.Begin(0, 1, 0);
    g.Cancel();
    if (g.Release(1))
        return false;
    return true;
}
static_assert(NumberGestureRegression(), "Number input tap / drag / dead zone / cancel regression");
static_assert(VainSabers::PCUI::NumberDecimals(.00025f) == 3);
static_assert(VainSabers::PCUI::NumberDecimals(.005f) == 3);
static_assert(VainSabers::PCUI::NumberDecimals(.01f) == 2);
static_assert(VainSabers::PCUI::NumberDecimals(1) == 0);
