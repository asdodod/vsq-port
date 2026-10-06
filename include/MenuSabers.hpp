#pragma once
#include "PresetData.hpp"
namespace GlobalNamespace {
class VRController;
class ColorScheme;
} // namespace GlobalNamespace
namespace VRUIControls {
class VRPointer;
}
namespace VainSabers::MenuSabers {
void Bind(GlobalNamespace::VRController *left, GlobalNamespace::VRController *right);
void Refresh(const Preset *preview = nullptr);
void HidePreviewSabers(bool hidden);
UnityEngine::Color GetColor(bool isLeft);
void SetColorScheme(GlobalNamespace::ColorScheme *scheme);
void RefreshPointers();
void UpdatePointers(VRUIControls::VRPointer *pointer);
} // namespace VainSabers::MenuSabers
