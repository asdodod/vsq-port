#include "BlurSaberPart.hpp"
#include "VainSabersAssets.hpp"
#include "PluginConfig.hpp"
#include "PresetResources.hpp"
#include "TrailResources.hpp"
#include "PresetGeometry.hpp"
#include "UnityEngine/ImageConversion.hpp"
#include "UnityEngine/Time.hpp"
#include "UnityEngine/Bounds.hpp"
#include "UnityEngine/GameObject.hpp"
#include "UnityEngine/Transform.hpp"
#include "UnityEngine/Shader.hpp"
#include "UnityEngine/Object.hpp"
#include "UnityEngine/HideFlags.hpp"
#include "UnityEngine/Texture2D.hpp"
#include "UnityEngine/TextureFormat.hpp"
#include "UnityEngine/TextureWrapMode.hpp"
#include "UnityEngine/FilterMode.hpp"
#include <algorithm>
#include <cmath>

DEFINE_TYPE(VainSabers, BlurSaberPart);

namespace VainSabers {
static UnityEngine::Texture2D *MakeGradient(const std::vector<FloatGradientKey> &keys, float fallback,
                                            float rimFactor = 0, float rimPower = 3) {
    // Float pixels preserve the PC's signed rim addends; RGBA32 would clamp them.
    auto texture = UnityEngine::Texture2D::New_ctor(128, 1, UnityEngine::TextureFormat::RGBAFloat, false, true);
    texture->set_wrapMode(UnityEngine::TextureWrapMode::Clamp);
    texture->set_filterMode(UnityEngine::FilterMode::Bilinear);
    texture->set_hideFlags(UnityEngine::HideFlags::DontSave);
    ArrayW<UnityEngine::Color> pixels(128);
    for (int i = 0; i < 128; ++i) {
        const float t = i / 127.0f;
        const float value = EvaluateGradient(
            keys, t, keys.empty() && rimFactor != 0 ? rimFactor * std::pow(t, std::max(rimPower, .0001f)) : fallback);
        pixels[i] = {value, value, value, 1};
    }
    texture->SetPixels(pixels);
    texture->Apply(false, true);
    return texture;
}

struct BlurShaderProps {
    int32_t _VertexEnabled = 0;
    int32_t _HistPos = 0;
    int32_t _HistFwd = 0;
    int32_t _HistUp = 0;
    int32_t _HistCount = 0;
    int32_t _VertexBlurFade = 0;
    int32_t _VertexHueShift = 0;
    int32_t _VertexCustomColor = 0;
    int32_t _VertexGlowMul = 0;
    int32_t _VertexOpacityMul = 0;
    int32_t _VertexEnableRoundedNormals = 0;
    int32_t _VertexLength = 0;
    int32_t _VertexGeometry = 0;
    int32_t _DepthOffset = 0;
    // Fragment-stage properties the C# version sets but we were missing
    int32_t _Glow = 0;
    int32_t _RimPowerGradient = 0;
    int32_t _RimPerpendicular = 0;
    int32_t _GlowAddendGradient = 0;
    int32_t _OpacityMultiplierGradient = 0;
    int32_t _ColorTexEnabled = 0;
    int32_t _GlowTexEnabled = 0;
    int32_t _VainSaberBlurSoftness = 0;

