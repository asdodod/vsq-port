#include "VainSabersUI.hpp"
#include "PluginConfig.hpp"
#include "PresetLoader.hpp"
#include "DefaultPresets.hpp"
#include "PresetFilePolicy.hpp"
#include "MenuSabers.hpp"
#include "BlurSaber.hpp"
#include "UnityEngine/Time.hpp"
#include "bsml/shared/BSML/MainThreadScheduler.hpp"
#include <filesystem>
#include <fstream>
#include <sstream>

namespace VainSabers {
namespace {
std::string Read(const std::string &path) {
    std::ifstream f(path);
    return {std::istreambuf_iterator<char>(f), {}};
}
bool ValidName(const std::string &n) {
    return !n.empty() && n.size() <= 64 && n != "." && n != ".." &&
           n.find_first_of("/\\:*?\"<>|\r\n") == std::string::npos && n.back() != '.' && n.back() != ' ';
}
} // namespace
void VainSabersMenuHost::OpenEditor(const std::string &name) {
    ClosePanel();
    auto s = State();
    auto path = PluginConfig::GetPresetFilePath(name);
    if (IsVainSaberExport(path.c_str())) {
        s->status = "Cannot edit a read-only .vainsaber export.";
        VS_LOG("Cannot open editor: preset '%s' is a read-only .vainsaber export", name.c_str());
        ShowHome();
        return;
    }
    auto source = Read(path);
    if (source.empty())
        source = std::string(GetEmbeddedPresetJson(name));
    if (!s->document.Parse(source)) {
        s->status = "Cannot edit this preset: invalid or unsupported JSON.";
        ShowHome();
        return;
    }
    s->document.path = path;
    s->document.name = name;
    s->saveAs = name;
    s->status.clear();
    s->exportConfirmation.clear();
    s->editing = true;
    s->preview = true;
    s->holdSabers = true;
    s->deleteConfirm = s->revertConfirm = 0;
    BuildEditor();
    Preview();
}
void VainSabersMenuHost::CreatePreset() {
    auto s = State();
    const std::string source = "{\"version\":2,\"parts\":[]}";
    const auto dir = PluginConfig::GetPresetDirectory();
    std::error_code ec;
    std::filesystem::create_directories(dir, ec);
    auto fail = [&](const std::string &reason) {
        s->status = "Cannot create preset: " + reason;
        VS_LOG("%s", s->status.c_str());
    };
    if (ec) {
        fail(ec.message());
        return;
    }
    const std::string base = "NewPreset";
    std::string name, path;
    for (int i = 0;; ++i) {
        name = base + (i ? std::to_string(i) : "");
        path = dir + "/" + name + ".json";
        bool taken = false;
        for (auto extension : {".json", ".vainsaber", ".txt"}) {
            taken |= std::filesystem::exists(dir + "/" + name + extension, ec);
            if (ec) {
                fail(ec.message());
                return;
            }
        }
        if (!taken)
            break;
    }
    // Match PC: create an empty file immediately, select it, then wait for Edit.
    {
        std::ofstream file(path, std::ios::binary | std::ios::trunc);
        file.write(source.data(), source.size());
        file.close();
        if (!file) {
            std::filesystem::remove(path, ec);
            fail("write failed in /sdcard/VainSabers");
            return;
        }
    }
    ClosePanel();
    GetPluginConfig().currentSaber = name;
    GetPluginConfig().Save();
    s->status = "Created: " + name;
    ShowHome();
    MenuSabers::Refresh();
}
void VainSabersMenuHost::Preview() {
    auto s = State();
    if (!s->editing)
        return;
    // Updating a number is immediate; expensive saber reconstruction is
    // coalesced while dragging. The queued update reads the latest document.
    float now = UnityEngine::Time::get_unscaledTime(), delay = .05f - (now - s->previewLast);
    if (delay > 0) {
        if (!s->previewQueued) {
            s->previewQueued = true;
            UnityW<VainSabersMenuHost> host = this;
            int epoch = s->previewEpoch;
            BSML::MainThreadScheduler::ScheduleAfterTime(delay, [host, epoch]() mutable {
                if (!host)
                    return;
                auto state = host->State();
                if (state->previewEpoch != epoch)
                    return;
                state->previewQueued = false;
                host->Preview();
            });
        }
        return;
    }
    s->previewLast = now;
    if (!s->preview) {
        MenuSabers::Refresh();
        return;
    }
    Preset preset;
    if (!PresetLoader::LoadFromJsonString(s->document.Serialize(), preset))
        return;
    if (s->holdSabers) {
        UI::Clear(_previewRoot);
        MenuSabers::Refresh(&preset);
        return;
    }
    if (!_previewRoot) {
        MenuSabers::Refresh();
        MenuSabers::HidePreviewSabers(true);
        _previewRoot = UnityEngine::GameObject::New_ctor("VainSabers Static Preview");
        _previewRoot->set_layer(5);
        _previewRoot->get_transform()->set_position({0, .75f, 1.8f});
        _previewRoot->get_transform()->set_eulerAngles({-90, 0, 0});
        for (int side = 0; side < 2; ++side) {
            auto anchor = UnityEngine::GameObject::New_ctor(side == 0 ? "Left" : "Right");
            anchor->set_layer(5);
            anchor->get_transform()->SetParent(_previewRoot->get_transform(), false);
            anchor->get_transform()->set_localPosition({side == 0 ? -.12f : .12f, 0, 0});
            auto saber = anchor->AddComponent<BlurSaber *>();
            saber->Init(anchor->get_transform(), MenuSabers::GetColor(side == 0), side == 0);
            saber->_followMenuColors = true;
        }
    }
    for (int i = 0; i < _previewRoot->get_transform()->get_childCount(); ++i) {
        auto saber = _previewRoot->get_transform()->GetChild(i)->GetComponent<BlurSaber *>();
        if (saber)
            saber->ApplyPreset(preset);
    }
}
void VainSabersMenuHost::SavePreset() {
    auto s = State();
    auto &d = s->document;
    if (!s->editing || IsVainSaberExport(d.path.c_str()))
        return;
    if (!ValidName(s->saveAs)) {
        s->status = "Invalid name (1-64 characters; no path separators).";
        BuildEditor();
        return;
    }
    std::error_code ec;
    auto path = PluginConfig::GetPresetDirectory() + "/" + s->saveAs + ".json";
    if (s->saveAs == d.name && std::filesystem::path(d.path).extension() == ".json")
        path = d.path;
    if (path != d.path && std::filesystem::exists(path, ec)) {
        s->status = "Name already exists. Choose a different name.";
        BuildEditor();
        return;
    }
    auto serialized = d.Serialize();
    Preset checked;
    if (!PresetLoader::LoadFromJsonString(serialized, checked)) {
        s->status = "Preset validation failed; nothing saved.";
        BuildEditor();
        return;
    }
    if (std::filesystem::exists(path, ec)) {
        std::filesystem::copy_file(path, path + ".bak", std::filesystem::copy_options::overwrite_existing, ec);
        if (ec) {
            s->status = "Cannot create backup: " + ec.message();
            BuildEditor();
            return;
        }
    }
    auto temp = path + ".tmp";
    {
        std::ofstream file(temp, std::ios::binary | std::ios::trunc);
        file.write(serialized.data(), serialized.size());
        file.flush();
        if (!file) {
            s->status = "Write failed; original preset is unchanged.";
            BuildEditor();
            return;
        }
    }
    std::filesystem::rename(temp, path, ec);
    if (ec) {
        s->status = "Save failed: " + ec.message();
        BuildEditor();
        return;
    }
    auto oldName = d.name;
    d.path = path;
    d.name = s->saveAs;
    d.original = serialized;
    d.dirty = false;
    auto &c = GetPluginConfig();
    if (c.currentSaber == oldName)
        c.currentSaber = d.name;
    if (c.menuSaberPreset == oldName)
        c.menuSaberPreset = d.name;
    c.Save();
    s->status = "Saved: " + d.name;
    ClosePanel();
    ShowHome();
}

} // namespace VainSabers
