#include "TimedPoseHistory.hpp"

struct TestPose {
    float x = 0;
    constexpr TestPose LerpTo(TestPose other, float t) const {
        return {x + (other.x - x) * t};
    }
};
constexpr bool Near(float a, float b) {
    return (a - b < .0001f) && (b - a < .0001f);
}
constexpr bool HistoryRegression() {
    VainSabers::TimedPoseHistory<TestPose, 100> history;
    for (int fps : {60, 72, 80, 90, 120}) {
        history.Clear();
        for (int frame = 0; frame < 30; frame++) {
            const double time = frame / static_cast<double>(fps);
            for (int refresh = 0; refresh < 7; refresh++)
                history.Record({static_cast<float>(time)}, time, frame);
        }
        const double end = 29 / static_cast<double>(fps);
        if (history.count != 30 || !Near(history.Sample(end - .016).x, static_cast<float>(end - .016)))
            return false;
    }
    history.Clear();
    for (int frame = 0; frame < 20; frame++) {
        double time = frame / 72.0;
        history.Record({static_cast<float>(time)}, time, frame);
        // Two callbacks plus four saber parts can refresh the same Unity frame.
        for (int i = 0; i < 6; i++)
            history.Record({static_cast<float>(time)}, time, frame);
    }
    if (history.count != 20)
        return false;
    const double now = 19 / 72.0;
    if (!Near(history.Sample(now - .016).x, static_cast<float>(now - .016)))
        return false;
    if (!Near(history.Sample(now).x, static_cast<float>(now)))
        return false;
    if (!Near(history.Sample(-1).x, 0))
        return false;
    history.Record({.5f}, now, 19);
    if (history.count != 20 || !Near(history.Sample(now).x, .5f))
        return false;
    // An advancing frame with the same timestamp is safe (paused frame/time).
    history.Record({.6f}, now, 20);
    if (!Near(history.Sample(now).x, .6f))
        return false;
    history.Clear();
    if (history.count)
        return false;
    for (int i = 0; i < 150; i++)
        history.Record({static_cast<float>(i)}, i * .01, i);
    if (history.count != 100 || !Near(history.Sample(-10).x, 50))
        return false;
    if (!Near(history.Sample(1.485).x, 148.5f))
        return false;
    // Time resets when gameplay restarts: no blur jump to the old scene.
    history.Record({3}, 0, 0);
    return history.count == 1 && Near(history.Sample(0).x, 3);
}
static_assert(HistoryRegression(), "Pose history must measure actual time once per Unity frame");
