#include "BlurSaber.hpp"
#include "VainSabersAssets.hpp"
#include "PluginConfig.hpp"
#include "PresetLoader.hpp"
#include "MenuSabers.hpp"
#include "GlobalNamespace/SaberType.hpp"
#include "SaberRibbonTrail.hpp"
#include "SaberTipTrail.hpp"
#include "UnityEngine/GameObject.hpp"
#include "UnityEngine/Transform.hpp"
#include "UnityEngine/Shader.hpp"
#include "UnityEngine/Object.hpp"
#include <algorithm>

DEFINE_TYPE(VainSabers, BlurSaber);

namespace VainSabers {

void BlurSaber::Awake() {
    _saberColor = UnityEngine::Color{1.0f, 1.0f, 1.0f, 1.0f};
    _isLeft = false;
    _colorManager = nullptr;
    _followMenuColors = false;
}

void BlurSaber::Start() {}

void BlurSaber::Update() {}

void BlurSaber::LateUpdate() {
    if (_colorManager)
        SetColor(_colorManager->ColorForSaberType(_isLeft ? GlobalNamespace::SaberType::SaberA
                                                          : GlobalNamespace::SaberType::SaberB));
    else if (_followMenuColors)
        SetColor(MenuSabers::GetColor(_isLeft));
}

void BlurSaber::OnDestroy() {}

UnityEngine::Color BlurSaber::SquarePreserveLuminance(UnityEngine::Color c) {
    float lum = 0.299f * c.r + 0.587f * c.g + 0.114f * c.b;
    float r2 = c.r * c.r;
    float g2 = c.g * c.g;
    float b2 = c.b * c.b;
    float lum2 = 0.299f * r2 + 0.587f * g2 + 0.114f * b2;
    float scale = (lum2 > 0.00001f) ? (lum / lum2) : 0.0f;

    return UnityEngine::Color{std::clamp(r2 * scale, 0.0f, 1.0f), std::clamp(g2 * scale, 0.0f, 1.0f),
                              std::clamp(b2 * scale, 0.0f, 1.0f), c.a};
}

void BlurSaber::Init(UnityEngine::Transform *target, UnityEngine::Color color, bool isLeft) {
    _saberColor = color;
    _isLeft = isLeft;

    auto go = this->get_gameObject();

    // 1. Setup MovementTracker on this GameObject
    _tracker = go->GetComponent<MovementTracker *>();
    if (!_tracker) {
        _tracker = go->AddComponent<MovementTracker *>();
    }
    _tracker->Init(target);

    // 2. Set global blur softness
    const auto &config = GetPluginConfig();
    UnityEngine::Shader::SetGlobalFloat(UnityEngine::Shader::PropertyToID(StringW("_VainSaberBlurSoftness")),
                                        config.blurSoftness);

    // 3. Load preset
    SetPreset(StringW(config.currentSaber));

    VS_LOG("BlurSaber initialized successfully on target %p (isLeft=%d, preset=%s) with color (%.2f, %.2f, %.2f)",
           (void *)target, isLeft ? 1 : 0, config.currentSaber.c_str(), color.r, color.g, color.b);
}

void BlurSaber::SetPreset(StringW presetName) {
    std::string nameStr = static_cast<std::string>(presetName);
    if (nameStr.empty())
        nameStr = "default";

    Preset preset;
    if (!PresetLoader::LoadByName(nameStr, preset))
        return;
    ApplyPreset(preset);
    VS_LOG("Applied preset '%s': %zu parts, left=%d", nameStr.c_str(), preset.parts.size(), _isLeft ? 1 : 0);
}

void BlurSaber::ApplyPreset(const Preset &preset) {

    // 1. Destroy existing BlurSaberPart children
    auto tr = this->get_transform();
    int childCount = tr->get_childCount();
    for (int i = childCount - 1; i >= 0; i--) {
        auto child = tr->GetChild(i);
        if (child && child->GetComponent<BlurSaberPart *>()) {
            child->get_gameObject()->SetActive(false);
            UnityEngine::Object::Destroy(child->get_gameObject());
        }
    }

    // 3. Instantiate parts
    UnityEngine::Color tonemapped = SquarePreserveLuminance(
        UnityEngine::Color{_saberColor.r * 0.8f, _saberColor.g * 0.8f, _saberColor.b * 0.8f, _saberColor.a});

    int parentLayer = tr->get_gameObject()->get_layer();

    for (const auto &part : preset.parts) {
        if (part.side == SaberSide::LeftOnly && !_isLeft)
            continue;
        if (part.side == SaberSide::RightOnly && _isLeft)
            continue;

        auto partGo = UnityEngine::GameObject::New_ctor(StringW(part.name));
        partGo->set_layer(parentLayer);
        partGo->get_transform()->SetParent(tr, false);

        auto *partComp = partGo->AddComponent<BlurSaberPart *>();
        partComp->Init(_tracker);
        partComp->ApplyPartData(part, _isLeft);
        partComp->SetColor(tonemapped);
    }

    // 4. Rebuild trails (tip trails & ribbon trails)
    RebuildTrails(preset);
}

void BlurSaber::RebuildTrails(const Preset &preset) {
    auto tr = this->get_transform();
    if (!tr)
        return;

    // 1. Destroy existing trail objects
    int childCount = tr->get_childCount();
    for (int i = childCount - 1; i >= 0; i--) {
        auto child = tr->GetChild(i);
        if (child) {
            if (child->GetComponent<SaberTipTrail *>() || child->GetComponent<SaberRibbonTrail *>()) {
                child->get_gameObject()->SetActive(false);
                UnityEngine::Object::Destroy(child->get_gameObject());
            }
        }
    }

    const auto &config = GetPluginConfig();

    if (!preset.useCustomTrails) {
        // Default trails: 1 Tip trail, 1 Ribbon trail
        if (config.tipTrailMS > 0) {
            SaberTrailData tipData;
            tipData.position = {0.0f, 0.0f, 1.0f};
            tipData.color = {1.0f, 1.0f, 1.0f, 1.0f};
            tipData.customBlend = 1.0f;
            tipData.glow = 1.0f;
            tipData.opacity = 1.0f;
            tipData.width = 0.008f;
            tipData.length = config.tipTrailMS;
            tipData.queueOffset = 0;
            tipData.depthOffset = 0.0f;
            tipData.fade = 1.0f;
            tipData.motionActivation = 0.0f;

            auto tipGo = UnityEngine::GameObject::New_ctor(StringW("DefaultTipTrail"));
            tipGo->set_layer(tr->get_gameObject()->get_layer());
            tipGo->get_transform()->SetParent(tr, false);
            auto *tipTrail = tipGo->AddComponent<SaberTipTrail *>();
            tipTrail->Init(_tracker, tipData, tr);
            tipTrail->SetGameColor(_saberColor);
        }

        if (config.bladeTrailMS > 0) {
            SaberTrailData bladeData;
            bladeData.position = {0.0f, 0.0f, 1.0f};
            bladeData.color = {1.0f, 1.0f, 1.0f, 1.0f};
            bladeData.customBlend = 1.0f;
            bladeData.glow = 1.0f;
            bladeData.opacity = 0.3f;
            bladeData.width = 0.01f;
            bladeData.length = config.bladeTrailMS;
            bladeData.queueOffset = 0;
            bladeData.depthOffset = 0.0f;
            bladeData.fade = 1.0f;
            bladeData.motionActivation = 0.0f;

            auto bladeGo = UnityEngine::GameObject::New_ctor(StringW("DefaultRibbonTrail"));
            bladeGo->set_layer(tr->get_gameObject()->get_layer());
            bladeGo->get_transform()->SetParent(tr, false);
            auto *ribbonTrail = bladeGo->AddComponent<SaberRibbonTrail *>();
            ribbonTrail->Init(_tracker, bladeData, tr);
            ribbonTrail->SetGameColor(_saberColor);
        }
    } else {
        // Custom trails from preset
        int tipIdx = 0;
        for (const auto &td : preset.tipTrails) {
            auto tipGo = UnityEngine::GameObject::New_ctor(StringW("TipTrail_" + std::to_string(tipIdx++)));
            tipGo->set_layer(tr->get_gameObject()->get_layer());
            tipGo->get_transform()->SetParent(tr, false);
            auto *tipTrail = tipGo->AddComponent<SaberTipTrail *>();
            tipTrail->Init(_tracker, td, tr);
            tipTrail->SetGameColor(_saberColor);
        }

        int bladeIdx = 0;
        for (const auto &td : preset.bladeTrails) {
            auto bladeGo = UnityEngine::GameObject::New_ctor(StringW("CustomBladeTrail_" + std::to_string(bladeIdx++)));
            bladeGo->set_layer(tr->get_gameObject()->get_layer());
            bladeGo->get_transform()->SetParent(tr, false);
            auto *ribbonTrail = bladeGo->AddComponent<SaberRibbonTrail *>();
            ribbonTrail->Init(_tracker, td, tr);
            ribbonTrail->SetGameColor(_saberColor);
        }
    }
}

void BlurSaber::SetColor(UnityEngine::Color color) {
    if (_saberColor.r == color.r && _saberColor.g == color.g && _saberColor.b == color.b && _saberColor.a == color.a)
        return;
    _saberColor = color;
    UnityEngine::Color tonemapped =
        SquarePreserveLuminance(UnityEngine::Color{color.r * 0.8f, color.g * 0.8f, color.b * 0.8f, color.a});

    auto parts = this->GetComponentsInChildren<BlurSaberPart *>();
    if (parts) {
        for (int i = 0; i < parts.size(); i++) {
            if (parts[i]) {
                parts[i]->SetColor(tonemapped);
            }
        }
    }

    auto tipTrails = this->GetComponentsInChildren<SaberTipTrail *>();
    if (tipTrails) {
        for (int i = 0; i < tipTrails.size(); i++) {
            if (tipTrails[i]) {
                tipTrails[i]->SetGameColor(color);
            }
        }
    }

    auto ribbonTrails = this->GetComponentsInChildren<SaberRibbonTrail *>();
    if (ribbonTrails) {
        for (int i = 0; i < ribbonTrails.size(); i++) {
            if (ribbonTrails[i]) {
                ribbonTrails[i]->SetGameColor(color);
            }
        }
    }
}

} // namespace VainSabers
