#include "VainSabersUI.hpp"
#include "PcUI.hpp"
#include "PcHomeMarkup.hpp"
#include "PluginConfig.hpp"
#include "PresetLoader.hpp"
#include "MenuSabers.hpp"
#include <filesystem>
#include <algorithm>
DEFINE_TYPE(VainSabers, VainSabersMenuHost);
DEFINE_TYPE(VainSabers, VainSabersSettingsViewController);
namespace VainSabers {
namespace {
ListW<System::Object *> Choices(const std::vector<std::string> &names) {
    auto list = ListW<System::Object *>::New();
    list->EnsureCapacity(names.size());
    for (auto &name : names)
        list->Add(static_cast<System::Object *>(StringW(name).convert()));
    return list;
}
} // namespace
std::vector<std::string> GetMenuPresetNames() {
    std::vector<std::string> names;
    std::error_code ec;
    for (auto it = std::filesystem::directory_iterator(PluginConfig::GetPresetDirectory(), ec);
         !ec && it != std::filesystem::directory_iterator(); it.increment(ec)) {
        auto ext = it->path().extension().string();
        if (it->is_regular_file(ec) && (ext == ".json" || ext == ".vainsaber"))
            names.push_back(it->path().stem().string());
    }
    std::sort(names.begin(), names.end());
    names.erase(std::unique(names.begin(), names.end()), names.end());
    return names;
}
MenuEditorState *VainSabersMenuHost::State() {
    if (!_nativeStateHandle)
        _nativeStateHandle = reinterpret_cast<int64_t>(new MenuEditorState());
    return reinterpret_cast<MenuEditorState *>(_nativeStateHandle);
}
void VainSabersMenuHost::OnDestroy() {
    ClosePanel();
    delete reinterpret_cast<MenuEditorState *>(_nativeStateHandle);
    _nativeStateHandle = 0;
}
void VainSabersMenuHost::OnDisable() {
    ClosePanel();
}
void VainSabersMenuHost::ClosePanel() {
    PCUI::ClosePopup();
    UI::Clear(_editorRoot);
    UI::Clear(_floatingPanel);
    UI::Clear(_previewRoot);
    if (_nativeStateHandle) {
        auto s = State();
        bool restore = s->editing;
        s->editing = false;
        s->previewQueued = false;
        s->previewLast = -1;
        ++s->previewEpoch;
        if (restore)
            MenuSabers::Refresh();
    }
}
bool VainSabersMenuHost::get_modEnabled() {
    return GetPluginConfig().enabled;
}
void VainSabersMenuHost::set_modEnabled(bool value) {
    if (_syncingHome || value == GetPluginConfig().enabled)
        return;
    GetPluginConfig().enabled = value;
    GetPluginConfig().Save();
    MenuSabers::Refresh();
}
StringW VainSabersMenuHost::get_menuMode() {
    return GetPluginConfig().menuMode == 0 ? "Vanilla" : GetPluginConfig().menuMode == 1 ? "Saber" : "Pointer";
}
void VainSabersMenuHost::set_menuMode(StringW value) {
    auto name = static_cast<std::string>(value);
    int next = name == "Vanilla" ? 0 : name == "Saber" ? 1 : 2;
    auto &c = GetPluginConfig();
    if (_syncingHome || c.menuMode == next)
        return;
    c.menuMode = next;
    c.Save();
    MenuSabers::Refresh();
    if (MenuPresetContainer)
        MenuPresetContainer->get_gameObject()->SetActive(next == 2);
    if (root)
        UnityEngine::UI::LayoutRebuilder::ForceRebuildLayoutImmediate(
            root->GetComponent<UnityEngine::RectTransform *>());
}
StringW VainSabersMenuHost::get_SelectedPreset() {
    return StringW(GetPluginConfig().currentSaber);
}
StringW VainSabersMenuHost::get_SelectedMenuPreset() {
    return StringW(GetPluginConfig().menuSaberPreset);
}
void VainSabersMenuHost::set_SelectedPreset(StringW value) {
    if (_syncingHome)
        return;
    auto name = static_cast<std::string>(value);
    if (name == GetPluginConfig().currentSaber)
        return;
    Preset preset;
    if (!PresetLoader::LoadFromFile(PluginConfig::GetPresetFilePath(name), preset)) {
        State()->status = "Preset could not be loaded: " + name;
        VS_LOG("%s", State()->status.c_str());
        return;
    }
    GetPluginConfig().currentSaber = name;
    GetPluginConfig().Save();
    MenuSabers::Refresh();
}
void VainSabersMenuHost::set_SelectedMenuPreset(StringW value) {
    if (_syncingHome)
        return;
    auto name = static_cast<std::string>(value);
    if (name == GetPluginConfig().menuSaberPreset)
        return;
    Preset preset;
    if (!PresetLoader::LoadFromFile(PluginConfig::GetPresetFilePath(name), preset)) {
        State()->status = "Preset could not be loaded: " + name;
        VS_LOG("%s", State()->status.c_str());
        return;
    }
    GetPluginConfig().menuSaberPreset = name;
    GetPluginConfig().Save();
    MenuSabers::Refresh();
}
ListW<System::Object *> VainSabersMenuHost::get_PresetNames() {
    auto names = GetMenuPresetNames();
    if (names.empty())
        names.push_back(GetPluginConfig().currentSaber);
    return Choices(names);
}
ListW<System::Object *> VainSabersMenuHost::get_MenuPresetNames() {
    return get_PresetNames();
}
ListW<System::Object *> VainSabersMenuHost::get_menuModeChoices() {
    return Choices({"Vanilla", "Saber", "Pointer"});
}
void VainSabersMenuHost::EditSaberPreset() {
    OpenEditor(GetPluginConfig().currentSaber);
}
void VainSabersMenuHost::EditMenuPreset() {
    OpenEditor(GetPluginConfig().menuSaberPreset);
}
void VainSabersMenuHost::CreateNewPreset() {
    CreatePreset();
}
void VainSabersMenuHost::ToggleSettingsPanel() {
    if (_floatingPanel)
        ClosePanel();
    else
        OpenSettings();
}
void VainSabersMenuHost::UpdatePresetDropdown() {
    auto names = GetMenuPresetNames();
    if (names.empty())
        names.push_back(GetPluginConfig().currentSaber);
    auto &c = GetPluginConfig();
    _syncingHome = true;
    auto update = [&](BSML::DropdownListSetting *setting, const std::string &name) {
        if (!setting)
            return;
        setting->values = Choices(names);
        setting->UpdateChoices();
        auto found = std::find(names.begin(), names.end(), name);
        setting->index = found == names.end() ? 0 : static_cast<int>(found - names.begin());
        setting->dropdown->SelectCellWithIdx(setting->index);
        setting->UpdateState();
    };
    update(SaberPresetDropdown, c.currentSaber);
    update(MenuPresetDropdown, c.menuSaberPreset);
    _syncingHome = false;
    if (MenuPresetContainer)
        MenuPresetContainer->get_gameObject()->SetActive(c.menuMode == 2);
    if (EditSaberButton)
        EditSaberButton->set_interactable(!names.empty());
    if (EditMenuButton)
        EditMenuButton->set_interactable(!names.empty());
    if (root)
        UnityEngine::UI::LayoutRebuilder::ForceRebuildLayoutImmediate(
            root->GetComponent<UnityEngine::RectTransform *>());
}
void VainSabersMenuHost::ShowHome() {
    auto s = State();
    if (_homeRoot && s->homeParser) {
        UpdatePresetDropdown();
        return;
    }
    UI::Clear(_homeRoot);
    auto container = PCUI::Box(get_transform(), 0, 0, 0, 0);
    _homeRoot = container->get_gameObject();
    UI::Fill(_homeRoot, 0);
    _homeRoot->set_name("VainSabersUI PC BSML");
    _syncingHome = true;
    s->homeParser = BSML::parse_and_construct(kPcHomeMarkup, container, this);
    _syncingHome = false;
    UpdatePresetDropdown();
}
void VainSabersMenuHost::OpenSettings() {
    ClosePanel();
    auto screen = BSML::Lite::CreateFloatingScreen({110, 94}, {0, 1.2f, 2}, {0, 0, 0}, 120, false, false);
    _floatingPanel = screen->get_gameObject();
    PCUI::ConfigureFloatingPanel(_floatingPanel);
    screen->get_transform()->set_localScale({.018f, .018f, .018f});
    auto root = PCUI::Box(screen->get_transform(), 0, 0, 110, 94);
    UI::Fill(root->get_gameObject(), 0);
    auto body = PCUI::Panel(root, 0, 0, 110, 94, "Saber Settings");
    PCUI::Form form{body, 108};
    auto &c = GetPluginConfig();
    auto number = [&](const char *label, float value, float lo, float hi, float step, int digits,
                      std::function<void(float)> set) {
        form.Number(label, value, lo, hi, step, digits, [set](float v) {
            set(v);
            GetPluginConfig().Save();
        });
    };
    form.Header("Blur");
    number("Blur MS", c.blurMS, 0, 25, 1, 0, [](float v) { GetPluginConfig().blurMS = v; });
    number("Softness", c.blurSoftness, 0, 1, .01f, 2, [](float v) { GetPluginConfig().blurSoftness = v; });
    form.Header("Default Trails");
    number("Blade Trail MS", c.bladeTrailMS, 0, 200, 1, 0, [](float v) { GetPluginConfig().bladeTrailMS = v; });
    number("Tip Trail MS", c.tipTrailMS, 0, 200, 1, 0, [](float v) { GetPluginConfig().tipTrailMS = v; });
    form.Header("Quality & Position");
    number("Quality", c.saberQuality, .01f, 1.5f, .01f, 2, [](float v) {
        GetPluginConfig().saberQuality = v;
        MenuSabers::Refresh();
    });
    number("Z Rotation", c.zRotationOffset, -180, 180, 1, 0, [](float v) {
        GetPluginConfig().zRotationOffset = v;
        MenuSabers::Refresh();
    });
    form.Header("Smoothing");
    form.Toggle("Pos Smoothing", c.positionSmoothingEnabled, [this](bool v) {
        GetPluginConfig().positionSmoothingEnabled = v;
        GetPluginConfig().Save();
        OpenSettings();
    });
    if (c.positionSmoothingEnabled)
        number("Pos Smoothness", c.positionSmoothingStrength, 0, 1, .01f, 2,
               [](float v) { GetPluginConfig().positionSmoothingStrength = v; });
    form.Toggle("Rot Smoothing", c.rotationSmoothingEnabled, [this](bool v) {
        GetPluginConfig().rotationSmoothingEnabled = v;
        GetPluginConfig().Save();
        OpenSettings();
    });
    if (c.rotationSmoothingEnabled)
        number("Rot Smoothness", c.rotationSmoothingStrength, 0, 1, .01f, 2,
               [](float v) { GetPluginConfig().rotationSmoothingStrength = v; });
    // The pointer controls share the PC modes, while preserving the game's UI ray.
    form.Header("Menu Pointers");
    form.Dropdown("Pointer",
                  c.pointerMode == 0   ? "Vanilla"
                  : c.pointerMode == 1 ? "VainSabers"
                                       : "None",
                  {"Vanilla", "VainSabers", "None"}, [this](std::string v) {
                      GetPluginConfig().pointerMode = v == "Vanilla" ? 0 : v == "VainSabers" ? 1 : 2;
                      GetPluginConfig().Save();
                      MenuSabers::RefreshPointers();
                      OpenSettings();
                  });
    form.Dropdown("Laser",
                  c.laserMode == 0   ? "Vanilla"
                  : c.laserMode == 1 ? "VainSabers"
                                     : "None",
                  {"Vanilla", "VainSabers", "None"}, [this](std::string v) {
                      GetPluginConfig().laserMode = v == "Vanilla" ? 0 : v == "VainSabers" ? 1 : 2;
                      GetPluginConfig().Save();
                      MenuSabers::RefreshPointers();
                      OpenSettings();
                  });
    if (c.laserMode == 1)
        number("Laser Blur", c.menuPointerLaserBlurFactor, 0, 1, .01f, 2,
               [](float v) { GetPluginConfig().menuPointerLaserBlurFactor = v; });
    if (c.pointerMode == 1) {
        auto row = form.Row();
        auto names = GetMenuPresetNames();
        PCUI::Dropdown(PCUI::Box(row, 0, 0, 85, 4), "Dot Preset", c.menuPointerDotPreset, names, [](std::string v) {
            GetPluginConfig().menuPointerDotPreset = v;
            GetPluginConfig().Save();
            MenuSabers::RefreshPointers();
        });
        PCUI::Button(PCUI::Box(row, 87, 0, 21, 4), "Edit",
                     [this] { OpenEditor(GetPluginConfig().menuPointerDotPreset); });
    }
    PCUI::FitForm(form, 81);
    auto close = PCUI::Box(body, 0, 82, 108, 4);
    close->get_gameObject()->AddComponent<UnityEngine::UI::LayoutElement *>()->set_ignoreLayout(true);
    PCUI::Button(close, "Close", [this] { ClosePanel(); }, {.55f, .35f, .2f, 1});
}
void VainSabersSettingsViewController::DidActivate(bool, bool, bool) {
    auto h = get_gameObject()->GetComponent<VainSabersMenuHost *>();
    if (!h)
        h = get_gameObject()->AddComponent<VainSabersMenuHost *>();
    h->ShowHome();
}
void VainSabersSettingsViewController::Register() {
    BSML::Init();
    BSML::Register::RegisterGameplaySetupTab("VainSabers", [](UnityEngine::GameObject *go, bool) {
        auto h = go->GetComponent<VainSabersMenuHost *>();
        if (!h)
            h = go->AddComponent<VainSabersMenuHost *>();
        h->ShowHome();
    });
    VS_LOG("VainSabers registered only in gameplay Mods (Quest revision 7, keypad events and trail activation)");
}
} // namespace VainSabers
