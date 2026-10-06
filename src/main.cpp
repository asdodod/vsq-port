#include "main.hpp"
#include "scotland2/shared/modloader.h"
#include "VainSabersAssets.hpp"
#include "PluginConfig.hpp"
#include "PresetLoader.hpp"
#include "VainSabersSettingsMenu.hpp"
#include "MenuSabers.hpp"

// Custom types
#include "custom-types/shared/register.hpp"

// Cordl includes for game types
#include "GlobalNamespace/SaberModelController.hpp"
#include "GlobalNamespace/Saber.hpp"
#include "GlobalNamespace/SaberType.hpp"
#include "GlobalNamespace/SaberTrail.hpp"
#include "GlobalNamespace/ColorManager.hpp"
#include "GlobalNamespace/ColorSchemesSettings.hpp"
#include "GlobalNamespace/ColorScheme.hpp"
#include "GlobalNamespace/TubeBloomPrePassLight.hpp"
#include "GlobalNamespace/SetSaberFakeGlowColor.hpp"
#include "GlobalNamespace/SetSaberGlowColor.hpp"
#include "GlobalNamespace/Parametric3SliceSpriteController.hpp"
#include "GlobalNamespace/VRController.hpp"
#include "VRUIControls/VRPointer.hpp"

// Unity types
#include "UnityEngine/Transform.hpp"
#include "UnityEngine/GameObject.hpp"
#include "UnityEngine/Color.hpp"
#include "UnityEngine/MeshRenderer.hpp"
#include "UnityEngine/Renderer.hpp"

static modloader::ModInfo modInfo{MOD_ID, VERSION, 0};

// Loads the config from disk using our modInfo, then returns it for use
Configuration &getConfig() {
    static Configuration config(modInfo);
    return config;
}

#include "BlurSaber.hpp"

