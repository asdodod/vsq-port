#pragma once

#include "main.hpp"
#include "PresetData.hpp"
#include "UnityEngine/Mesh.hpp"
#include "UnityEngine/Color.hpp"
#include <functional>
#include <vector>

namespace VainSabers {

class BlurTube {
  public:
    struct TubeRingInfo {
        float zPos = 0.0f;
        float offX = 0.0f;
        float offY = 0.0f;
        float radius = 0.015f;
        float radiusSlope = 0.0f;
        bool isZero = false;
        float ringT = 0.0f;
        UnityEngine::Color color{1.0f, 1.0f, 1.0f, 1.0f};
        float glow = 1.0f;
        float customWeight = 1.0f;
        float opacity = 1.0f;
        float uvOffset = 0.0f;
    };

    // Builds a GPU tube mesh matching BlurPart.cginc vertex expectations:
    // - vertex.xyz: (zPos, offX, offY)
    // - normal.xyz: (cosTheta, sinTheta, sign)
    // - tangent.xyzw: (radiusSlope, isZeroFlag, ringT, absRadius)
    // - color.rgba: (col.r, col.g, col.b, glow)
    // - uv0.xy: (u, v)
    // - uv1.xyzw: (customWeight, opacity, 0, 0)
    static UnityEngine::Mesh *BuildGpuTube(int ringVerts, int ringCount,
                                           const std::function<TubeRingInfo(int ringIndex)> &ringGetter);

    // Builds standard parametric simple tube matching PC VainSabers 0.0.5
    static UnityEngine::Mesh *BuildSimpleBladeTube(int ringVerts, float length, float startRadius, float endRadius,
                                                   UnityEngine::Color startColor, UnityEngine::Color endColor,
                                                   float startGlow, float endGlow, float startCustomWeight,
                                                   float endCustomWeight, float startOpacity, float endOpacity,
                                                   float bulgeAmount, int minimumRings, bool enableEndCaps,
                                                   float endCapExtension, bool inverted);

    // Builds advanced tube with custom rings matching PC VainSabers 0.0.5
    static UnityEngine::Mesh *BuildAdvancedBladeTube(int ringVerts, float length, const std::vector<RingData> &rings);
};

} // namespace VainSabers
