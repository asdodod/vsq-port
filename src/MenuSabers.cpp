#include "MenuSabers.hpp"
#include "BlurSaber.hpp"
#include "PluginConfig.hpp"
#include "VainSabersAssets.hpp"
#include "GlobalNamespace/VRController.hpp"
#include "GlobalNamespace/ColorsOverrideSettingsPanelController.hpp"
#include "GlobalNamespace/ColorSchemesSettings.hpp"
#include "GlobalNamespace/ColorScheme.hpp"
#include "UnityEngine/Object.hpp"
#include "UnityEngine/Resources.hpp"
#include "UnityEngine/Renderer.hpp"
#include "UnityEngine/GameObject.hpp"
#include "UnityEngine/XR/XRNode.hpp"

namespace VainSabers::MenuSabers {
namespace {
UnityW<GlobalNamespace::VRController> controllers[2];
UnityW<BlurSaber> sabers[2];
UnityEngine::Color menuColors[2] = {{200.f / 255, 20.f / 255, 20.f / 255, 1},
                                    {40.f / 255, 142.f / 255, 210.f / 255, 1}};
void ReadColorScheme() {
    // ColorSchemesSettings is a managed service, not a Unity Object. Read the
    // reference owned by the game's panel rather than keeping an unrooted pointer.
    auto panels =
        UnityEngine::Resources::FindObjectsOfTypeAll<GlobalNamespace::ColorsOverrideSettingsPanelController *>();
    for (auto panel : panels) {
        if (panel && panel->____colorSchemesSettings) {
            SetColorScheme(panel->____colorSchemesSettings->GetOverrideColorScheme());
            return;
        }
    }
}
void Discover() {
    auto all = UnityEngine::Object::FindObjectsOfType<GlobalNamespace::VRController *>();
    for (auto controller : all) {
        auto tr = controller->get_transform();
        if (!tr->Find(StringW("MenuHandle")))
            continue;
        int side = controller->get_node() == UnityEngine::XR::XRNode::LeftHand ? 0 : 1;
        controllers[side] = controller;
    }
}
} // namespace
UnityEngine::Color GetColor(bool isLeft) {
    return menuColors[isLeft ? 0 : 1];
}
void SetColorScheme(GlobalNamespace::ColorScheme *scheme) {
    // The PC menu uses the override scheme, with these exact Color32 defaults.
    menuColors[0] = scheme ? scheme->get_saberAColor() : UnityEngine::Color{200.f / 255, 20.f / 255, 20.f / 255, 1};
    menuColors[1] = scheme ? scheme->get_saberBColor() : UnityEngine::Color{40.f / 255, 142.f / 255, 210.f / 255, 1};
}
void Bind(GlobalNamespace::VRController *left, GlobalNamespace::VRController *right) {
    controllers[0] = left;
    controllers[1] = right;
    Refresh();
    RefreshPointers();
}
void HidePreviewSabers(bool hidden) {
    for (auto saber : sabers)
        if (saber)
            saber->get_gameObject()->SetActive(!hidden);
}
void Refresh(const Preset *preview) {
    Discover();
    ReadColorScheme();
    auto &cfg = GetPluginConfig();
    bool visible = preview || (cfg.enabled && cfg.menuMode != 0);
    if (visible && !Assets::IsLoaded() && !Assets::LoadAssets())
        visible = false;
    for (int side = 0; side < 2; ++side) {
        auto controller = controllers[side];
        if (!controller)
            continue;
        auto handle = controller->get_transform()->Find(StringW("MenuHandle"));
        if (!handle)
            continue;
        // PC hides these four handle visuals, while keeping the UI ray alive.
        for (auto name : {"Glowing", "Normal", "FakeGlow0", "FakeGlow1"}) {
            auto child = handle->Find(StringW(name));
            if (!child)
                continue;
            auto renderer = child->GetComponent<UnityEngine::Renderer *>();
            if (renderer)
                renderer->set_enabled(!visible);
        }
        auto existing = handle->Find(StringW("VainSabersMenuSaber"));
        auto saber = existing ? existing->GetComponent<BlurSaber *>() : nullptr;
        if (!saber && visible) {
            auto go = UnityEngine::GameObject::New_ctor(StringW("VainSabersMenuSaber"));
            go->set_layer(handle->get_gameObject()->get_layer());
            go->get_transform()->SetParent(handle, false);
            saber = go->AddComponent<BlurSaber *>();
            saber->Init(handle, GetColor(side == 0), side == 0);
            saber->_followMenuColors = true;
        }
        sabers[side] = saber;
        if (!saber)
            continue;
        saber->get_gameObject()->SetActive(visible);
        if (visible) {
            saber->SetColor(GetColor(side == 0));
            if (preview)
                saber->ApplyPreset(*preview);
            else
                saber->SetPreset(StringW(cfg.menuMode == 2 ? cfg.menuSaberPreset : cfg.currentSaber));
        }
    }
}
} // namespace VainSabers::MenuSabers
