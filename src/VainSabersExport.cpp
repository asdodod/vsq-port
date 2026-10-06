#include "VainSabersUI.hpp"
#include "PresetExport.hpp"
#include "PresetLoader.hpp"
#include "PluginConfig.hpp"
#include "BlurSaberPart.hpp"
#include "VainSabersAssets.hpp"
#include "UnityEngine/Camera.hpp"
#include "UnityEngine/CameraClearFlags.hpp"
#include "UnityEngine/StereoTargetEyeMask.hpp"
#include "UnityEngine/RenderTexture.hpp"
#include "UnityEngine/RenderTextureFormat.hpp"
#include "UnityEngine/TextureFormat.hpp"
#include "UnityEngine/ImageConversion.hpp"
#include "UnityEngine/Rect.hpp"

namespace VainSabers {
namespace {
// Dedicated camera and temporary sabers: no menus or headset screenshots in exports.
void Thumbnail(const Preset &preset, const std::filesystem::path &path) {
    using namespace UnityEngine;
    if (!Assets::IsLoaded() && !Assets::LoadAssets())
        throw std::runtime_error("Saber assets could not be loaded");
    auto root = GameObject::New_ctor("VainSabers export preview");
    root->set_layer(30);
    root->get_transform()->set_position({1000, 1000, 1000});
    auto cameraGo = GameObject::New_ctor("VainSabers export camera");
    auto cam = cameraGo->AddComponent<Camera *>();
    cam->set_enabled(false);
    auto target = RenderTexture::New_ctor(512, 512, 24, RenderTextureFormat::ARGB32);
    auto tex = Texture2D::New_ctor(512, 512, TextureFormat::RGB24, false);
    auto previous = RenderTexture::get_active();
    auto cleanup = [&] {
        RenderTexture::set_active(previous);
        cam->set_targetTexture(nullptr);
        target->Release();
        root->SetActive(false);
        Object::Destroy(root);
        Object::Destroy(cameraGo);
        Object::Destroy(target);
        Object::Destroy(tex);
    };
    try {
        float extent = 1;
        for (auto &part : preset.parts)
            extent = std::max(extent, std::abs(part.length) + std::abs(part.position.z) + std::abs(part.position.x) +
                                          std::abs(part.position.y));
        for (int side = 0; side < 2; ++side) {
            auto anchor = GameObject::New_ctor("Preview saber");
            anchor->set_layer(30);
            anchor->get_transform()->SetParent(root->get_transform(), false);
            anchor->get_transform()->set_localPosition({(side == 0 ? -.17f : .17f) * extent, 0, 0});
            auto tracker = anchor->AddComponent<MovementTracker *>();
            tracker->Init(anchor->get_transform());
            for (const auto &data : preset.parts) {
                if (data.side == SaberSide::LeftOnly && side != 0)
                    continue;
                if (data.side == SaberSide::RightOnly && side != 1)
                    continue;
                auto partGo = GameObject::New_ctor(StringW(data.name));
                partGo->set_layer(30);
                partGo->get_transform()->SetParent(anchor->get_transform(), false);
                auto part = partGo->AddComponent<BlurSaberPart *>();
                part->Init(tracker);
                part->ApplyPartData(data, side == 0);
                part->SetColor(side == 0 ? Color{.8f, .12f, .15f, 1} : Color{.15f, .55f, .9f, 1});
                part->ApplyMaterialProps();
            }
        }
        cam->set_stereoTargetEye(StereoTargetEyeMask::None);
        cam->set_cullingMask(1 << 30);
        cam->set_orthographic(true);
        cam->set_orthographicSize(extent * .8f);
        cam->set_nearClipPlane(.01f);
        cam->set_farClipPlane(20 * extent);
        cam->set_clearFlags(CameraClearFlags::SolidColor);
        cam->set_backgroundColor({.022f, .028f, .045f, 1});
        cam->get_transform()->set_position({1000, 1000 - 3 * extent, 1000 + extent * .35f});
        cam->get_transform()->LookAt(Vector3{1000, 1000, 1000 + extent * .35f}, Vector3{0, 0, 1});
        cam->set_targetTexture(target);
        cam->Render();
        RenderTexture::set_active(target);
        tex->ReadPixels(Rect{0, 0, 512, 512}, 0, 0);
        tex->Apply();
        auto bytes = ImageConversion::EncodeToPNG(tex);
        if (!bytes || bytes.size() == 0)
            throw std::runtime_error("PNG encoding failed");
        WritePresetFile(path, reinterpret_cast<const char *>(bytes.begin()), bytes.size());
    } catch (...) {
        cleanup();
        throw;
    }
    cleanup();
}
std::string ExportOne(std::string name, std::string source) {
    if (!ValidPresetName(name))
        throw std::runtime_error("Invalid preset filename");
    PresetDocument d;
    if (!d.Parse(source))
        throw std::runtime_error(name + ": unsupported preset JSON");
    EmbedPresetAssets(d, d.json, PluginConfig::GetPresetDirectory());
    const std::filesystem::path dir = PluginConfig::GetPresetDirectory();
    std::error_code ec;
    std::filesystem::create_directories(dir, ec);
    if (ec)
        throw std::runtime_error("Cannot access /sdcard/VainSabers: " + ec.message());
    auto content = d.Serialize();
    WritePresetFile(dir / (name + ".vainsaber"), content.data(), content.size());
    Preset preset;
    if (!PresetLoader::LoadFromJsonString(content, preset))
        throw std::runtime_error("Exported preset cannot be parsed");
    try {
        Thumbnail(preset, dir / (name + ".png"));
    } catch (const std::exception &e) {
        return "Exported " + name + ".vainsaber; PNG failed: " + e.what();
    }
    return "Exported " + name + ".vainsaber (+ PNG) to /sdcard/VainSabers";
}
} // namespace
void VainSabersMenuHost::ExportPreset() {
    auto s = State();
    if (!s->editing)
        return;
    try {
        s->status = ExportOne(s->saveAs, s->document.Serialize());
    } catch (const std::exception &e) {
        s->status = "Export failed: " + std::string(e.what());
        VS_LOG("Preset export failed: %s", e.what());
    }
    BuildEditor();
}
} // namespace VainSabers
