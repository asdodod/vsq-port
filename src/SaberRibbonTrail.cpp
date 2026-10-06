#include "SaberRibbonTrail.hpp"
#include "TrailMotion.hpp"
#include "RibbonGeometry.hpp"
#include "TrailResources.hpp"
#include "VainSabersAssets.hpp"
#include "UnityEngine/GameObject.hpp"
#include "UnityEngine/Shader.hpp"
#include "UnityEngine/Bounds.hpp"
#include <algorithm>
#include <cmath>

DEFINE_TYPE(VainSabers, SaberRibbonTrail);

namespace VainSabers {

struct RibbonShaderProps {
    int32_t _TrailHistCount = 0;
    int32_t _TrailOpacityScale = 0;
    int32_t _CustomColor = 0;
    int32_t _GlowBoost = 0;
    int32_t _DepthOffset = 0;

    static const RibbonShaderProps &Get() {
        static RibbonShaderProps s_props = [] {
            RibbonShaderProps p;
            p._TrailHistCount = UnityEngine::Shader::PropertyToID(StringW("_TrailHistCount"));
            p._TrailOpacityScale = UnityEngine::Shader::PropertyToID(StringW("_TrailOpacityScale"));
            p._CustomColor = UnityEngine::Shader::PropertyToID(StringW("_CustomColor"));
            p._GlowBoost = UnityEngine::Shader::PropertyToID(StringW("_GlowBoost"));
            p._DepthOffset = UnityEngine::Shader::PropertyToID(StringW("_DepthOffset"));
            return p;
        }();
        return s_props;
    }
};

RibbonNativeData *SaberRibbonTrail::GetNativeData() {
    if (!_nativeDataHandle) {
        _nativeDataHandle = reinterpret_cast<int64_t>(new RibbonNativeData());
    }
    return reinterpret_cast<RibbonNativeData *>(_nativeDataHandle);
}

void SaberRibbonTrail::Awake() {
    if (!_nativeDataHandle) {
        _nativeDataHandle = reinterpret_cast<int64_t>(new RibbonNativeData());
    }
    if (!_propBlock) {
        _propBlock = UnityEngine::MaterialPropertyBlock::New_ctor();
    }
}

void SaberRibbonTrail::Init(VainSabers::MovementTracker *tracker, const VainSabers::SaberTrailData &data,
                            UnityEngine::Transform *saberTransform) {
    this->_tracker = tracker;
    auto go = this->get_gameObject();

    if (!_meshFilter) {
        _meshFilter = go->GetComponent<UnityEngine::MeshFilter *>();
        if (!_meshFilter)
            _meshFilter = go->AddComponent<UnityEngine::MeshFilter *>();
    }
    if (!_meshRenderer) {
        _meshRenderer = go->GetComponent<UnityEngine::MeshRenderer *>();
        if (!_meshRenderer)
            _meshRenderer = go->AddComponent<UnityEngine::MeshRenderer *>();
    }

    auto *shader = Assets::VertexGlowShader.unsafePtr();
    if (!shader || !shader->get_isSupported())
        shader = Assets::VertexGlowShaderUntextured.unsafePtr();

    if (shader && shader->get_isSupported() && !_material) {
        _material = UnityEngine::Material::New_ctor(shader);
        _material->set_name(StringW("SaberRibbonTrail (Instance)"));
        _material->SetShaderPassEnabled(StringW("ALPHA"), false);
        _material->SetInteger(RibbonShaderProps::Get()._TrailHistCount, 0);
    }

    if (_meshRenderer && _material) {
        _meshRenderer->set_sharedMaterial(_material);
        _meshRenderer->set_sortingOrder(100);
        _meshRenderer->set_enabled(true);
    }

    ApplyConfig(data);
}

UnityEngine::Color SaberRibbonTrail::SquarePreserveLuminance(UnityEngine::Color c) {
    float lum = 0.299f * c.r + 0.587f * c.g + 0.114f * c.b;
    float r2 = c.r * c.r;
    float g2 = c.g * c.g;
    float b2 = c.b * c.b;
    float lum2 = 0.299f * r2 + 0.587f * g2 + 0.114f * b2;
    float scale = (lum2 > 0.00001f) ? (lum / lum2) : 0.0f;
    return UnityEngine::Color{std::clamp(r2 * scale, 0.0f, 1.0f), std::clamp(g2 * scale, 0.0f, 1.0f),
                              std::clamp(b2 * scale, 0.0f, 1.0f), c.a};
}

void SaberRibbonTrail::SetGameColor(UnityEngine::Color color) {
    auto *d = GetNativeData();
    d->gameColor = color;
    d->tonemappedGame =
        SquarePreserveLuminance(UnityEngine::Color{color.r * 0.8f, color.g * 0.8f, color.b * 0.8f, color.a});
    if (_material) {
        const auto &props = RibbonShaderProps::Get();
        _material->SetColor(props._CustomColor, d->tonemappedGame);
    }
    RebuildColors();
}

void SaberRibbonTrail::ApplyConfig(const VainSabers::SaberTrailData &data) {
    auto *d = GetNativeData();
    d->trailData = data;
    d->segmentCount = std::clamp(data.length / 6, 4, 512);

    if (_material) {
        const auto &props = RibbonShaderProps::Get();
        _material->set_renderQueue(3600 + data.queueOffset);
        _material->SetFloat(props._GlowBoost, data.glow);
        _material->SetFloat(props._DepthOffset, data.depthOffset);
        _material->SetColor(props._CustomColor, d->tonemappedGame);
        ApplyTrailResources(_material, data, _colorTexture, _glowTexture);
    }

    RebuildMesh();
}

void SaberRibbonTrail::RebuildMesh() {
    auto *d = GetNativeData();
    int segmentCount = d->segmentCount;
    constexpr int kVertSubdiv = 6;
    constexpr int kVertCount = kVertSubdiv + 1;
    int totalVerts = (segmentCount + 1) * kVertCount;
    int totalTris = segmentCount * kVertSubdiv * 6;

    if (_mesh) {
        UnityEngine::Object::Destroy(_mesh);
        _mesh = nullptr;
    }
    _mesh = UnityEngine::Mesh::New_ctor();
    _mesh->set_name(StringW("SaberRibbonTrail"));
    _mesh->MarkDynamic();

    ArrayW<UnityEngine::Vector3> vertices(totalVerts);
    _vertices = vertices;
    _colors = ArrayW<UnityEngine::Color>(totalVerts);
    ArrayW<UnityEngine::Vector2> uvs(totalVerts);
    ArrayW<int32_t> triangles(totalTris);

    for (int i = 0; i < totalVerts; i++) {
        vertices[i] = UnityEngine::Vector3{0.0f, 0.0f, 0.0f};
    }

    // Build triangles
    int triIdx = 0;
    for (int i = 0; i < segmentCount; i++) {
        for (int v = 0; v < kVertSubdiv; v++) {
            int vert00 = i * kVertCount + v;
            int vert01 = vert00 + 1;
            int vert10 = (i + 1) * kVertCount + v;
            int vert11 = vert10 + 1;

            triangles[triIdx++] = vert00;
            triangles[triIdx++] = vert10;
            triangles[triIdx++] = vert01;

            triangles[triIdx++] = vert01;
            triangles[triIdx++] = vert10;
            triangles[triIdx++] = vert11;
        }
    }

    // Colors and UVs
    int vIdx = 0;
    for (int i = 0; i <= segmentCount; i++) {
        float t = static_cast<float>(i) / static_cast<float>(segmentCount);
        auto blendedColor = TrailColor(d->trailData, t, d->tonemappedGame);
        float a = std::lerp(0.9f, 0.0f, t * d->trailData.fade) * std::pow(t, 0.02f);
        float segmentOpacity = a * a;
        float staticOpacity = segmentOpacity * d->trailData.opacity;

        UnityEngine::Color tipBase{blendedColor.r, blendedColor.g, blendedColor.b, 0.0f};
        UnityEngine::Color tipFull{blendedColor.r, blendedColor.g, blendedColor.b, staticOpacity};

        for (int v = 0; v < kVertCount; v++) {
            float vFrac = static_cast<float>(v) / static_cast<float>(kVertSubdiv);
            _colors[vIdx] =
                UnityEngine::Color{std::lerp(tipBase.r, tipFull.r, vFrac), std::lerp(tipBase.g, tipFull.g, vFrac),
                                   std::lerp(tipBase.b, tipFull.b, vFrac), std::lerp(tipBase.a, tipFull.a, vFrac)};
            UnityEngine::Vector2 uv;
            uv.x = t;
            uv.y = vFrac;
            uvs[vIdx] = uv;
            vIdx++;
        }
    }

    _mesh->set_vertices(vertices);
    _mesh->set_colors(_colors);
    _mesh->set_uv(uvs);
    _mesh->set_triangles(triangles);

    UnityEngine::Bounds giantBounds{UnityEngine::Vector3::get_zero(), UnityEngine::Vector3{100.0f, 100.0f, 100.0f}};
    _mesh->set_bounds(giantBounds);

    if (_meshFilter) {
        _meshFilter->set_mesh(_mesh);
    }
}

void SaberRibbonTrail::RebuildColors() {
    if (!_mesh || !_colors)
        return;
    auto *d = GetNativeData();
    for (int i = 0; i <= d->segmentCount; ++i) {
        float t = float(i) / d->segmentCount;
        auto color = TrailColor(d->trailData, t, d->tonemappedGame);
        float a = std::lerp(.9f, 0.f, t * d->trailData.fade) * std::pow(t, .02f);
        for (int v = 0; v < 7; ++v)
            _colors[i * 7 + v] = {color.r, color.g, color.b, a * a * d->trailData.opacity * (v / 6.f)};
    }
    _mesh->set_colors(_colors);
}

void SaberRibbonTrail::UpdateOpacity(float tipSpeed) {
    auto *d = GetNativeData();
    float activation = std::clamp(d->trailData.motionActivation, 0.0f, 1.0f);
    if (activation <= 0.001f) {
        d->opacity = 1.0f;
        return;
    }
    tipSpeed *= 0.5f;
    float expFactor = std::exp((1.0f - activation) * 2.2f);
    float threshold = 0.7f * activation;
    float gated = std::clamp((tipSpeed - threshold) * expFactor, 0.0f, 1.0f);
    d->opacity = std::max(gated, std::max(0.f, d->opacity - UnityEngine::Time::get_deltaTime() * 4.f * expFactor));
}

void SaberRibbonTrail::LateUpdate() {
    if (!_tracker || !_tracker->get_transform() || !_meshRenderer || !_material || !_mesh || !_vertices)
        return;

    auto *d = GetNativeData();
    if (d->trailData.length <= 0) {
        _meshRenderer->set_enabled(false);
        return;
    }

    float duration = d->trailData.length * 0.001f;
    _tracker->SampleNonAlloc(32, duration, d->histPoseBuffer.data());

    // Motion activation uses a fixed 20 ms window, just like PC and tip trails.
    // Adjacent ribbon samples are duration/31 apart, not 20 ms. Dividing their
    // displacement by 20 ms underestimated speed (10x at 60 ms), hiding trails.
    float tipSpeed = std::sqrt(TrailMotionSpeedSquared(*_tracker));
    UpdateOpacity(tipSpeed);

    for (int i = 0; i < 32; i++) {
        const auto &p = d->histPoseBuffer[i];
        d->positions[i] = p.position;
        d->forwards[i] = p.GetForward();
        d->ups[i] = p.GetUp();
    }

    float motionFade = 1.f;
    if (d->trailData.motionFadePower > .001f) {
        float distance = 0.f;
        UnityEngine::Vector3 lastTip{}, lastBase{};
        for (int i = 0; i < 32; ++i) {
            auto tip = RibbonTip(d->positions[i], d->forwards[i], d->ups[i], d->trailData.position);
            auto base = RibbonLerp(d->positions[i], tip, std::clamp(d->trailData.width, 0.f, 1.f));
            if (i > 0) {
                auto a = tip - lastTip, b = base - lastBase;
                distance += std::sqrt(a.x * a.x + a.y * a.y + a.z * a.z) + std::sqrt(b.x * b.x + b.y * b.y + b.z * b.z);
            }
            lastTip = tip;
            lastBase = base;
        }
        motionFade = std::exp(-distance * .5f * d->trailData.motionFadePower);
    }
    float opacity = std::clamp(d->opacity * motionFade, 0.f, 1.f);

    // Generate the same 32-pose ribbon in world space, then transform it into
    // the moving mesh's local space once per frame. Reuse the managed buffer.
    auto worldToLocal = get_transform()->get_worldToLocalMatrix();
    float baseFraction = std::clamp(d->trailData.width, 0.f, 1.f);
    for (int i = 0; i <= d->segmentCount; ++i) {
        float hist = float(i) / d->segmentCount * 31;
        int index = std::min(int(hist), 30);
        float frac = hist - index;
        auto pos = RibbonLerp(d->positions[index], d->positions[index + 1], frac);
        auto forward = RibbonLerp(d->forwards[index], d->forwards[index + 1], frac);
        auto up = RibbonLerp(d->ups[index], d->ups[index + 1], frac);
        auto tip = RibbonTip(pos, forward, up, d->trailData.position);
        auto base = RibbonLerp(pos, tip, baseFraction);
        for (int v = 0; v < 7; ++v)
            _vertices[i * 7 + v] = RibbonLocalPoint(RibbonLerp(base, tip, v / 6.f), worldToLocal);
    }
    _mesh->set_vertices(_vertices);

    const auto &props = RibbonShaderProps::Get();
    if (!_propBlock) {
        _propBlock = UnityEngine::MaterialPropertyBlock::New_ctor();
    }

    _propBlock->Clear();
    _propBlock->SetFloat(props._TrailOpacityScale, opacity);

    // The simple, supported fallback shader has no opacity uniform.
    // Bake activation into colors only for that fallback, without rebuilding.
    if (_material->get_shader().unsafePtr() == Assets::VertexGlowShaderUntextured.unsafePtr()) {
        RebuildColors();
        for (int i = 0; i < _colors.size(); ++i)
            _colors[i].a *= opacity;
        _mesh->set_colors(_colors);
    }

    _meshRenderer->SetPropertyBlock(_propBlock);
    _meshRenderer->set_enabled(true);
}

void SaberRibbonTrail::OnDestroy() {
    if (_colorTexture)
        UnityEngine::Object::Destroy(_colorTexture);
    if (_glowTexture)
        UnityEngine::Object::Destroy(_glowTexture);
    if (_mesh) {
        UnityEngine::Object::Destroy(_mesh);
        _mesh = nullptr;
    }
    if (_material) {
        UnityEngine::Object::Destroy(_material);
        _material = nullptr;
    }
    if (_nativeDataHandle) {
        delete reinterpret_cast<RibbonNativeData *>(_nativeDataHandle);
        _nativeDataHandle = 0;
    }
    _propBlock = nullptr;
}

} // namespace VainSabers
