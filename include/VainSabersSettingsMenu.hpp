#pragma once

#include "main.hpp"
#include "custom-types/shared/macros.hpp"
#include "HMUI/ViewController.hpp"
#include "bsml/shared/BSML-Lite.hpp"
#include "bsml/shared/BSML.hpp"

#include <string>
#include <vector>
#include "UnityEngine/MonoBehaviour.hpp"
#include "PresetDocument.hpp"
#include "bsml/shared/BSML/Components/Settings/DropdownListSetting.hpp"
#include "UnityEngine/UI/Button.hpp"

namespace VainSabers {
struct MenuEditorState;
}
DECLARE_CLASS_CODEGEN(VainSabers, VainSabersMenuHost, UnityEngine::MonoBehaviour) {
    DECLARE_INSTANCE_FIELD(UnityW<UnityEngine::GameObject>, _homeRoot);
    DECLARE_INSTANCE_FIELD(UnityW<UnityEngine::GameObject>, _floatingPanel);
    DECLARE_INSTANCE_FIELD(UnityW<UnityEngine::GameObject>, _editorRoot);
    DECLARE_INSTANCE_FIELD(UnityW<UnityEngine::GameObject>, _previewRoot);
    DECLARE_INSTANCE_FIELD(int64_t, _nativeStateHandle);
    DECLARE_INSTANCE_FIELD(bool, _syncingHome);
    DECLARE_INSTANCE_FIELD(UnityW<UnityEngine::Transform>, root);
    DECLARE_INSTANCE_FIELD(UnityW<UnityEngine::Transform>, MenuPresetContainer);
    DECLARE_INSTANCE_FIELD(UnityW<BSML::DropdownListSetting>, SaberPresetDropdown);
    DECLARE_INSTANCE_FIELD(UnityW<BSML::DropdownListSetting>, MenuPresetDropdown);
    DECLARE_INSTANCE_FIELD(UnityW<UnityEngine::UI::Button>, EditSaberButton);
    DECLARE_INSTANCE_FIELD(UnityW<UnityEngine::UI::Button>, EditMenuButton);
    DECLARE_INSTANCE_METHOD(bool, get_modEnabled);
    DECLARE_INSTANCE_METHOD(void, set_modEnabled, bool value);
    DECLARE_INSTANCE_METHOD(StringW, get_menuMode);
    DECLARE_INSTANCE_METHOD(void, set_menuMode, StringW value);
    DECLARE_INSTANCE_METHOD(StringW, get_SelectedPreset);
    DECLARE_INSTANCE_METHOD(void, set_SelectedPreset, StringW value);
    DECLARE_INSTANCE_METHOD(StringW, get_SelectedMenuPreset);
    DECLARE_INSTANCE_METHOD(void, set_SelectedMenuPreset, StringW value);
    DECLARE_INSTANCE_METHOD(ListW<System::Object *>, get_PresetNames);
    DECLARE_INSTANCE_METHOD(ListW<System::Object *>, get_MenuPresetNames);
    DECLARE_INSTANCE_METHOD(ListW<System::Object *>, get_menuModeChoices);
    DECLARE_INSTANCE_METHOD(void, UpdatePresetDropdown);
    DECLARE_INSTANCE_METHOD(void, EditSaberPreset);
    DECLARE_INSTANCE_METHOD(void, EditMenuPreset);
    DECLARE_INSTANCE_METHOD(void, CreateNewPreset);
    DECLARE_INSTANCE_METHOD(void, ToggleSettingsPanel);
    DECLARE_INSTANCE_METHOD(void, OnDestroy);
    DECLARE_INSTANCE_METHOD(void, OnDisable);

  public:
    VainSabers::MenuEditorState *State();
    void ShowHome();
    void OpenSettings();
    void ClosePanel();
    void OpenEditor(const std::string &name);
    void BuildEditor();
    void Preview();
    void SavePreset();
    void CreatePreset();
    void ExportPreset();
    void DeletePreset();
};

// VainSabersSettingsViewController must inherit HMUI::ViewController to be used
// with BSML::Register::RegisterSettingsMenu<T>
DECLARE_CLASS_CODEGEN(VainSabers, VainSabersSettingsViewController, HMUI::ViewController) {
    DECLARE_INSTANCE_FIELD(UnityW<UnityEngine::GameObject>, _contentRoot);
    DECLARE_OVERRIDE_METHOD_MATCH(void, DidActivate, &HMUI::ViewController::DidActivate, bool firstActivation,
                                  bool addedToHierarchy, bool screenSystemEnabling);

  public:
    static void Register();
};
