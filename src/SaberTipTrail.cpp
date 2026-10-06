#include "SaberTipTrail.hpp"
#include "TrailMotion.hpp"
#include "VainSabersAssets.hpp"
#include "UnityEngine/GameObject.hpp"
#include "UnityEngine/Shader.hpp"
#include "UnityEngine/AnimationCurve.hpp"
#include "UnityEngine/GradientColorKey.hpp"
#include "UnityEngine/GradientAlphaKey.hpp"
#include "UnityEngine/Time.hpp"
#include <cmath>
#include <algorithm>

DEFINE_TYPE(VainSabers, SaberTipTrail);

namespace VainSabers {

TipTrailNativeData *SaberTipTrail::GetNativeData() {
    if (!_nativeDataHandle) {
        _nativeDataHandle = reinterpret_cast<int64_t>(new TipTrailNativeData());
    }
    return reinterpret_cast<TipTrailNativeData *>(_nativeDataHandle);
}

void SaberTipTrail::Awake() {
    GetNativeData();
}

void SaberTipTrail::OnDestroy() {
    if (_nativeDataHandle) {
        delete reinterpret_cast<TipTrailNativeData *>(_nativeDataHandle);
        _nativeDataHandle = 0;
    }
}

UnityEngine::Color SaberTipTrail::SquarePreserveLuminance(UnityEngine::Color c) {
    float lum = 0.299f * c.r + 0.587f * c.g + 0.114f * c.b;
    float r2 = c.r * c.r;
    float g2 = c.g * c.g;
    float b2 = c.b * c.b;
    float lum2 = 0.299f * r2 + 0.587f * g2 + 0.114f * b2;
    float scale = (lum2 > 0.00001f) ? (lum / lum2) : 0.0f;

    return UnityEngine::Color{std::clamp(r2 * scale, 0.0f, 1.0f), std::clamp(g2 * scale, 0.0f, 1.0f),
                              std::clamp(b2 * scale, 0.0f, 1.0f), c.a};
}

void SaberTipTrail::UpdateSampleCounts(int lengthMs) {
    auto *d = GetNativeData();
    int coarse = std::clamp(lengthMs / 4, 4, 256);
    d->coarseCount = coarse;
    d->refinedCount = coarse * 2 - 1;
    d->refinedCount2 = d->refinedCount * 2 - 1;

    d->poseBuffer.resize(d->coarseCount);
    d->coarsePositions.resize(d->coarseCount);
    d->refinedPositions.resize(d->refinedCount);
    d->refinedPositions2.resize(d->refinedCount2);

    _linePositions = ArrayW<UnityEngine::Vector3>(d->refinedCount2);
    if (_lineRenderer) {
        _lineRenderer->set_positionCount(d->refinedCount2);
    }
}

void SaberTipTrail::Init(MovementTracker *tracker, const SaberTrailData &data, UnityEngine::Transform *saberTransform) {
    _tracker = tracker;

    auto go = this->get_gameObject();
    if (!_lineRenderer) {
        _lineRenderer = go->GetComponent<UnityEngine::LineRenderer *>();
        if (!_lineRenderer) {
            _lineRenderer = go->AddComponent<UnityEngine::LineRenderer *>();
        }
    }

    auto *shader = Assets::VertexGlowShaderUntextured.unsafePtr();
    if (!shader)
        shader = Assets::VertexGlowShader.unsafePtr();
    if (!shader)
        shader = Assets::BlurPartShader.unsafePtr();

    if (shader && !_material) {
        _material = UnityEngine::Material::New_ctor(shader);
        _material->set_name(StringW("SaberTipTrail (Instance)"));
    }

    if (_lineRenderer && _material) {
        _lineRenderer->set_material(_material);
        _lineRenderer->set_useWorldSpace(true);

        auto curve = UnityEngine::AnimationCurve::New_ctor();
        curve->AddKey(0.0f, 0.0f);
        curve->AddKey(0.3f, 1.0f);
        curve->AddKey(1.0f, 0.0f);
        _lineRenderer->set_widthCurve(curve);
    }

    if (!_gradient) {
        _gradient = UnityEngine::Gradient::New_ctor();
    }

    ApplyConfig(data);
}

void SaberTipTrail::ApplyConfig(const SaberTrailData &data) {
    auto *d = GetNativeData();
    d->trailData = data;
    d->baseColor = data.color;

    UpdateSampleCounts(data.length);

    if (_lineRenderer) {
        _lineRenderer->set_positionCount(d->refinedCount2);
        _lineRenderer->set_widthMultiplier(data.width);
        _lineRenderer->set_sortingOrder(100);
        _lineRenderer->set_useWorldSpace(true);
    }

    if (_material) {
        _material->set_renderQueue(3600 + data.queueOffset);
        _material->SetFloat(UnityEngine::Shader::PropertyToID(StringW("_GlowBoost")), data.glow);
        _material->SetFloat(UnityEngine::Shader::PropertyToID(StringW("_DepthOffset")), data.depthOffset);
    }

    UpdateFinalColor();
}

void SaberTipTrail::SetGameColor(UnityEngine::Color color) {
    auto *d = GetNativeData();
    d->gameColor = color;
    UpdateFinalColor();
}

void SaberTipTrail::UpdateFinalColor() {
    auto *d = GetNativeData();
    d->tonemappedGame = SquarePreserveLuminance(
        UnityEngine::Color{d->gameColor.r * 0.8f, d->gameColor.g * 0.8f, d->gameColor.b * 0.8f, d->gameColor.a});
    d->tonemappedGame.a = d->gameColor.a;

    if (_material) {
        _material->SetColor(UnityEngine::Shader::PropertyToID(StringW("_CustomColor")), d->tonemappedGame);
    }
}

void SaberTipTrail::RefinePositions(const std::vector<UnityEngine::Vector3> &coarse,
                                    std::vector<UnityEngine::Vector3> &refined) {
    int coarseLen = static_cast<int>(coarse.size());
    int newLength = coarseLen * 2 - 1;
    if (static_cast<int>(refined.size()) != newLength) {
        refined.resize(newLength);
    }

    for (int i = 0; i < coarseLen - 1; i++) {
        refined[2 * i] = coarse[i];
        refined[2 * i + 1] =
            UnityEngine::Vector3{(coarse[i].x + coarse[i + 1].x) * 0.5f, (coarse[i].y + coarse[i + 1].y) * 0.5f,
                                 (coarse[i].z + coarse[i + 1].z) * 0.5f};
    }
    refined[newLength - 1] = coarse[coarseLen - 1];

    for (int i = 1; i < coarseLen - 1; i++) {
        int index = 2 * i;
        UnityEngine::Vector3 midpointAverage = UnityEngine::Vector3{
            (refined[index - 1].x + refined[index + 1].x) * 0.5f, (refined[index - 1].y + refined[index + 1].y) * 0.5f,
            (refined[index - 1].z + refined[index + 1].z) * 0.5f};
        refined[index] = UnityEngine::Vector3{(refined[index].x + midpointAverage.x) * 0.5f,
                                              (refined[index].y + midpointAverage.y) * 0.5f,
                                              (refined[index].z + midpointAverage.z) * 0.5f};
    }
}

void SaberTipTrail::LateUpdate() {
    auto *d = GetNativeData();
    if (!_tracker || !_lineRenderer)
        return;

    bool hasLength = (d->trailData.length > 0);
    _lineRenderer->set_enabled(hasLength);
    if (!hasLength)
        return;

    // Estimate tip speed
    float tipSpeed = std::sqrt(TrailMotionSpeedSquared(*_tracker));

    // Sample coarse poses from history
    UnityEngine::Vector3 localOffset = d->trailData.position;
    _tracker->SampleNonAlloc(d->coarseCount, d->trailData.length * 0.001f, d->poseBuffer.data());

    for (int i = 0; i < d->coarseCount; i++) {
        d->coarsePositions[i] = d->poseBuffer[i].position + (d->poseBuffer[i].rotation * localOffset);
    }

    // Refine 2x (e.g. 24 -> 47 -> 93)
    RefinePositions(d->coarsePositions, d->refinedPositions);
    RefinePositions(d->refinedPositions, d->refinedPositions2);

    // Copy to LineRenderer array
    if (!_linePositions || _linePositions.size() != d->refinedCount2) {
        _linePositions = ArrayW<UnityEngine::Vector3>(d->refinedCount2);
        _lineRenderer->set_positionCount(d->refinedCount2);
    }
    for (int i = 0; i < d->refinedCount2; i++) {
        _linePositions[i] = d->refinedPositions2[i];
    }
    _lineRenderer->SetPositions(_linePositions);

    // Motion activation & opacity decay
    float activation = std::clamp(d->trailData.motionActivation, 0.0f, 1.0f);
    if (activation <= 0.001f) {
        d->opacity = 1.0f;
    } else {
        tipSpeed *= 0.8f;
        float expFactor = std::exp((1.0f - activation) * 2.2f);
        float threshold = 0.8f * activation;
        float gated = std::clamp((tipSpeed - threshold) * expFactor, 0.0f, 1.0f);
        float decay = 3.0f * expFactor;
        float dt = UnityEngine::Time::get_deltaTime();
        float target = 0.0f;
        float diff = target - d->opacity;
        float step = dt * decay;
        if (std::abs(diff) <= step)
            d->opacity = target;
        else
            d->opacity += (diff > 0.0f ? step : -step);
        d->opacity = std::max(gated, d->opacity);
    }

    UpdateGradient(d->opacity * d->trailData.opacity);
}

void SaberTipTrail::UpdateGradient(float opacity) {
    auto *d = GetNativeData();
    if (!_lineRenderer || !_gradient)
        return;

    auto colorKeys = ArrayW<UnityEngine::GradientColorKey>(8);
    for (int i = 0; i < 8; ++i) {
        float t = i / 7.f;
        colorKeys[i] = UnityEngine::GradientColorKey(TrailColor(d->trailData, t, d->tonemappedGame), t);
    }

    auto alphaKeys = ArrayW<UnityEngine::GradientAlphaKey>(2);
    alphaKeys[0] = UnityEngine::GradientAlphaKey(0.9f * opacity, 0.0f);
    alphaKeys[1] = UnityEngine::GradientAlphaKey(0.9f * opacity * (1.0f - d->trailData.fade), 1.0f);

    _gradient->SetKeys(colorKeys, alphaKeys);
    _lineRenderer->set_colorGradient(_gradient);
}

} // namespace VainSabers