// ============================================================================
// Hook: SaberModelController::Init
// Intercepts when the game creates a saber model
// ============================================================================
MAKE_HOOK_FIND_CLASS_UNSAFE_INSTANCE(SaberModelController_Init, "", "SaberModelController", "Init", void,
                                     GlobalNamespace::SaberModelController *self, UnityEngine::Transform *parent,
                                     GlobalNamespace::Saber *saber, UnityEngine::Color trailTintColor) {
    // Call original first so the base saber model initializes
    SaberModelController_Init(self, parent, saber, trailTintColor);

    const auto &config = VainSabers::GetPluginConfig();
    if (!config.enabled) {
        VS_LOG("VainSabers is disabled in config, skipping attachment.");
        return;
    }

    VS_LOG("=== SaberModelController::Init hooked! ===");

    // Get the saber color and type from the color manager
    auto *colorMgr = self->____colorManager;
    UnityEngine::Color saberColor = trailTintColor;
    int saberTypeVal = 0;
    bool isLeft = saber && saber->get_saberType() == GlobalNamespace::SaberType::SaberA;

    if (colorMgr && saber) {
        auto saberType = saber->get_saberType();
        saberTypeVal = static_cast<int>(saberType);
        isLeft = (saberType == GlobalNamespace::SaberType::SaberA);
        saberColor = colorMgr->ColorForSaberType(saberType);
        VS_LOG("Saber spawned! Type: %d (%s), Color: (%.2f, %.2f, %.2f, %.2f), Parent: %p", saberTypeVal,
               isLeft ? "Left" : "Right", saberColor.r, saberColor.g, saberColor.b, saberColor.a, (void *)parent);
    } else {
        VS_LOG("ColorManager or saber null, parent: %p", (void *)parent);
    }

    // Keep the game's visible saber if assets fail instead of hiding it first.
    if (!parent || (!VainSabers::Assets::IsLoaded() && !VainSabers::Assets::LoadAssets())) {
        VS_LOG("Cannot attach VainSabers; leaving vanilla saber visible");
        return;
    }
    // Hide vanilla saber visuals and bloom/glow textures if configured
    if (config.hideVanillaSaber && self) {
        // Disable the entire GameObject of the vanilla saber model if separate from parent
        auto selfGo = self->get_gameObject();
        if (selfGo && parent && selfGo != parent->get_gameObject()) {
            selfGo->SetActive(false);
        }

        // Disable TubeBloomPrePassLight (fake bloom quad generator)
        if (self->____saberLight) {
            self->____saberLight->set_enabled(false);
            auto lightGo = self->____saberLight->get_gameObject();
            if (lightGo && parent && lightGo != parent->get_gameObject()) {
                lightGo->SetActive(false);
            }
        }

        // Disable vanilla trail
        if (self->____saberTrail) {
            self->____saberTrail->set_enabled(false);
        }

        // Disable fake glow components and objects
        if (self->____setSaberFakeGlowColors) {
            for (int i = 0; i < self->____setSaberFakeGlowColors.size(); i++) {
                auto fakeGlow = self->____setSaberFakeGlowColors[i];
                if (fakeGlow) {
                    fakeGlow->set_enabled(false);
                    auto fgGo = fakeGlow->get_gameObject();
                    if (fgGo && parent && fgGo != parent->get_gameObject()) {
                        fgGo->SetActive(false);
                    }
                }
            }
        }

        // Disable glow components and objects
        if (self->____setSaberGlowColors) {
            for (int i = 0; i < self->____setSaberGlowColors.size(); i++) {
                auto glow = self->____setSaberGlowColors[i];
                if (glow) {
                    glow->set_enabled(false);
                    auto gGo = glow->get_gameObject();
                    if (gGo && parent && gGo != parent->get_gameObject()) {
                        gGo->SetActive(false);
                    }
                }
            }
        }

        // Disable all renderers (MeshRenderer, SpriteRenderer, LineRenderer, TrailRenderer, etc.) under self
        auto allRenderers = self->GetComponentsInChildren<UnityEngine::Renderer *>();
        if (allRenderers) {
            for (int i = 0; i < allRenderers.size(); i++) {
                if (allRenderers[i]) {
                    allRenderers[i]->set_enabled(false);
                }
            }
        }

        // Sweep parent for any TubeBloomPrePassLight or Parametric3SliceSpriteController outside self
        if (parent) {
            auto parentGo = parent->get_gameObject();
            if (parentGo) {
                auto parentLights = parentGo->GetComponentsInChildren<GlobalNamespace::TubeBloomPrePassLight *>();
                if (parentLights) {
                    for (int i = 0; i < parentLights.size(); i++) {
                        if (parentLights[i]) {
                            parentLights[i]->set_enabled(false);
                            auto lgo = parentLights[i]->get_gameObject();
                            if (lgo && lgo != parentGo) {
                                lgo->SetActive(false);
                            }
                        }
                    }
                }
                auto parentSprites =
                    parentGo->GetComponentsInChildren<GlobalNamespace::Parametric3SliceSpriteController *>();
                if (parentSprites) {
                    for (int i = 0; i < parentSprites.size(); i++) {
                        if (parentSprites[i]) {
                            parentSprites[i]->set_enabled(false);
                            auto sgo = parentSprites[i]->get_gameObject();
                            if (sgo && sgo != parentGo) {
                                sgo->SetActive(false);
                            }
                        }
                    }
                }
            }
        }

        VS_LOG("Vanilla saber visuals and bloom/glow textures hidden for %s saber", isLeft ? "Left" : "Right");
    }

    if (parent) {
        auto parentGo = parent->get_gameObject();
        if (parentGo) {
            auto *blurSaber = parentGo->GetComponent<VainSabers::BlurSaber *>();
            if (!blurSaber) {
                blurSaber = parentGo->AddComponent<VainSabers::BlurSaber *>();
                blurSaber->Init(parent, saberColor, isLeft);
                VS_LOG("Phase 5 & 6 SUCCESS: Attached BlurSaber to saber parent (isLeft=%d, preset=%s)!",
                       isLeft ? 1 : 0, config.currentSaber.c_str());
            } else {
                blurSaber->SetColor(saberColor);
                VS_LOG("Updated existing BlurSaber color on parent %p!", (void *)parent);
            }
            blurSaber->_colorManager = colorMgr;
        }
    }
}

