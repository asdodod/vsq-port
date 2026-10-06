#include "VainSabersAssets.hpp"
#include "UnityEngine/HideFlags.hpp"
#include <filesystem>
#include <fstream>
#include <vector>
#include <algorithm>

// Forward declare embedded bytes from vs_assets_data.cpp
extern const size_t vs_assets_bytes_len;
extern const uint8_t vs_assets_bytes[];

namespace VainSabers {

static UnityW<UnityEngine::AssetBundle> s_bundle = nullptr;

UnityW<UnityEngine::Shader> Assets::TestShader = nullptr;
UnityW<UnityEngine::Shader> Assets::SaberShader = nullptr;
UnityW<UnityEngine::Shader> Assets::VertexGlowShaderUntextured = nullptr;
UnityW<UnityEngine::Shader> Assets::VertexGlowShader = nullptr;
UnityW<UnityEngine::Shader> Assets::BlurPartShader = nullptr;

UnityW<UnityEngine::Material> Assets::NormalSaberMaterial = nullptr;
UnityW<UnityEngine::Material> Assets::InvertedSaberMaterial = nullptr;
UnityW<UnityEngine::Material> Assets::NormalLitSaberMaterial = nullptr;
UnityW<UnityEngine::Material> Assets::InvertedLitSaberMaterial = nullptr;

UnityW<UnityEngine::GameObject> Assets::BlurSaberPrefab = nullptr;

static bool s_loaded = false;

bool Assets::IsLoaded() {
    return s_loaded;
}

// A previous extracted bundle must never shadow shaders shipped in an update.
// Use a content-specific filename so extraction is safe even after an interrupted write.
static std::string GetBundlePath() {
    uint64_t hash = 14695981039346656037ULL;
    for (size_t i = 0; i < vs_assets_bytes_len; ++i) {
        hash = (hash ^ vs_assets_bytes[i]) * 1099511628211ULL;
    }
    const std::string filename = "vs_assets-" + std::to_string(hash);
    const std::vector<std::string> directories = {"/sdcard/ModData/com.beatgames.beatsaber/Mods/VainSabers",
                                                  "/data/user/0/com.beatgames.beatsaber/files/vainsabers"};
    for (const auto &dir : directories) {
        std::error_code ec;
        std::filesystem::create_directories(dir, ec);
        if (ec)
            continue;
        const auto path = dir + "/" + filename;
        // Check bytes, not just length: partial or stale data must not be loaded.
        std::ifstream existing(path, std::ios::binary | std::ios::ate);
        if (existing && existing.tellg() == static_cast<std::streamoff>(vs_assets_bytes_len)) {
            std::vector<uint8_t> bytes(vs_assets_bytes_len);
            existing.seekg(0);
            existing.read(reinterpret_cast<char *>(bytes.data()), bytes.size());
            if (existing && std::equal(bytes.begin(), bytes.end(), vs_assets_bytes)) {
                VS_LOG("Using verified embedded bundle: %s (%zu bytes)", path.c_str(), vs_assets_bytes_len);
                return path;
            }
        }
        existing.close();
        const auto tempPath = path + ".tmp";
        std::ofstream out(tempPath, std::ios::binary | std::ios::trunc);
        if (!out)
            continue;
        out.write(reinterpret_cast<const char *>(vs_assets_bytes), vs_assets_bytes_len);
        out.close();
        if (!out)
            continue;
        std::filesystem::rename(tempPath, path, ec);
        if (!ec) {
            VS_LOG("Extracted current embedded bundle: %s (%zu bytes)", path.c_str(), vs_assets_bytes_len);
            return path;
        }
    }
    VS_LOG("ERROR: Unable to extract current embedded bundle to writable storage");
    return "";
}
bool Assets::LoadAssets() {
    if (s_loaded)
        return true;

    std::string bundlePath = GetBundlePath();
    if (bundlePath.empty()) {
        VS_LOG("ERROR: Could not locate or extract vs_assets bundle!");
        return false;
    }

    VS_LOG("Loading vs_assets bundle from %s...", bundlePath.c_str());
    auto bundle = UnityEngine::AssetBundle::LoadFromFile(StringW(bundlePath));
    if (!bundle) {
        VS_LOG("ERROR: AssetBundle::LoadFromFile returned null for path: %s", bundlePath.c_str());
        return false;
    }

    auto assetNames = bundle->GetAllAssetNames();
    int count = assetNames.size();
    VS_LOG("=== vs_assets loaded successfully! Asset count: %d ===", count);
    for (int i = 0; i < count; i++) {
        std::string name = static_cast<std::string>(assetNames[i]);
        VS_LOG("  [%d] %s", i, name.c_str());
    }

    // Load shaders
    TestShader = bundle->LoadAsset<UnityW<UnityEngine::Shader>>(StringW("vs_test"));
    SaberShader = bundle->LoadAsset<UnityW<UnityEngine::Shader>>(StringW("vs_saber"));
    VertexGlowShaderUntextured = bundle->LoadAsset<UnityW<UnityEngine::Shader>>(StringW("vs_flatglow"));
    VertexGlowShader = bundle->LoadAsset<UnityW<UnityEngine::Shader>>(StringW("vs_flatglow_2side"));
    BlurPartShader = bundle->LoadAsset<UnityW<UnityEngine::Shader>>(StringW("vs_blurpart"));

    VS_LOG("Loaded shaders: Test=%p, Saber=%p, FlatGlow=%p, FlatGlow2Side=%p, BlurPart=%p", (void *)TestShader.ptr(),
           (void *)SaberShader.ptr(), (void *)VertexGlowShaderUntextured.ptr(), (void *)VertexGlowShader.ptr(),
           (void *)BlurPartShader.ptr());

    // Load materials
    NormalSaberMaterial = bundle->LoadAsset<UnityW<UnityEngine::Material>>(StringW("saber"));
    InvertedSaberMaterial = bundle->LoadAsset<UnityW<UnityEngine::Material>>(StringW("saberinverted"));
    NormalLitSaberMaterial = bundle->LoadAsset<UnityW<UnityEngine::Material>>(StringW("saberlit"));
    InvertedLitSaberMaterial = bundle->LoadAsset<UnityW<UnityEngine::Material>>(StringW("saberlitinverted"));

    VS_LOG("Loaded materials: Saber=%p, Inverted=%p, Lit=%p, LitInverted=%p", (void *)NormalSaberMaterial.ptr(),
           (void *)InvertedSaberMaterial.ptr(), (void *)NormalLitSaberMaterial.ptr(),
           (void *)InvertedLitSaberMaterial.ptr());

    auto validateMaterial = [](UnityW<UnityEngine::Material> &mat, const char *name,
                               UnityW<UnityEngine::Shader> &fallback) {
        if (!mat) {
            VS_LOG("Material %s is null!", name);
            return;
        }
        auto shader = mat->get_shader();
        bool supported = shader && shader->get_isSupported();
        VS_LOG("Material %s shader: %p, isSupported: %d", name, (void *)shader.ptr(), supported ? 1 : 0);
        if (!supported && fallback && fallback->get_isSupported()) {
            VS_LOG("Setting fallback shader for material %s", name);
            mat->set_shader(fallback);
        }
    };

    validateMaterial(NormalSaberMaterial, "NormalSaberMaterial", BlurPartShader);
    validateMaterial(InvertedSaberMaterial, "InvertedSaberMaterial", BlurPartShader);
    validateMaterial(NormalLitSaberMaterial, "NormalLitSaberMaterial", BlurPartShader);
    validateMaterial(InvertedLitSaberMaterial, "InvertedLitSaberMaterial", BlurPartShader);

    s_bundle = bundle;

    // Load prefab
    BlurSaberPrefab = bundle->LoadAsset<UnityW<UnityEngine::GameObject>>(StringW("BlurSaberPrefab"));
    VS_LOG("Loaded prefab: BlurSaberPrefab=%p", (void *)BlurSaberPrefab.ptr());

    auto protectAsset = [](UnityEngine::Object *obj) {
        if (obj) {
            obj->set_hideFlags(
                UnityEngine::HideFlags(static_cast<int32_t>(UnityEngine::HideFlags::DontUnloadUnusedAsset) |
                                       static_cast<int32_t>(UnityEngine::HideFlags::HideAndDontSave)));
        }
    };

    protectAsset(s_bundle);
    protectAsset(TestShader);
    protectAsset(SaberShader);
    protectAsset(VertexGlowShaderUntextured);
    protectAsset(VertexGlowShader);
    protectAsset(BlurPartShader);
    protectAsset(NormalSaberMaterial);
    protectAsset(InvertedSaberMaterial);
    protectAsset(NormalLitSaberMaterial);
    protectAsset(InvertedLitSaberMaterial);
    protectAsset(BlurSaberPrefab);

    if (!BlurPartShader || !BlurPartShader->get_isSupported() || !NormalSaberMaterial || !InvertedSaberMaterial ||
        !NormalLitSaberMaterial || !InvertedLitSaberMaterial) {
        VS_LOG("ERROR: Required blur shader/materials are unavailable; keeping vanilla sabers");
        s_bundle->Unload(true);
        s_bundle = nullptr;
        return false;
    }
    s_loaded = true;
    return true;
}

} // namespace VainSabers
