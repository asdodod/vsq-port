#pragma once

#include "main.hpp"
#include "PresetData.hpp"
#include "MovementTracker.hpp"
#include "custom-types/shared/macros.hpp"
#include "UnityEngine/MonoBehaviour.hpp"
#include "UnityEngine/MeshFilter.hpp"
#include "UnityEngine/MeshRenderer.hpp"
#include "UnityEngine/Mesh.hpp"
#include "UnityEngine/Material.hpp"
#include "UnityEngine/MaterialPropertyBlock.hpp"
#include "UnityEngine/Color.hpp"
#include "UnityEngine/Vector4.hpp"
#include "UnityEngine/Vector3.hpp"
#include "UnityEngine/Transform.hpp"
#include <array>

namespace VainSabers {

struct RibbonNativeData {
    SaberTrailData trailData{};
    UnityEngine::Color gameColor{1.0f, 1.0f, 1.0f, 1.0f};
    UnityEngine::Color tonemappedGame{1.0f, 1.0f, 1.0f, 1.0f};
    float opacity = 0.0f;
    int segmentCount = 30;
    std::array<Pose, 32> histPoseBuffer{};
    std::array<UnityEngine::Vector3, 32> positions{}, forwards{}, ups{};
};

} // namespace VainSabers

DECLARE_CLASS_CODEGEN(VainSabers, SaberRibbonTrail, UnityEngine::MonoBehaviour) {
    DECLARE_INSTANCE_FIELD(VainSabers::MovementTracker *, _tracker);
    DECLARE_INSTANCE_FIELD(UnityW<UnityEngine::MeshFilter>, _meshFilter);
    DECLARE_INSTANCE_FIELD(UnityW<UnityEngine::MeshRenderer>, _meshRenderer);
    DECLARE_INSTANCE_FIELD(UnityW<UnityEngine::Mesh>, _mesh);
    DECLARE_INSTANCE_FIELD(UnityW<UnityEngine::Material>, _material);
    DECLARE_INSTANCE_FIELD(UnityEngine::MaterialPropertyBlock *, _propBlock);
    DECLARE_INSTANCE_FIELD(ArrayW<UnityEngine::Vector3>, _vertices);
    DECLARE_INSTANCE_FIELD(ArrayW<UnityEngine::Color>, _colors);
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
    VainSabers::RibbonNativeData *GetNativeData();
    void RebuildMesh();
    void RebuildColors();
    void UpdateOpacity(float tipSpeed);
    static UnityEngine::Color SquarePreserveLuminance(UnityEngine::Color c);
};
