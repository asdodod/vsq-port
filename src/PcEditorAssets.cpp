#include "PcUI.hpp"
#include "PluginConfig.hpp"
#include "TrailResources.hpp"
#include "UnityEngine/Sprite.hpp"
#include "UnityEngine/SpriteMeshType.hpp"
#include "UnityEngine/UI/Image.hpp"
#include <filesystem>
#include <cctype>

namespace VainSabers::PCUI {
namespace {
std::vector<std::string> AssetNames(bool texture, const std::string &selected) {
    std::vector<std::string> names;
    std::error_code ec;
    for (auto it = std::filesystem::directory_iterator(PluginConfig::GetPresetDirectory(), ec);
         !ec && it != std::filesystem::directory_iterator(); it.increment(ec)) {
        if (!it->is_regular_file(ec))
            continue;
        auto ext = it->path().extension().string();
        std::transform(ext.begin(), ext.end(), ext.begin(), [](unsigned char c) { return std::tolower(c); });
        if (texture ? (ext == ".png" || ext == ".jpg" || ext == ".jpeg") : ext == ".obj")
            names.push_back(it->path().filename().string());
    }
    if (!selected.empty() && std::find(names.begin(), names.end(), selected) == names.end())
        names.push_back(selected);
    std::sort(names.begin(), names.end());
    names.insert(names.begin(), "None");
    return names;
}
struct TexturePreview {
    UnityW<UnityEngine::Texture2D> texture;
    UnityW<UnityEngine::Sprite> sprite;
    UnityW<UnityEngine::GameObject> grid;
    void Clear() {
        if (sprite)
            UnityEngine::Object::Destroy(sprite);
        if (texture)
            UnityEngine::Object::Destroy(texture);
        sprite = nullptr;
        texture = nullptr;
    }
};
} // namespace

void AssetDropdown(Transform *parent, const std::string &label, rapidjson::Value &object, const char *key,
                   PresetDocument &doc, bool texture, std::function<void()> changed) {
    std::string field = key;
    auto selected = PresetDocument::Text(object, key, "");
    auto names = AssetNames(texture, selected);
    Dropdown(parent, label, selected.empty() ? "None" : selected, names,
             [value = &object, d = &doc, field, changed](std::string name) {
                 d->SetAsset(*value, field.c_str(), name);
                 changed();
             });
}

void TextureField(Transform *parent, const std::string &label, rapidjson::Value &object, const char *prefix,
                  PresetDocument &doc, std::function<void()> changed) {
    const std::string pre = prefix, key = pre + "Texture";
    const float width = parent->GetComponent<UnityEngine::RectTransform *>()->get_sizeDelta().x;
    auto ptr = &object;
    auto d = &doc;
    AssetDropdown(Box(parent, 0, 0, width - 5, 4), label, object, key.c_str(), doc, true, changed);
    Button(Box(parent, width - 4, 0, 4, 4), "...", [parent, ptr, d, pre, key, changed, width] {
        auto root = Popup(parent, width * .5f - 20, -1, 40, 65);
        Form f{Box(root, 1, 1, 38, 63), 38};
        auto preview = std::make_shared<TexturePreview>();
        SetPopupCleanup([preview] { preview->Clear(); });
        auto selected = PresetDocument::Text(*ptr, key.c_str(), "");
        auto names = AssetNames(true, selected);
        auto imageRoot = Box(root, 1, 7, 38, 24);
        Solid(imageRoot, {.15f, .15f, .15f, 1});
        UnityW<UnityEngine::UI::Image> image = BSML::Lite::CreateImage(imageRoot, nullptr);
        UI::Fill(image->get_gameObject(), 0);
        image->set_raycastTarget(false);
        auto updateImage = [ptr, key, preview, image]() mutable {
            if (!image)
                return;
            preview->Clear();
            preview->texture = LoadPresetTexture(PresetDocument::Text(*ptr, key.c_str(), ""),
                PresetDocument::Text(*ptr, (key + "Base64").c_str(), ""), 1);
            if (preview->texture) {
                float w = preview->texture->get_width(), h = preview->texture->get_height();
                preview->sprite = UnityEngine::Sprite::Create(preview->texture, {0, 0, w, h}, {.5f, .5f}, 100);
                image->set_sprite(preview->sprite);
                image->set_color({1, 1, 1, 1});
            } else {
                image->set_sprite(nullptr);
                image->set_color({.12f, .12f, .12f, 1});
            }
        };
        f.Dropdown("", selected.empty() ? "None" : selected, names, [ptr, d, key, changed, updateImage](std::string name) mutable {
            d->SetAsset(*ptr, key.c_str(), name);
            changed();
            updateImage();
        });
        f.Space(25);
        std::string countKey = pre + "AtlasCount", speedKey = pre + "AtlasSpeedFlip";
        auto updateGrid = [ptr, countKey, preview, imageRoot] {
            UI::Clear(preview->grid);
            auto grid = Box(imageRoot, 0, 0, 38, 24);
            preview->grid = grid->get_gameObject();
            int nx = std::clamp(static_cast<int>(PresetDocument::Component(*ptr, countKey.c_str(), 0, 1)), 1, 16);
            int ny = std::clamp(static_cast<int>(PresetDocument::Component(*ptr, countKey.c_str(), 1, 1)), 1, 16);
            for (int x = 1; x < nx; ++x)
                Solid(Box(grid, 38.f * x / nx, 0, .12f, 24), {1, 1, 1, .4f});
            for (int y = 1; y < ny; ++y)
                Solid(Box(grid, 0, 24.f * y / ny, 38, .12f), {1, 1, 1, .4f});
        };
        for (size_t i = 0; i < 2; ++i)
            f.Number(i ? "Count Y" : "Count X", PresetDocument::Component(*ptr, countKey.c_str(), i, 1), 1, 16, 1, 0,
                [ptr, d, countKey, i, changed, updateGrid](float v) {
                    d->SetComponent(*ptr, countKey.c_str(), i, v, 1);
                    changed();
                    updateGrid();
                });
        f.Number("Speed (fps)", PresetDocument::Component(*ptr, speedKey.c_str(), 0, 1), 0, 120, 1, 0,
            [ptr, d, speedKey, changed](float v) { d->SetComponent(*ptr, speedKey.c_str(), 0, v, 1); changed(); },
            {1, 1, 1, 1}, 5);
        for (size_t i = 1; i < 3; ++i)
            f.Toggle(i == 1 ? "Reverse X" : "Reverse Y", PresetDocument::Component(*ptr, speedKey.c_str(), i, 0) > .5f,
                [ptr, d, speedKey, i, changed](bool v) { d->SetComponent(*ptr, speedKey.c_str(), i, v ? 1 : 0); changed(); });
        updateImage();
        updateGrid();
    }, {.15f, .15f, .15f, 1})->set_fontSize(2.5f);
}
} // namespace VainSabers::PCUI
