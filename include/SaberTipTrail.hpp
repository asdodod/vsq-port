#pragma once

#include "main.hpp"
#include "PresetData.hpp"
#include "MovementTracker.hpp"
#include "custom-types/shared/macros.hpp"
#include "UnityEngine/MonoBehaviour.hpp"
#include "UnityEngine/LineRenderer.hpp"
#include "UnityEngine/Material.hpp"
#include "UnityEngine/Gradient.hpp"
#include "UnityEngine/Color.hpp"
#include "UnityEngine/Vector3.hpp"
#include "UnityEngine/Transform.hpp"
#include <vector>

namespace VainSabers {

struct TipTrailNativeData {
    SaberTrailData trailData{};
    UnityEngine::Color gameColor{1.0f, 1.0f, 1.0f, 1.0f};
    UnityEngine::Color tonemappedGame{1.0f, 1.0f, 1.0f, 1.0f};
    UnityEngine::Color baseColor{1.0f, 1.0f, 1.0f, 1.0f};
    float opacity = 0.0f;
    int coarseCount = 24;
    int refinedCount = 47;
    int refinedCount2 = 93;
    std::vector<Pose> poseBuffer;
    std::vector<UnityEngine::Vector3> coarsePositions;
    std::vector<UnityEngine::Vector3> refinedPositions;
    std::vector<UnityEngine::Vector3> refinedPositions2;
};

} // namespace VainSabers

DECLARE_CLASS_CODEGEN(VainSabers, SaberTipTrail, UnityEngine::MonoBehaviour) {
    DECLARE_INSTANCE_FIELD(VainSabers::MovementTracker *, _tracker);
    DECLARE_INSTANCE_FIELD(UnityW<UnityEngine::LineRenderer>, _lineRenderer);
    DECLARE_INSTANCE_FIELD(UnityW<UnityEngine::Material>, _material);
    DECLARE_INSTANCE_FIELD(ArrayW<UnityEngine::Vector3>, _linePositions);
    DECLARE_INSTANCE_FIELD(UnityEngine::Gradient *, _gradient);
    DECLARE_INSTANCE_FIELD(int64_t, _nativeDataHandle);

    DECLARE_INSTANCE_METHOD(void, Awake);
    DECLARE_INSTANCE_METHOD(void, LateUpdate);
    DECLARE_INSTANCE_METHOD(void, OnDestroy);

  public:
    void Init(VainSabers::MovementTracker * tracker, const VainSabers::SaberTrailData &data,
              UnityEngine::Transform *saberTransform);
    void SetGameColor(UnityEngine::Color color);
    void ApplyConfig(const VainSabers::SaberTrailData &data);

  private:
    VainSabers::TipTrailNativeData *GetNativeData();
    void UpdateSampleCounts(int lengthMs);
    void RefinePositions(const std::vector<UnityEngine::Vector3> &coarse, std::vector<UnityEngine::Vector3> &refined);
    void UpdateFinalColor();
    void UpdateGradient(float opacity);
    static UnityEngine::Color SquarePreserveLuminance(UnityEngine::Color c);
};
