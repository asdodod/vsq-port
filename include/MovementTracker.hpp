#pragma once

#include "main.hpp"
#include "custom-types/shared/macros.hpp"
#include "UnityEngine/MonoBehaviour.hpp"
#include "UnityEngine/Transform.hpp"
#include "UnityEngine/Vector3.hpp"
#include "UnityEngine/Quaternion.hpp"
#include "UnityEngine/Time.hpp"
#include "PoseHelpers.hpp"
#include "TimedPoseHistory.hpp"
#include <array>

namespace VainSabers {

static constexpr int kHistoryCapacity = 100;
using MovementTrackerState = TimedPoseHistory<Pose, kHistoryCapacity>;

} // namespace VainSabers

DECLARE_CLASS_CODEGEN(VainSabers, MovementTracker, UnityEngine::MonoBehaviour) {
    DECLARE_INSTANCE_FIELD(UnityW<UnityEngine::Transform>, _target);
    DECLARE_INSTANCE_FIELD(int64_t, _nativeStateHandle);

    DECLARE_INSTANCE_METHOD(void, Awake);
    DECLARE_INSTANCE_METHOD(void, Update);
    DECLARE_INSTANCE_METHOD(void, OnDestroy);
    DECLARE_INSTANCE_METHOD(void, Init, UnityEngine::Transform *target);
    DECLARE_INSTANCE_METHOD(void, ClearHistory);

  public:
    VainSabers::MovementTrackerState *GetState();
    void ManualUpdate();
    void SampleNonAlloc(int samples, float duration, VainSabers::Pose *result);
    VainSabers::Pose GetCurrentPose() const;
    VainSabers::Pose GetPoseAgo(float timeAgo);
};
