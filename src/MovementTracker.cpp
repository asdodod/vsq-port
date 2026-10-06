#include "MovementTracker.hpp"
#include "PluginConfig.hpp"
#include <cmath>

DEFINE_TYPE(VainSabers, MovementTracker);

namespace VainSabers {

MovementTrackerState *MovementTracker::GetState() {
    if (!_nativeStateHandle) {
        _nativeStateHandle = reinterpret_cast<int64_t>(new MovementTrackerState());
    }
    return reinterpret_cast<MovementTrackerState *>(_nativeStateHandle);
}

void MovementTracker::Awake() {
    if (!_nativeStateHandle) {
        _nativeStateHandle = reinterpret_cast<int64_t>(new MovementTrackerState());
    }
}

void MovementTracker::OnDestroy() {
    if (_nativeStateHandle) {
        delete reinterpret_cast<MovementTrackerState *>(_nativeStateHandle);
        _nativeStateHandle = 0;
    }
}

void MovementTracker::Init(UnityEngine::Transform *target) {
    this->_target = target;
    if (!_nativeStateHandle) {
        _nativeStateHandle = reinterpret_cast<int64_t>(new MovementTrackerState());
    }
    ClearHistory();
}

void MovementTracker::ClearHistory() {
    auto *s = GetState();
    if (s) {
        s->Clear();
    }
}

Pose MovementTracker::GetCurrentPose() const {
    if (_target) {
        return GetTransformPose(_target.unsafePtr());
    }
    return GetTransformPose(const_cast<MovementTracker *>(this)->get_transform());
}

void MovementTracker::ManualUpdate() {
    auto *s = GetState();
    Pose currentPose = GetCurrentPose();
    const auto &cfg = GetPluginConfig();
    const int frame = UnityEngine::Time::get_frameCount();
    // Always interpolate against the preceding frame, so same-frame refreshes
    // do not repeatedly apply smoothing to themselves.
    const size_t previous = (s->lastFrame == frame) ? 1 : 0;
    if (s->count > previous) {
        const auto &old = s->Get(previous);
        const float dt = std::max(0.0f, UnityEngine::Time::get_unscaledTime() - static_cast<float>(old.time));
        auto factor = [&](float strength) { return 1 - std::exp(-dt / std::max(.0001f, .06f * strength)); };
        if (cfg.positionSmoothingEnabled)
            currentPose.position = UnityEngine::Vector3::Lerp(old.pose.position, currentPose.position,
                                                              factor(cfg.positionSmoothingStrength));
        if (cfg.rotationSmoothingEnabled)
            currentPose.rotation = UnityEngine::Quaternion::Slerp(old.pose.rotation, currentPose.rotation,
                                                                  factor(cfg.rotationSmoothingStrength));
    }
    s->Record(currentPose, UnityEngine::Time::get_unscaledTime(), UnityEngine::Time::get_frameCount());
}

void MovementTracker::Update() {
    ManualUpdate();
}

void MovementTracker::SampleNonAlloc(int samples, float duration, Pose *result) {
    if (samples <= 0 || !result)
        return;

    ManualUpdate();
    auto *s = GetState();
    Pose currentPose = s->count ? s->Get(0).pose : GetCurrentPose();

    if (samples == 1 || duration <= 0.0001f || !s || s->count == 0) {
        for (int i = 0; i < samples; i++) {
            result[i] = currentPose;
        }
        return;
    }

    const double now = s->Get(0).time;
    for (int i = 0; i < samples; ++i)
        result[i] = s->Sample(now - duration * i / static_cast<double>(samples - 1));
}

Pose MovementTracker::GetPoseAgo(float timeAgo) {
    Pose res[2];
    SampleNonAlloc(2, std::max(timeAgo, 0.0f), res);
    return res[1];
}

} // namespace VainSabers
