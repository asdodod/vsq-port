#pragma once

#include "main.hpp"
#include "custom-types/shared/macros.hpp"
#include "UnityEngine/MonoBehaviour.hpp"
#include "UnityEngine/Transform.hpp"
#include "UnityEngine/Color.hpp"
#include "MovementTracker.hpp"
#include "BlurSaberPart.hpp"
#include "PresetData.hpp"
#include "GlobalNamespace/ColorManager.hpp"

DECLARE_CLASS_CODEGEN(VainSabers, BlurSaber, UnityEngine::MonoBehaviour) {
    DECLARE_INSTANCE_FIELD(VainSabers::MovementTracker *, _tracker);
    DECLARE_INSTANCE_FIELD(UnityEngine::Color, _saberColor);
    DECLARE_INSTANCE_FIELD(bool, _isLeft);
    DECLARE_INSTANCE_FIELD(GlobalNamespace::ColorManager *, _colorManager);
    DECLARE_INSTANCE_FIELD(bool, _followMenuColors);

    DECLARE_INSTANCE_METHOD(void, Awake);
    DECLARE_INSTANCE_METHOD(void, Start);
    DECLARE_INSTANCE_METHOD(void, Update);
    DECLARE_INSTANCE_METHOD(void, LateUpdate);
    DECLARE_INSTANCE_METHOD(void, OnDestroy);
    DECLARE_INSTANCE_METHOD(void, Init, UnityEngine::Transform *target, UnityEngine::Color color, bool isLeft);
    DECLARE_INSTANCE_METHOD(void, SetColor, UnityEngine::Color color);
    DECLARE_INSTANCE_METHOD(void, SetPreset, StringW presetName);

  public:
    void ApplyPreset(const VainSabers::Preset &preset);
    void RebuildTrails(const VainSabers::Preset &preset);
    static UnityEngine::Color SquarePreserveLuminance(UnityEngine::Color c);
};