    static const BlurShaderProps &Get() {
        static BlurShaderProps s_props = [] {
            BlurShaderProps p;
            p._VertexEnabled = UnityEngine::Shader::PropertyToID(StringW("_VertexEnabled"));
            p._HistPos = UnityEngine::Shader::PropertyToID(StringW("_HistPos"));
            p._HistFwd = UnityEngine::Shader::PropertyToID(StringW("_HistFwd"));
            p._HistUp = UnityEngine::Shader::PropertyToID(StringW("_HistUp"));
            p._HistCount = UnityEngine::Shader::PropertyToID(StringW("_HistCount"));
            p._VertexBlurFade = UnityEngine::Shader::PropertyToID(StringW("_VertexBlurFade"));
            p._VertexHueShift = UnityEngine::Shader::PropertyToID(StringW("_VertexHueShift"));
            p._VertexCustomColor = UnityEngine::Shader::PropertyToID(StringW("_VertexCustomColor"));
            p._VertexGlowMul = UnityEngine::Shader::PropertyToID(StringW("_VertexGlowMul"));
            p._VertexOpacityMul = UnityEngine::Shader::PropertyToID(StringW("_VertexOpacityMul"));
            p._VertexEnableRoundedNormals = UnityEngine::Shader::PropertyToID(StringW("_VertexEnableRoundedNormals"));
            p._VertexLength = UnityEngine::Shader::PropertyToID(StringW("_VertexLength"));
            p._VertexGeometry = UnityEngine::Shader::PropertyToID(StringW("_VertexGeometry"));
            p._DepthOffset = UnityEngine::Shader::PropertyToID(StringW("_DepthOffset"));
            p._Glow = UnityEngine::Shader::PropertyToID(StringW("_Glow"));
            p._RimPowerGradient = UnityEngine::Shader::PropertyToID(StringW("_RimPowerGradient"));
            p._RimPerpendicular = UnityEngine::Shader::PropertyToID(StringW("_RimPerpendicular"));
            p._GlowAddendGradient = UnityEngine::Shader::PropertyToID(StringW("_GlowAddendGradient"));
            p._OpacityMultiplierGradient = UnityEngine::Shader::PropertyToID(StringW("_OpacityMultiplierGradient"));
            p._ColorTexEnabled = UnityEngine::Shader::PropertyToID(StringW("_ColorTexEnabled"));
            p._GlowTexEnabled = UnityEngine::Shader::PropertyToID(StringW("_GlowTexEnabled"));
            p._VainSaberBlurSoftness = UnityEngine::Shader::PropertyToID(StringW("_VainSaberBlurSoftness"));
            return p;
        }();
        return s_props;
    }
};

BlurPartNativeData *BlurSaberPart::GetNativeData() {
    if (!_nativeDataHandle) {
        _nativeDataHandle = reinterpret_cast<int64_t>(new BlurPartNativeData());
    }
    return reinterpret_cast<BlurPartNativeData *>(_nativeDataHandle);
}

void BlurSaberPart::Awake() {
    _customColor = UnityEngine::Color{1.0f, 1.0f, 1.0f, 1.0f};

    if (!_nativeDataHandle) {
        _nativeDataHandle = reinterpret_cast<int64_t>(new BlurPartNativeData());
    }

    if (!m_vertexHistPos) {
        m_vertexHistPos = ArrayW<UnityEngine::Vector4>(16);
        m_vertexHistFwd = ArrayW<UnityEngine::Vector4>(16);
        m_vertexHistUp = ArrayW<UnityEngine::Vector4>(16);

        for (int i = 0; i < 16; i++) {
            m_vertexHistPos[i] = UnityEngine::Vector4{0.0f, 0.0f, 0.0f, 1.0f};
            m_vertexHistFwd[i] = UnityEngine::Vector4{0.0f, 0.0f, 1.0f, 0.0f};
            m_vertexHistUp[i] = UnityEngine::Vector4{0.0f, 1.0f, 0.0f, 0.0f};
        }
    }

    if (!m_propertyBlock) {
        m_propertyBlock = UnityEngine::MaterialPropertyBlock::New_ctor();
    }
}

UnityEngine::Material *BlurSaberPart::GetActiveBaseMaterial() {
    auto *d = GetNativeData();
    UnityEngine::Material *baseMat = nullptr;
    if (d->lit) {
        baseMat =
            d->inverted ? Assets::InvertedLitSaberMaterial.unsafePtr() : Assets::NormalLitSaberMaterial.unsafePtr();
    } else {
        baseMat = d->inverted ? Assets::InvertedSaberMaterial.unsafePtr() : Assets::NormalSaberMaterial.unsafePtr();
    }
    if (!baseMat) {
        baseMat = Assets::NormalSaberMaterial.unsafePtr();
    }
    return baseMat;
}

void BlurSaberPart::EnsureRuntimeMaterial() {
    if (_material)
        return;

    auto *baseMat = GetActiveBaseMaterial();
    if (baseMat) {
        _material = UnityEngine::Object::Instantiate<UnityEngine::Material *>(baseMat);
        if (_material) {
            _material->set_name(StringW("Saber (Instance)"));
            _material->set_hideFlags(UnityEngine::HideFlags(static_cast<int32_t>(UnityEngine::HideFlags::DontSave)));
        }
    }

    if (!_material && Assets::BlurPartShader) {
        _material = UnityEngine::Material::New_ctor(Assets::BlurPartShader.unsafePtr());
        if (_material) {
            _material->set_name(StringW("Saber (Instance)"));
            _material->set_hideFlags(UnityEngine::HideFlags(static_cast<int32_t>(UnityEngine::HideFlags::DontSave)));
        }
    }
}

void BlurSaberPart::Start() {
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

    EnsureRuntimeMaterial();
    if (_material && _meshRenderer) {
        _meshRenderer->set_sharedMaterial(_material);
        _meshRenderer->set_enabled(true);
    }

    if (!_mesh) {
        RebuildMesh();
    }
    ApplyMaterialProps();
}

void BlurSaberPart::Init(VainSabers::MovementTracker *tracker) {
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
}

void BlurSaberPart::SetColor(UnityEngine::Color color) {
    this->_customColor = color;
    if (_material) {
        const auto &data = GetNativeData()->materialData;
        const auto rim = UnityEngine::Color::Lerp(data.rimColor, color, data.fresnelCustomBlend);
        _material->SetColor(UnityEngine::Shader::PropertyToID(StringW("_RimColor")), rim);
    }
}

void BlurSaberPart::OnDestroy() {
    if (_colorTexture)
        UnityEngine::Object::Destroy(_colorTexture);
    if (_glowTexture)
        UnityEngine::Object::Destroy(_glowTexture);
    if (_rimGradient)
        UnityEngine::Object::Destroy(_rimGradient);
    if (_glowGradient)
        UnityEngine::Object::Destroy(_glowGradient);
    if (_opacityGradient)
        UnityEngine::Object::Destroy(_opacityGradient);
    if (_mesh) {
        UnityEngine::Object::Destroy(_mesh);
        _mesh = nullptr;
    }
    if (_material) {
        UnityEngine::Object::Destroy(_material);
        _material = nullptr;
    }
    if (_nativeDataHandle) {
        delete reinterpret_cast<BlurPartNativeData *>(_nativeDataHandle);
        _nativeDataHandle = 0;
    }
    m_propertyBlock = nullptr;
}

void BlurSaberPart::ApplyPartData(const VainSabers::PartData &data, bool isLeft) {
    auto tr = this->get_transform();
    UnityEngine::Vector3 pos = data.position;
    UnityEngine::Vector3 rot = data.rotation;

    if (data.mirrorOnLeft && isLeft) {
        pos.x *= -1.0f;
        rot.y *= -1.0f;
        rot.z *= -1.0f;
    }
    rot.z += GetPluginConfig().zRotationOffset;

    tr->set_localPosition(pos);
    tr->set_localEulerAngles(rot);

    auto *d = GetNativeData();
    d->isLeft = isLeft;
    d->timeStarted = UnityEngine::Time::get_time();
    d->materialData = data;
    d->length = data.length;
    d->geometryMode = data.geometryMode;
    d->hueShift = data.hueShift;

    d->startRadius = data.startRadius;
    d->startColor = data.startColor;
    d->startCustomColorWeight = data.startCustomWeight;
    d->startGlow = data.startGlow;
    d->startOpacity = data.startOpacity;

    d->endRadius = data.endRadius;
    d->endColor = data.endColor;
    d->endCustomColorWeight = data.endCustomWeight;
    d->endGlow = data.endGlow;
    d->endOpacity = data.endOpacity;

    d->inverted = data.inverted;
    d->lit = data.lit;
    d->blurFactor = data.blur;
    d->blurFadeFactor = data.blurFade;
    d->enableEndCaps = data.enableEndCaps;
    d->enableRoundedNormals = data.enableRoundedNormals;
    d->endCapExtension = data.endCapExtension;

    d->bulgeAmount = data.bulgeAmount;
    d->minimumRings = data.minimumRings;
    d->renderQueueOffset = data.renderQueueOffset;
    d->depthOffset = data.depthOffset;

    float radius = std::max(data.startRadius, data.endRadius);
    if (data.geometryMode == GeometryType::Advanced) {
        radius = .05f;
        for (auto &ring : data.rings)
            radius = std::max(radius, std::abs(ring.radius));
    }
    d->ringVerts = data.manualRingVerts
                       ? std::clamp(data.ringVertsManual, 4, 20)
                       : std::clamp(static_cast<int>(std::round(GetPluginConfig().saberQuality *
                                                                (6 + 30 * std::clamp(radius / .02f, 0.f, 1.f)))),
                                    6, 36);
    d->rings = data.rings;

    EnsureRuntimeMaterial();
    if (_material) {
        // Reapplying edited data must also switch between lit/inverted material families.
        if (auto base = GetActiveBaseMaterial())
            _material->set_shader(base->get_shader());
        _material->DisableKeyword(StringW("_GEOMETRY_SPRITE"));
        _material->DisableKeyword(StringW("_GEOMETRY_OBJ"));
        if (data.geometryMode == GeometryType::Sprite)
            _material->EnableKeyword("_GEOMETRY_SPRITE");
        if (data.geometryMode == GeometryType::Obj)
            _material->EnableKeyword("_GEOMETRY_OBJ");
        // Quest has no PC alpha-channel bloom compositor. Do not overwrite its alpha.
        _material->EnableKeyword(StringW("_DISABLE_GLOW_PASS"));
        if (data.disableDepthPrepass)
            _material->EnableKeyword(StringW("_DISABLE_DEPTH_PREPASS"));
        else
            _material->DisableKeyword(StringW("_DISABLE_DEPTH_PREPASS"));
        // SetInt writes a FLOAT in Unity 2021; the shader uniform is an actual int.
        // SetInteger on Material exists in the game's metadata, unlike MPB.SetInteger.
        _material->SetInteger(BlurShaderProps::Get()._HistCount, 16);
        auto set = [&](const char *name, float value) {
            _material->SetFloat(UnityEngine::Shader::PropertyToID(StringW(name)), value);
        };
        set("_SpecularStrength", data.specularStrength);
        set("_SpecularPower", std::max(data.specularPower, .01f));
        set("_Metallic", data.metallic);
        set("_Smoothness", data.smoothness);
        set("_CubemapStrength", data.cubemapStrength);
        set("_CubemapRotation", data.cubemapRotation);
        set("_FresnelStrength", data.fresnelStrength);
        set("_FresnelPower", std::max(data.fresnelPower, .01f));
        const auto rim = UnityEngine::Color::Lerp(data.rimColor, _customColor, data.fresnelCustomBlend);
        _material->SetColor(UnityEngine::Shader::PropertyToID(StringW("_RimColor")), rim);
    }
    if (_rimGradient)
        UnityEngine::Object::Destroy(_rimGradient);
    if (_glowGradient)
        UnityEngine::Object::Destroy(_glowGradient);
    if (_opacityGradient)
        UnityEngine::Object::Destroy(_opacityGradient);
    _rimGradient = MakeGradient(data.rimGradient, 0, data.rimFactor, data.rimPower);
    _glowGradient = MakeGradient(data.glowGradient, 0);
    _opacityGradient = MakeGradient(data.opacityGradient, 1);
    if (_colorTexture)
        UnityEngine::Object::Destroy(_colorTexture);
    if (_glowTexture)
        UnityEngine::Object::Destroy(_glowTexture);
    _colorTexture = LoadPresetTexture(data.colorTexture, data.colorTextureBase64, data.textureWrap);
    _glowTexture = LoadPresetTexture(data.glowTexture, data.glowTextureBase64, data.textureWrap);

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

    if (_material) {
        // The 0.0.5 PC release overrides the legacy material queues for every part.
        _material->set_renderQueue(3600 + d->renderQueueOffset);
    }

    if (_meshRenderer && _material) {
        _meshRenderer->set_sharedMaterial(_material);
        _meshRenderer->set_sortingOrder(100);
        _meshRenderer->set_enabled(true);
    }

    RebuildMesh();
    ApplyMaterialProps();
}

void BlurSaberPart::RebuildMesh() {
    auto *d = GetNativeData();
    if (_mesh) {
        UnityEngine::Object::Destroy(_mesh);
        _mesh = nullptr;
    }

    if (d->geometryMode == GeometryType::Sprite || d->geometryMode == GeometryType::Obj) {
        _mesh = MakePresetGeometry(d->materialData);
    } else if (d->geometryMode == GeometryType::Advanced && d->rings.size() >= 2) {
        _mesh = BlurTube::BuildAdvancedBladeTube(d->ringVerts, d->length, d->rings);
    } else {
        _mesh = BlurTube::BuildSimpleBladeTube(d->ringVerts, d->length, d->startRadius, d->endRadius, d->startColor,
                                               d->endColor, d->startGlow, d->endGlow, d->startCustomColorWeight,
                                               d->endCustomColorWeight, d->startOpacity, d->endOpacity, d->bulgeAmount,
                                               d->minimumRings, d->enableEndCaps, d->endCapExtension, d->inverted);
    }

    if (!_meshFilter) {
        auto go = this->get_gameObject();
        _meshFilter = go->GetComponent<UnityEngine::MeshFilter *>();
        if (!_meshFilter)
            _meshFilter = go->AddComponent<UnityEngine::MeshFilter *>();
    }

    if (_meshFilter && _mesh) {
        _meshFilter->set_mesh(_mesh);
    }
}

void BlurSaberPart::SampleGpuHistory() {
    if (!_tracker || !_tracker->get_transform() || !this->get_transform())
        return;

    auto *d = GetNativeData();
    const auto &config = GetPluginConfig();
    float blurTime = std::min(config.blurMS * 0.001f * d->blurFactor, 0.025f);
    _tracker->SampleNonAlloc(8, blurTime, d->coarsePoses.data());

    // Smoothing pass on coarse poses
    for (int i = 1; i < 7; i++) {
        const auto &prev = d->coarsePoses[i - 1];
        const auto &curr = d->coarsePoses[i];
        const auto &next = d->coarsePoses[i + 1];

        UnityEngine::Vector3 avgPos = (curr.position + prev.position + next.position) / 3.0f;
        UnityEngine::Vector3 avgFwd = (curr.GetForward() + prev.GetForward() + next.GetForward()).get_normalized();
        UnityEngine::Vector3 avgUp = (curr.GetUp() + prev.GetUp() + next.GetUp()).get_normalized();

        d->coarsePoses[i] = Pose(UnityEngine::Vector3::Lerp(curr.position, avgPos, 1.0f),
                                 UnityEngine::Quaternion::LookRotation(avgFwd, avgUp));
    }

    // Refine 8 poses into 16 poses
    for (int i = 0; i < 8; i++) {
        int evenIdx = 2 * i;
        if (evenIdx < 16) {
            d->refinedPoses[evenIdx] = d->coarsePoses[i];
        }
        if (evenIdx + 1 < 16 && i < 7) {
            d->refinedPoses[evenIdx + 1] = d->coarsePoses[i].LerpTo(d->coarsePoses[i + 1], 0.5f);
        }
    }
    d->refinedPoses[15] = d->coarsePoses[7];

    for (int i = 1; i < 7; i++) {
        int idx = 2 * i;
        if (idx > 0 && idx < 15) {
            Pose midpoint = d->refinedPoses[idx - 1].LerpTo(d->refinedPoses[idx + 1], 0.5f);
            d->refinedPoses[idx] = d->refinedPoses[idx].LerpTo(midpoint, 0.5f);
        }
    }

    // Transform into local space and pack uniforms
    auto tr = this->get_transform();
    UnityEngine::Matrix4x4 wtl = tr->get_worldToLocalMatrix();
    Pose localPose =
        GetTransformPose(tr.unsafePtr()).TransformPose(_tracker->get_transform()->get_worldToLocalMatrix());
    UnityEngine::Matrix4x4 localPoseMat = localPose.AsMatrix();

    for (int i = 0; i < 16; i++) {
        UnityEngine::Matrix4x4 sampleMat = d->refinedPoses[i].AsMatrix();
        UnityEngine::Matrix4x4 combined =
            UnityEngine::Matrix4x4::op_Multiply(UnityEngine::Matrix4x4::op_Multiply(wtl, sampleMat), localPoseMat);
        Pose p = Pose::FromMatrix(combined);
        UnityEngine::Vector3 fwd = p.GetForward();
        UnityEngine::Vector3 up = p.GetUp();

        m_vertexHistPos[i] = UnityEngine::Vector4{p.position.x, p.position.y, p.position.z, 1.0f};
        m_vertexHistFwd[i] = UnityEngine::Vector4{fwd.x, fwd.y, fwd.z, 0.0f};
        m_vertexHistUp[i] = UnityEngine::Vector4{up.x, up.y, up.z, 0.0f};
    }
}

void BlurSaberPart::ApplyMaterialProps() {
    EnsureRuntimeMaterial();
    if (!_meshRenderer || !_material)
        return;

    if (!m_propertyBlock) {
        m_propertyBlock = UnityEngine::MaterialPropertyBlock::New_ctor();
    }
    if (!m_vertexHistPos) {
        m_vertexHistPos = ArrayW<UnityEngine::Vector4>(16);
        m_vertexHistFwd = ArrayW<UnityEngine::Vector4>(16);
        m_vertexHistUp = ArrayW<UnityEngine::Vector4>(16);
        for (int i = 0; i < 16; i++) {
            m_vertexHistPos[i] = UnityEngine::Vector4{0.0f, 0.0f, 0.0f, 1.0f};
            m_vertexHistFwd[i] = UnityEngine::Vector4{0.0f, 0.0f, 1.0f, 0.0f};
            m_vertexHistUp[i] = UnityEngine::Vector4{0.0f, 1.0f, 0.0f, 0.0f};
        }
    }

    const auto &props = BlurShaderProps::Get();
    auto *d = GetNativeData();
    const auto &config = GetPluginConfig();

    float hue = d->hueShift, glow = 1, opacity = 1;
    auto pos = d->materialData.position, rot = d->materialData.rotation;
    const float alive = UnityEngine::Time::get_time() - d->timeStarted;
    for (auto &a : d->materialData.animators) {
        float wave = std::sin(6.28318530718f * a.frequency * alive);
        if (a.type == "HueShiftAdder")
            hue += a.speed * alive;
        else if (a.type == "HueShiftOscillator")
            hue += a.amplitude * wave;
        else if (a.type == "GlowOscillator")
            glow += a.amplitude * (.5f + .5f * wave);
        else if (a.type == "OpacityOscillator")
            opacity += a.amplitude * (.5f + .5f * wave);
        else if (a.type == "PositionOscillator") {
            float *axes[] = {&pos.x, &pos.y, &pos.z};
            *axes[std::clamp(a.axis, 0, 2)] += a.amplitude * wave;
        } else if (a.type == "RotationOscillator" || a.type == "RotationAdder") {
            float *axes[] = {&rot.x, &rot.y, &rot.z};
            *axes[std::clamp(a.axis, 0, 2)] += a.type == "RotationAdder" ? a.speed * alive : a.amplitude * wave;
        }
    }
    if (d->isLeft && d->materialData.mirrorOnLeft) {
        pos.x = -pos.x;
        rot.y = -rot.y;
        rot.z = -rot.z;
    }
    rot.z += config.zRotationOffset;
    get_transform()->set_localPosition(pos);
    get_transform()->set_localEulerAngles(rot);

    SampleGpuHistory();
    m_propertyBlock->SetFloat(props._VertexEnabled, 1.0f);
    m_propertyBlock->SetVectorArray(props._HistPos, m_vertexHistPos);
    m_propertyBlock->SetVectorArray(props._HistFwd, m_vertexHistFwd);
    m_propertyBlock->SetVectorArray(props._HistUp, m_vertexHistUp);
    m_propertyBlock->SetFloat(props._VertexBlurFade, d->blurFadeFactor);
    m_propertyBlock->SetFloat(props._VertexHueShift, hue);
    m_propertyBlock->SetVector(props._VertexCustomColor,
                               UnityEngine::Vector4{_customColor.r, _customColor.g, _customColor.b, 1.0f});
    m_propertyBlock->SetFloat(props._VertexGlowMul, glow);
    m_propertyBlock->SetFloat(props._VertexOpacityMul, opacity);
    m_propertyBlock->SetFloat(props._VertexEnableRoundedNormals, d->enableRoundedNormals ? 1.0f : 0.0f);
    m_propertyBlock->SetFloat(props._VertexLength, d->length);
    m_propertyBlock->SetFloat(props._VertexGeometry, 0.0f);
    auto id = [](const char *name) { return UnityEngine::Shader::PropertyToID(StringW(name)); };
    m_propertyBlock->SetVector(id("_VertexSpriteSize"),
                               {d->materialData.spriteSizeX, d->materialData.spriteSizeY, 0, 0});
    m_propertyBlock->SetFloat(id("_VertexObjScale"), d->materialData.objScale);
    if (_mesh && d->geometryMode == GeometryType::Obj) {
        auto bounds = _mesh->get_bounds();
        auto mn = bounds.get_min(), mx = bounds.get_max();
        m_propertyBlock->SetVector(id("_VertexObjBoundsMin"), {mn.x, mn.y, mn.z, 0});
        m_propertyBlock->SetVector(id("_VertexObjBoundsMax"), {mx.x, mx.y, mx.z, 0});
    }
    m_propertyBlock->SetFloat(props._DepthOffset, d->depthOffset + (d->inverted ? 0.0f : 0.001f));

    // Global and per-part blur softness uniform:
    m_propertyBlock->SetFloat(props._VainSaberBlurSoftness, config.blurSoftness);
    if (_material) {
        _material->SetFloat(props._VainSaberBlurSoftness, config.blurSoftness);
    }
    UnityEngine::Shader::SetGlobalFloat(props._VainSaberBlurSoftness, config.blurSoftness);

    // Fragment-stage uniforms the shader requires for proper opacity/glow:
    // _Glow multiplies vertex glow in the fragment shader. Without it (default=0), no glow.
    m_propertyBlock->SetFloat(props._Glow, 0.8f);
    // _RimPerpendicular controls fresnel blend (0 = full fresnel, 1 = perpendicular only)
    m_propertyBlock->SetFloat(props._RimPerpendicular, d->materialData.rimPerpendicular);
    // _ColorTexEnabled/_GlowTexEnabled: no custom textures, disable them
    m_propertyBlock->SetFloat(props._ColorTexEnabled, _colorTexture ? 1 : 0);
    m_propertyBlock->SetFloat(props._GlowTexEnabled, _glowTexture ? 1 : 0);
    if (_colorTexture)
        m_propertyBlock->SetTexture(id("_ColorTex"), _colorTexture);
    if (_glowTexture)
        m_propertyBlock->SetTexture(id("_GlowTex"), _glowTexture);

    // The preset's angle-dependent curves supply shading and edge transparency.
    if (_opacityGradient)
        m_propertyBlock->SetTexture(props._OpacityMultiplierGradient, _opacityGradient);
    if (_rimGradient)
        m_propertyBlock->SetTexture(props._RimPowerGradient, _rimGradient);
    if (_glowGradient)
        m_propertyBlock->SetTexture(props._GlowAddendGradient, _glowGradient);
    // The lit preset uses these in its RGB pass. Set once on the private material.
    // No texture or material allocations happen during frame updates.

    _meshRenderer->SetPropertyBlock(m_propertyBlock);
    _meshRenderer->set_sortingOrder(100);
    _meshRenderer->set_enabled(true);
}

void BlurSaberPart::LateUpdate() {
    ApplyMaterialProps();
}

} // namespace VainSabers