// PC refreshes menu sabers when the override scheme changes. Capture colors as
// values only; the game retains ownership of the settings service and scheme.
MAKE_HOOK_FIND_CLASS_UNSAFE_INSTANCE(ColorSchemesSettings_GetOverrideColorScheme, "", "ColorSchemesSettings",
                                     "GetOverrideColorScheme", GlobalNamespace::ColorScheme *,
                                     GlobalNamespace::ColorSchemesSettings *self) {
    auto scheme = ColorSchemesSettings_GetOverrideColorScheme(self);
    VainSabers::MenuSabers::SetColorScheme(scheme);
    return scheme;
}

// ============================================================================
// Hook: VRPointer::CreateLaserPointers
// Strips fake bloom textures (FakeGlow0, FakeGlow1, etc.) from menu pointers
// ============================================================================
MAKE_HOOK_FIND_CLASS_UNSAFE_INSTANCE(VRPointer_CreateLaserPointers, "VRUIControls", "VRPointer", "CreateLaserPointers",
                                     bool, VRUIControls::VRPointer *self) {
    bool res = VRPointer_CreateLaserPointers(self);

    VainSabers::MenuSabers::Bind(self->____leftVRController, self->____rightVRController);

    return res;
}

// ============================================================================
// Mod entry points
MAKE_HOOK_FIND_CLASS_UNSAFE_INSTANCE(VRPointer_LateUpdate, "VRUIControls", "VRPointer", "LateUpdate", void,
                                     VRUIControls::VRPointer *self) {
    VRPointer_LateUpdate(self);
    VainSabers::MenuSabers::UpdatePointers(self);
}
// ============================================================================

// Called at the early stages of game loading
MOD_EXTERN_FUNC void setup(CModInfo *info) noexcept {
    *info = modInfo.to_c();

    getConfig().Load();
    VainSabers::GetPluginConfig().Load();

    VS_LOG("VainSabers Quest v0.0.5 - setup complete!");
}

// Called later on in the game loading - Unity and il2cpp are fully ready
MOD_EXTERN_FUNC void late_load() noexcept {
    il2cpp_functions::Init();

    VS_LOG("VainSabers Quest - late_load called!");

    // Register custom types
    custom_types::Register::AutoRegister();

    // Load the VainSabers AssetBundle and log all asset names
    VS_LOG("VainSabers Quest - loading vs_assets bundle (Phase 1)...");
    bool assetsOk = VainSabers::Assets::LoadAssets();
    if (assetsOk) {
        VS_LOG("VainSabers Quest - Phase 1 SUCCESS: vs_assets bundle loaded!");
    } else {
        VS_LOG("VainSabers Quest - Phase 1 WARNING: vs_assets bundle could not be loaded!");
    }

    // Ensure default presets exist on disk (Phase 5 & 6)
    VS_LOG("VainSabers Quest - ensuring default presets exist (Phase 5 & 6)...");
    VainSabers::PluginConfig::EnsureDefaultPresetsExist();

    // Install our hook on SaberModelController::Init (Phase 2)
    VS_LOG("VainSabers Quest - installing SaberModelController::Init hook (Phase 2)...");
    INSTALL_HOOK(vsLogger, SaberModelController_Init);
    INSTALL_HOOK(vsLogger, ColorSchemesSettings_GetOverrideColorScheme);

    // Install our hook on VRPointer::CreateLaserPointers for menu bloom stripping
    VS_LOG("VainSabers Quest - installing VRPointer::CreateLaserPointers hook...");
    INSTALL_HOOK(vsLogger, VRPointer_CreateLaserPointers);
    INSTALL_HOOK(vsLogger, VRPointer_LateUpdate);

    // Register settings menu
    VS_LOG("VainSabers Quest - registering settings menu...");
    VainSabers::VainSabersSettingsViewController::Register();

    VS_LOG("VainSabers Quest - fully loaded and ready!");
}
