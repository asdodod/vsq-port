#pragma once

#include "UnityEngine/Vector3.hpp"
#include "UnityEngine/Vector2.hpp"
#include "UnityEngine/Color.hpp"
#include <string>
#include <vector>
#include "FloatGradient.hpp"

namespace VainSabers {

enum class GeometryType { Simple = 0, Advanced = 1, Sprite = 2, Obj = 3 };

enum class SaberSide { Both = 0, LeftOnly = 1, RightOnly = 2 };

struct RingData {
    float position = 0.0f;
    float radius = 0.015f;
    UnityEngine::Color color{1.0f, 1.0f, 1.0f, 1.0f};
    float customWeight = 1.0f;
    float glow = 1.0f;
    float opacity = 1.0f;
    bool inverted = false;
    float offsetX = 0.0f;
    float offsetY = 0.0f;
    float uvOffset = 0.0f;
};

struct PartData {
    std::string name = "Part";
    UnityEngine::Vector3 position{0.0f, 0.0f, 0.0f};
    UnityEngine::Vector3 rotation{0.0f, 0.0f, 0.0f}; // Euler angles in degrees
    float length = 1.0f;
    GeometryType geometryMode = GeometryType::Simple;
    int presetVersion = 2; // OBJ handedness changed in the PC version 2 format.
    int linkedPartIndex = -1;
    float spriteSizeX = .2f, spriteSizeY = .2f, objScale = 1;
    int spriteDivisionsX = 1, spriteDivisionsY = 1;
    bool doubleSided = false;
    std::string colorTexture, glowTexture, colorTextureBase64, glowTextureBase64, objFile, objBase64;
    int textureWrap = 0;
    UnityEngine::Vector2 colorAtlasCount{1, 1}, glowAtlasCount{1, 1};
    UnityEngine::Vector3 colorAtlasSpeedFlip{1, 0, 0}, glowAtlasSpeedFlip{1, 0, 0};
    UnityEngine::Vector3 lookDir{0, 0, 0};
    bool useLookDir = false;
    bool disableGlowPass = false;
    struct Animator {
        std::string type;
        float speed = .5f, amplitude = .5f, frequency = .5f;
        int axis = 0;
    };
    std::vector<Animator> animators;

    float hueShift = 0.0f;

    float startRadius = 0.015f;
    UnityEngine::Color startColor{1.0f, 1.0f, 1.0f, 1.0f};
    float startCustomWeight = 1.0f;
    float startGlow = 1.0f;
    float startOpacity = 1.0f;

    float endRadius = 0.015f;
    UnityEngine::Color endColor{1.0f, 1.0f, 1.0f, 1.0f};
    float endCustomWeight = 1.0f;
    float endGlow = 1.0f;
    float endOpacity = 1.0f;

    bool inverted = false;
    bool lit = false;
    float blur = 1.0f;
    float blurFade = 1.0f;
    bool enableEndCaps = true;
    bool enableRoundedNormals = true;
    float endCapExtension = 0.25f;

    float bulgeAmount = 0.0f;
    int minimumRings = 4;
    int renderQueueOffset = 0;
    float depthOffset = 0.0f;
    bool disableDepthPrepass = false;
    float rimFactor = 0;
    float rimPower = 3;
    float rimPerpendicular = 0;
    std::vector<FloatGradientKey> rimGradient, glowGradient, opacityGradient;
    float specularStrength = .41f;
    float specularPower = 48;
    float metallic = 0;
    float smoothness = 0;
    float cubemapStrength = .78f;
    float cubemapRotation = 0;
    float fresnelStrength = .6f;
    float fresnelPower = 2.89f;
    UnityEngine::Color rimColor{.47f, .51f, .57f, 1};
    float fresnelCustomBlend = 0;

    SaberSide side = SaberSide::Both;
    bool mirrorOnLeft = false;

    bool manualRingVerts = false;
    int ringVertsManual = 20;

    std::vector<RingData> rings;
};

struct SaberTrailData {
    struct ColorKey {
        float time = 0;
        UnityEngine::Color color{1, 1, 1, 1};
        int easing = 0;
    };
    std::vector<ColorKey> colorGradient;
    std::vector<FloatGradientKey> customBlendGradient;
    UnityEngine::Vector3 position{0.0f, 0.0f, 1.0f};
    UnityEngine::Color color{1.0f, 1.0f, 1.0f, 1.0f};
    float customBlend = 1.0f;
    float glow = 1.0f;
    float opacity = 0.3f;
    float width = 0.01f;
    int length = 60; // ms
    int queueOffset = 0;
    float depthOffset = 0.0f;
    float fade = 1.0f;
    float motionActivation = 1.0f;
    float motionFadePower = 0.0f;
    std::string colorTexture, glowTexture, colorTextureBase64, glowTextureBase64;
    int textureWrap = 1;
    UnityEngine::Vector2 colorAtlasCount{1, 1}, glowAtlasCount{1, 1};
    UnityEngine::Vector3 colorAtlasSpeedFlip{1, 0, 0}, glowAtlasSpeedFlip{1, 0, 0};
    bool noiseEnabled = false;
    float noiseIntensity = .02f, noiseScale = 2, noiseSpeed = 1;
};
inline UnityEngine::Color TrailColor(const SaberTrailData &trail, float t, UnityEngine::Color game) {
    auto col = trail.color;
    const auto &keys = trail.colorGradient;
    if (!keys.empty()) {
        col = keys.back().color;
        if (t <= keys.front().time)
            col = keys.front().color;
        else
            for (size_t i = 1; i < keys.size(); ++i)
                if (t < keys[i].time) {
                    float u = std::clamp((t - keys[i - 1].time) / std::max(.00001f, keys[i].time - keys[i - 1].time),
                                         0.f, 1.f);
                    switch (keys[i].easing) {
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
                    col = UnityEngine::Color::Lerp(keys[i - 1].color, keys[i].color, u);
                    break;
                }
    }
    return UnityEngine::Color::Lerp(
        col, game, std::clamp(EvaluateGradient(trail.customBlendGradient, t, trail.customBlend), 0.f, 1.f));
}

struct Preset {
    int version = 1;
    bool useCustomTrails = false;
    std::vector<PartData> parts;
    std::vector<SaberTrailData> tipTrails;
    std::vector<SaberTrailData> bladeTrails;
};

} // namespace VainSabers
