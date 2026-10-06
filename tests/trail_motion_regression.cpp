#include "TrailMotion.hpp"
#include "TimedPoseHistory.hpp"

struct TrailTestPose {
    struct Point {
        float x, y, z;
    } position;
    constexpr TrailTestPose LerpTo(TrailTestPose other, float t) const {
        return {{position.x + (other.position.x - position.x) * t, 0, 0}};
    }
};
struct TrailTestTracker {
    VainSabers::TimedPoseHistory<TrailTestPose, 100> history;
    double now;
    constexpr TrailTestPose GetPoseAgo(float age) {
        return history.Sample(now - age);
    }
};
constexpr bool Near(float a, float b) {
    return a - b < .001f && b - a < .001f;
}
constexpr bool MotionRegression() {
    for (int fps : {60, 72, 80, 90, 120}) {
        TrailTestTracker tracker{};
        for (int i = 0; i < 30; i++) {
            tracker.now = i / static_cast<double>(fps);
            tracker.history.Record({{float(tracker.now * 4), 0, 0}}, tracker.now, i);
        }
        // 4 m/s is fast enough to activate a custom ribbon at activation=1.
        if (!Near(VainSabers::TrailMotionSpeedSquared(tracker), 16))
            return false;
        for (float duration : {.03f, .06f, .15f, .2f}) {
            float displacement = tracker.history.Sample(tracker.now).position.x -
                                 tracker.history.Sample(tracker.now - duration / 31).position.x;
            float oldSpeed = displacement / .02f;
            if (duration <= .2f && oldSpeed * .5f >= .7f)
                return false;
        }
        // Stationary tracking produces no activation; disabled activation is
        // handled separately by UpdateOpacity and does not use this speed.
        tracker.history.Clear();
        tracker.history.Record({{0, 0, 0}}, tracker.now, 0);
        if (VainSabers::TrailMotionSpeedSquared(tracker) != 0)
            return false;
    }
    return true;
}
static_assert(MotionRegression(), "Ribbon activation must measure 20 ms, not adjacent mesh samples");
