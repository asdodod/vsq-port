#pragma once

#include "main.hpp"
#include "PresetData.hpp"
#include "custom-types/shared/macros.hpp"
#include "UnityEngine/MonoBehaviour.hpp"
#include "UnityEngine/MeshFilter.hpp"
#include "UnityEngine/MeshRenderer.hpp"
#include "UnityEngine/Mesh.hpp"
#include "UnityEngine/Material.hpp"
#include "UnityEngine/MaterialPropertyBlock.hpp"
#include "UnityEngine/Color.hpp"
#include "UnityEngine/Vector4.hpp"
#include "UnityEngine/Texture2D.hpp"
#include "MovementTracker.hpp"
#include "BlurTube.hpp"
#include "PoseHelpers.hpp"
#include <array>
#include <vector>

namespace VainSabers {

struct BlurPartNativeData {
    float length = 1.0f;
    float startRadius = 0.015f;
    float endRadius = 0.015f;
    UnityEngine::Color startColor{1.0f, 1.0f, 1.0f, 1.0f};
    UnityEngine::Color endColor{1.0f, 1.0f, 1.0f, 1.0f};
    float startCustomColorWeight = 1.0f;
    float endCustomColorWeight = 1.0f;
    float startGlow = 1.0f;
    float endGlow = 1.0f;
    float startOpacity = 1.0f;
    float endOpacity = 1.0f;
    float blurFactor = 1.0f;
    float blurFadeFactor = 1.0f;
    float depthOffset = 0.001f;
    bool enableEndCaps = true;
    bool enableRoundedNormals = true;
    float endCapExtension = 0.25f;
    float bulgeAmount = 0.0f;
    int minimumRings = 4;
    int renderQueueOffset = 0;
    float hueShift = 0.0f;
    bool inverted = false;
    bool lit = false;
    bool isLeft = false;
    float timeStarted = 0;
    PartData materialData;
    int ringVerts = 20;

    VainSabers::GeometryType geometryMode = VainSabers::GeometryType::Simple;
    std::vector<VainSabers::RingData> rings;

    std::array<VainSabers::Pose, 8> coarsePoses{};
    std::array<VainSabers::Pose, 16> refinedPoses{};
};

} // namespace VainSabers

DECLARE_CLASS_CODEGEN(VainSabers, BlurSaberPart, UnityEngine::MonoBehaviour) {
    DECLARE_INSTANCE_FIELD(VainSabers::MovementTracker *, _tracker);
    DECLARE_INSTANCE_FIELD(UnityW<UnityEngine::MeshFilter>, _meshFilter);
    DECLARE_INSTANCE_FIELD(UnityW<UnityEngine::MeshRenderer>, _meshRenderer);
    DECLARE_INSTANCE_FIELD(UnityW<UnityEngine::Mesh>, _mesh);
    DECLARE_INSTANCE_FIELD(UnityW<UnityEngine::Material>, _material);
    DECLARE_INSTANCE_FIELD(UnityW<UnityEngine::Texture2D>, _rimGradient);
    DECLARE_INSTANCE_FIELD(UnityW<UnityEngine::Texture2D>, _glowGradient);
    DECLARE_INSTANCE_FIELD(UnityW<UnityEngine::Texture2D>, _opacityGradient);
    DECLARE_INSTANCE_FIELD(UnityW<UnityEngine::Texture2D>, _colorTexture);
    DECLARE_INSTANCE_FIELD(UnityW<UnityEngine::Texture2D>, _glowTexture);
    DECLARE_INSTANCE_FIELD(UnityEngine::Color, _customColor);
    DECLARE_INSTANCE_FIELD(UnityEngine::MaterialPropertyBlock *, m_propertyBlock);
    DECLARE_INSTANCE_FIELD(ArrayW<UnityEngine::Vector4>, m_vertexHistPos);
    DECLARE_INSTANCE_FIELD(ArrayW<UnityEngine::Vector4>, m_vertexHistFwd);
    DECLARE_INSTANCE_FIELD(ArrayW<UnityEngine::Vector4>, m_vertexHistUp);
    DECLARE_INSTANCE_FIELD(int64_t, _nativeDataHandle);

    DECLARE_INSTANCE_METHOD(void, Awake);
    DECLARE_INSTANCE_METHOD(void, Start);
    DECLARE_INSTANCE_METHOD(void, LateUpdate);
    DECLARE_INSTANCE_METHOD(void, OnDestroy);
    DECLARE_INSTANCE_METHOD(void, Init, VainSabers::MovementTracker *tracker);
    DECLARE_INSTANCE_METHOD(void, SetColor, UnityEngine::Color color);

  public:
    VainSabers::BlurPartNativeData *GetNativeData();
    void ApplyPartData(const VainSabers::PartData &data, bool isLeft);
    void RebuildMesh();
    void SampleGpuHistory();
    void ApplyMaterialProps();
    void EnsureRuntimeMaterial();
    UnityEngine::Material *GetActiveBaseMaterial();
};
