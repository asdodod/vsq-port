#include "BlurTube.hpp"
#include "UnityEngine/Vector3.hpp"
#include "UnityEngine/Vector4.hpp"
#include "UnityEngine/Vector2.hpp"
#include "UnityEngine/Bounds.hpp"
#include <cmath>
#include <algorithm>

namespace VainSabers {

UnityEngine::Mesh *BlurTube::BuildGpuTube(int ringVerts, int ringCount,
                                          const std::function<TubeRingInfo(int ringIndex)> &ringGetter) {
    if (ringVerts < 3 || ringCount < 2)
        return nullptr;

    int vertsPerRing = ringVerts + 1;
    int vertCount = vertsPerRing * ringCount;
    int stripCount = ringCount - 1;
    int indexCount = ringVerts * stripCount * 6;

    auto *mesh = UnityEngine::Mesh::New_ctor();

    ArrayW<UnityEngine::Vector3> vertices(vertCount);
    ArrayW<UnityEngine::Vector3> normals(vertCount);
    ArrayW<UnityEngine::Vector4> tangents(vertCount);
    ArrayW<UnityEngine::Color> colors(vertCount);
    ArrayW<UnityEngine::Vector2> uvs(vertCount);
    ArrayW<UnityEngine::Vector4> bladeDir(vertCount);
    ArrayW<int32_t> triangles(indexCount);

    int t = 0;
    for (int ring = 0; ring < stripCount; ring++) {
        int ringStart = ring * vertsPerRing;
        int nextRingStart = (ring + 1) * vertsPerRing;
        for (int i = 0; i < ringVerts; i++) {
            int a = ringStart + i;
            int b = ringStart + i + 1;
            int c = nextRingStart + i;
            int d = nextRingStart + i + 1;
            triangles[t++] = a;
            triangles[t++] = c;
            triangles[t++] = b;
            triangles[t++] = b;
            triangles[t++] = c;
            triangles[t++] = d;
        }
    }

    int vIdx = 0;
    constexpr float kPi = 3.14159265358979323846f;

    for (int r = 0; r < ringCount; r++) {
        TubeRingInfo info = ringGetter(r);
        float sign = (info.radius >= 0.0f) ? 1.0f : -1.0f;
        float absRadius = std::abs(info.radius);
        UnityEngine::Color vertCol{info.color.r, info.color.g, info.color.b, info.glow};

        for (int i = 0; i <= ringVerts; i++) {
            float theta = 2.0f * kPi * static_cast<float>(i) / static_cast<float>(ringVerts);
            float cosT = std::cos(theta);
            float sinT = std::sin(theta);
            float u = sign * static_cast<float>(i) / static_cast<float>(ringVerts) + 0.5f * (1.0f - sign);
            float v_ = info.ringT + info.uvOffset;

            vertices[vIdx] = UnityEngine::Vector3{info.zPos, info.offX, info.offY};
            normals[vIdx] = UnityEngine::Vector3{cosT, sinT, sign};
            tangents[vIdx] = UnityEngine::Vector4{info.radiusSlope, info.isZero ? 1.0f : 0.0f, info.ringT, absRadius};
            colors[vIdx] = vertCol;
            uvs[vIdx] = UnityEngine::Vector2{u, v_};
            bladeDir[vIdx] = UnityEngine::Vector4{info.customWeight, info.opacity, 0.0f, 0.0f};
            vIdx++;
        }
    }

    mesh->set_vertices(vertices);
    mesh->set_normals(normals);
    mesh->set_tangents(tangents);
    mesh->set_colors(colors);
    mesh->set_uv(uvs);
    mesh->SetUVs(1, bladeDir);
    mesh->set_triangles(triangles);

    UnityEngine::Bounds giantBounds{UnityEngine::Vector3::get_zero(), UnityEngine::Vector3{10.0f, 10.0f, 10.0f}};
    mesh->set_bounds(giantBounds);

    return mesh;
}

UnityEngine::Mesh *BlurTube::BuildSimpleBladeTube(int ringVerts, float length, float startRadius, float endRadius,
                                                  UnityEngine::Color startColor, UnityEngine::Color endColor,
                                                  float startGlow, float endGlow, float startCustomWeight,
                                                  float endCustomWeight, float startOpacity, float endOpacity,
                                                  float bulgeAmount, int minimumRings, bool enableEndCaps,
                                                  float endCapExtension, bool inverted) {
    int baseRings = std::max(static_cast<int>(length * 8.0f), minimumRings);
    int ringCount = baseRings + (enableEndCaps ? 2 : 0);
    int mainCount = enableEndCaps ? ringCount - 2 : ringCount;

    float startRad = inverted ? -startRadius : startRadius;
    float endRad = inverted ? -endRadius : endRadius;

    return BuildGpuTube(ringVerts, ringCount, [=](int r) -> TubeRingInfo {
        TubeRingInfo info;
        info.offX = 0.0f;
        info.offY = 0.0f;
        info.uvOffset = 0.0f;

        if (enableEndCaps) {
            if (r == 0) {
                info.zPos = 0.0f - startRadius * 0.25f * endCapExtension;
                info.radius = startRad;
                info.isZero = true;
                info.radiusSlope = 0.0f;
                info.ringT = 0.0f;
                info.color = startColor;
                info.glow = startGlow;
                info.customWeight = startCustomWeight;
                info.opacity = startOpacity;
            } else if (r == ringCount - 1) {
                info.zPos = length + endRadius * 0.25f * endCapExtension;
                info.radius = endRad;
                info.isZero = true;
                info.radiusSlope = 0.0f;
                info.ringT = 1.0f;
                info.color = endColor;
                info.glow = endGlow;
                info.customWeight = endCustomWeight;
                info.opacity = endOpacity;
            } else {
                int mi = r - 1;
                float t = mainCount > 1 ? static_cast<float>(mi) / static_cast<float>(mainCount - 1) : 0.0f;
                info.zPos = t * length;
                float lin = (1.0f - t) * startRad + t * endRad;
                float bulge = 1.0f + 4.0f * (t - t * t) * bulgeAmount;
                info.radius = lin * bulge;

                float dLin = endRad - startRad;
                float dBulge_dt = 4.0f * (1.0f - 2.0f * t) * bulgeAmount;
                float dRad_dt = dLin * bulge + lin * dBulge_dt;
                info.radiusSlope = length > 0.0001f ? (dRad_dt / length) : 0.0f;
                info.isZero = std::abs(info.radius) < 0.0002f;
                info.ringT = t;

                info.color = UnityEngine::Color::Lerp(startColor, endColor, t);
                info.glow = (1.0f - t) * startGlow + t * endGlow;
                info.customWeight = (1.0f - t) * startCustomWeight + t * endCustomWeight;
                info.opacity = (1.0f - t) * startOpacity + t * endOpacity;
            }
        } else {
            float t = ringCount > 1 ? static_cast<float>(r) / static_cast<float>(ringCount - 1) : 0.0f;
            info.zPos = t * length;
            float lin = (1.0f - t) * startRad + t * endRad;
            float bulge = 1.0f + 4.0f * (t - t * t) * bulgeAmount;
            info.radius = lin * bulge;

            float dLin = endRad - startRad;
            float dBulge_dt = 4.0f * (1.0f - 2.0f * t) * bulgeAmount;
            float dRad_dt = dLin * bulge + lin * dBulge_dt;
            info.radiusSlope = length > 0.0001f ? (dRad_dt / length) : 0.0f;
            info.isZero = std::abs(info.radius) < 0.0002f;
            info.ringT = t;

            info.color = UnityEngine::Color::Lerp(startColor, endColor, t);
            info.glow = (1.0f - t) * startGlow + t * endGlow;
            info.customWeight = (1.0f - t) * startCustomWeight + t * endCustomWeight;
            info.opacity = (1.0f - t) * startOpacity + t * endOpacity;
        }

        return info;
    });
}

UnityEngine::Mesh *BlurTube::BuildAdvancedBladeTube(int ringVerts, float length, const std::vector<RingData> &rings) {
    if (rings.size() < 2)
        return nullptr;
    int ringCount = static_cast<int>(rings.size());

    return BuildGpuTube(ringVerts, ringCount, [=, &rings](int r) -> TubeRingInfo {
        const auto &ring = rings[r];
        TubeRingInfo info;
        info.zPos = ring.position * length;
        info.offX = ring.offsetX;
        info.offY = ring.offsetY;
        info.radius = ring.inverted ? -ring.radius : ring.radius;
        info.isZero = std::abs(ring.radius) < 0.0002f;
        info.ringT = ring.position;
        info.color = ring.color;
        info.glow = ring.glow;
        info.customWeight = ring.customWeight;
        info.opacity = ring.opacity;
        info.uvOffset = ring.uvOffset;

        float radiusSlope = 0.0f;
        if (!info.isZero && ringCount > 1 && length > 0.0001f) {
            int prevIdx = (r - 1 + ringCount) % ringCount;
            int nextIdx = (r + 1) % ringCount;
            float prevRad = rings[prevIdx].inverted ? -rings[prevIdx].radius : rings[prevIdx].radius;
            float nextRad = rings[nextIdx].inverted ? -rings[nextIdx].radius : rings[nextIdx].radius;
            float curT = ring.position;
            float dtPrev = curT - rings[prevIdx].position;
            if (dtPrev <= 0.0f)
                dtPrev += 1.0f;
            float dtNext = rings[nextIdx].position - curT;
            if (dtNext <= 0.0f)
                dtNext += 1.0f;
            float dt = dtPrev + dtNext;
            if (dt > 0.0001f) {
                radiusSlope = (nextRad - prevRad) / (dt * length);
            }
        }
        info.radiusSlope = radiusSlope;

        return info;
    });
}

} // namespace VainSabers
