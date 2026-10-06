#include "MenuSabers.hpp"
#include "BlurSaber.hpp"
#include "PluginConfig.hpp"
#include "VainSabersAssets.hpp"
#include "PresetLoader.hpp"
#include "VRUIControls/VRPointer.hpp"
#include "VRUIControls/VRLaserPointer.hpp"
#include "GlobalNamespace/VRController.hpp"
#include "UnityEngine/Renderer.hpp"
#include "UnityEngine/XR/XRNode.hpp"
namespace VainSabers::MenuSabers {
namespace {
struct PointerVisuals {
    UnityW<VRUIControls::VRPointer> pointer;
    UnityW<UnityEngine::GameObject> laserRoot, dotRoot;
    UnityW<BlurSaberPart> laser;
    UnityW<MovementTracker> tracker;
    UnityW<BlurSaber> dot;
    std::string preset;
    int lastLaserMode = -1, colorSide = -1;
    float lastBlur = -1;
    UnityEngine::Color lastColor{};
};
std::vector<PointerVisuals> visuals;
} // namespace
void RefreshPointers() {
    for (auto &v : visuals) {
        v.preset.clear();
        v.lastLaserMode = -1;
        v.lastBlur = -1;
    }
}
void UpdatePointers(VRUIControls::VRPointer *pointer) {
    if (!pointer)
        return;
    std::erase_if(visuals, [](auto &v) { return !v.pointer; });
    auto found =
        std::find_if(visuals.begin(), visuals.end(), [pointer](auto &v) { return v.pointer.unsafePtr() == pointer; });
    if (found == visuals.end()) {
        visuals.emplace_back();
        found = visuals.end() - 1;
        found->pointer = pointer;
    }
    auto &v = *found;
    auto &cfg = GetPluginConfig();
    int pm = cfg.enabled ? cfg.pointerMode : 0, lm = cfg.enabled ? cfg.laserMode : 0;
    auto controller = pointer->get_lastSelectedVrController();
    auto ray = controller ? controller->get_viewAnchorTransform() : nullptr;
    auto laser = pointer->____laserPointer;
    auto cursor = pointer->____cursorTransform;
    bool active = pointer->get_isActiveAndEnabled();
    if (laser && laser->____renderer)
        laser->____renderer->set_enabled(lm == 0);
    if (cursor)
        for (auto renderer : cursor->get_gameObject()->GetComponentsInChildren<UnityEngine::Renderer *>(true))
            renderer->set_enabled(pm == 0);
    if (pm == 0 && lm == 0) {
        if (v.laserRoot)
            v.laserRoot->SetActive(false);
        if (v.dotRoot)
            v.dotRoot->SetActive(false);
        return;
    }
    if (!ray)
        return;
    if (!Assets::IsLoaded() && !Assets::LoadAssets())
        return;
    int side = controller->get_node() == UnityEngine::XR::XRNode::LeftHand ? 0 : 1;
    auto color = GetColor(side == 0);
    bool colorChanged = v.colorSide != side || v.lastColor.r != color.r || v.lastColor.g != color.g ||
                        v.lastColor.b != color.b || v.lastColor.a != color.a;
    if (lm == 1 && !v.laser) {
        v.laserRoot = UnityEngine::GameObject::New_ctor("VainSabers Menu Laser");
        v.laserRoot->set_layer(5);
        v.laserRoot->get_transform()->SetParent(pointer->get_transform(), false);
        v.tracker = v.laserRoot->AddComponent<MovementTracker *>();
        v.tracker->Init(ray);
        auto go = UnityEngine::GameObject::New_ctor("Blur Laser");
        go->set_layer(5);
        go->get_transform()->SetParent(v.laserRoot->get_transform(), false);
        v.laser = go->AddComponent<BlurSaberPart *>();
        v.laser->Init(v.tracker);
        v.laser->SetColor(color);
    }
    if (v.laser) {
        v.tracker->_target = ray;
        if (v.lastLaserMode != lm || v.lastBlur != cfg.menuPointerLaserBlurFactor) {
            PartData part;
            part.length = 10;
            part.startRadius = part.endRadius = .0018f;
            part.startGlow = part.endGlow = 1.5f;
            part.blurFade = 16;
            part.blur = cfg.menuPointerLaserBlurFactor;
            v.laser->ApplyPartData(part, false);
            v.lastLaserMode = lm;
            v.lastBlur = cfg.menuPointerLaserBlurFactor;
        }
        v.laserRoot->get_transform()->set_position(ray->get_position());
        v.laserRoot->get_transform()->set_rotation(ray->get_rotation());
        float length = cursor && cursor->get_gameObject()->get_activeInHierarchy()
                           ? UnityEngine::Vector3::Distance(ray->get_position(), cursor->get_position())
                           : 10;
        v.laser->get_transform()->set_localScale({1, 1, std::clamp(length, .01f, 10.f) / 10});
        if (colorChanged)
            v.laser->SetColor(color);
        v.laserRoot->SetActive(active && lm == 1 && laser && laser->get_gameObject()->get_activeInHierarchy());
    }
    if (pm == 1 && !v.dot) {
        v.dotRoot = UnityEngine::GameObject::New_ctor("VainSabers Menu Dot");
        v.dotRoot->set_layer(5);
        v.dotRoot->get_transform()->SetParent(pointer->get_transform(), false);
        v.dot = v.dotRoot->AddComponent<BlurSaber *>();
        v.dot->Init(v.dotRoot->get_transform(), color, false);
    }
    if (v.dot) {
        if (v.preset != cfg.menuPointerDotPreset) {
            Preset p;
            if (PresetLoader::LoadByName(cfg.menuPointerDotPreset, p)) {
                p.useCustomTrails = true;
                v.dot->ApplyPreset(p);
            }
            v.preset = cfg.menuPointerDotPreset;
        }
        if (cursor) {
            v.dotRoot->get_transform()->set_position(cursor->get_position());
            v.dotRoot->get_transform()->set_rotation(ray->get_rotation());
        }
        if (colorChanged)
            v.dot->SetColor(color);
        v.dotRoot->SetActive(active && pm == 1 && cursor && cursor->get_gameObject()->get_activeInHierarchy());
    }
    v.colorSide = side;
    v.lastColor = color;
}
} // namespace VainSabers::MenuSabers
