#include "TrailResources.hpp"
#include "PresetResources.hpp"
#include "LegacyNoiseRandom.hpp"
#include "UnityEngine/ImageConversion.hpp"
#include "UnityEngine/Texture3D.hpp"
#include "UnityEngine/Color32.hpp"
#include "UnityEngine/TextureFormat.hpp"
#include "UnityEngine/TextureWrapMode.hpp"
#include "UnityEngine/FilterMode.hpp"
#include "UnityEngine/HideFlags.hpp"
#include "UnityEngine/Object.hpp"
#include "UnityEngine/Shader.hpp"
#include "UnityEngine/Vector4.hpp"
#include "main.hpp"

namespace VainSabers {
UnityEngine::Texture2D *LoadPresetTexture(const std::string &name, const std::string &base64, int wrap) {
    auto bytes = DecodePresetAsset(name, base64);
    if (bytes.empty()) {
        if (!name.empty() || !base64.empty())
            VS_LOG("Preset texture could not be read: %s", name.c_str());
        return nullptr;
    }
    ArrayW<uint8_t> data(bytes.size());
    std::copy(bytes.begin(), bytes.end(), data.begin());
    auto texture = UnityEngine::Texture2D::New_ctor(2, 2, UnityEngine::TextureFormat::RGBA32, true);
    if (!UnityEngine::ImageConversion::LoadImage(texture, data, false)) {
        UnityEngine::Object::Destroy(texture);
        VS_LOG("Preset image decode failed: %s", name.c_str());
        return nullptr;
    }
    texture->set_wrapMode(UnityEngine::TextureWrapMode(wrap));
    texture->set_filterMode(UnityEngine::FilterMode::Trilinear);
    return texture;
}

static UnityEngine::Texture3D *NoiseTexture() {
    static UnityW<UnityEngine::Texture3D> texture;
    if (texture)
        return texture;
    texture = UnityEngine::Texture3D::New_ctor(32, 32, 32, UnityEngine::TextureFormat::RGBA32, false);
    texture->set_name("VainSabers Trail Noise");
    texture->set_hideFlags(UnityEngine::HideFlags::HideAndDontSave);
    texture->set_wrapMode(UnityEngine::TextureWrapMode::Repeat);
    texture->set_filterMode(UnityEngine::FilterMode::Bilinear);
    // Quest metadata exposes SetPixels32, not the PC SetPixels(Color[]) overload.
    // RGBA32 keeps the same seeded noise with one quarter the upload buffer size.
    ArrayW<UnityEngine::Color32> pixels(32 * 32 * 32);
    LegacyNoiseRandom random;
    auto channel = [&] { return static_cast<uint8_t>(static_cast<float>(random.NextDouble()) * 255.f + .5f); };
    for (auto &pixel : pixels) {
        pixel.r = channel();
        pixel.g = channel();
        pixel.b = channel();
        pixel.a = 255;
    }
    texture->SetPixels32(pixels);
    texture->Apply(false, true);
    return texture;
}

void ApplyTrailResources(UnityEngine::Material *material, const SaberTrailData &data,
                         UnityW<UnityEngine::Texture2D> &color, UnityW<UnityEngine::Texture2D> &glow,
                         bool gpuNoise) {
    if (!material)
        return;
    if (color)
        UnityEngine::Object::Destroy(color);
    if (glow)
        UnityEngine::Object::Destroy(glow);
    color = LoadPresetTexture(data.colorTexture, data.colorTextureBase64, data.textureWrap);
    glow = LoadPresetTexture(data.glowTexture, data.glowTextureBase64, data.textureWrap);
    auto id = [](const char *name) { return UnityEngine::Shader::PropertyToID(name); };
    material->SetTexture(id("_ColorTex"),
                         color ? color.unsafePtr() : UnityEngine::Texture2D::get_whiteTexture().unsafePtr());
    material->SetTexture(id("_GlowTex"),
                         glow ? glow.unsafePtr() : UnityEngine::Texture2D::get_whiteTexture().unsafePtr());
    material->SetFloat(id("_ColorTexEnabled"), color ? 1 : 0);
    material->SetFloat(id("_GlowTexEnabled"), glow ? 1 : 0);
    material->SetVector(id("_ColorTexAtlasCount"), {data.colorAtlasCount.x, data.colorAtlasCount.y, 0, 0});
    material->SetVector(id("_GlowTexAtlasCount"), {data.glowAtlasCount.x, data.glowAtlasCount.y, 0, 0});
    material->SetVector(id("_ColorTexAtlasSpeedFlip"),
                        {data.colorAtlasSpeedFlip.x, data.colorAtlasSpeedFlip.y, data.colorAtlasSpeedFlip.z, 0});
    material->SetVector(id("_GlowTexAtlasSpeedFlip"),
                        {data.glowAtlasSpeedFlip.x, data.glowAtlasSpeedFlip.y, data.glowAtlasSpeedFlip.z, 0});
    material->SetFloat(id("_NoiseIntensity"), gpuNoise && data.noiseEnabled ? data.noiseIntensity : 0);
    material->SetFloat(id("_NoiseScale"), data.noiseScale);
    material->SetFloat(id("_NoiseSpeed"), data.noiseSpeed);
    material->SetFloat(id("_TrailDuration"), data.length * .001f);
    if (gpuNoise && data.noiseEnabled)
        material->SetTexture(id("_NoiseTex"), NoiseTexture());
    VS_LOG("Trail assets: color=%s (%d), glow=%s (%d), noise=%d intensity=%.3f scale=%.3f speed=%.3f",
           data.colorTexture.c_str(), color ? 1 : 0, data.glowTexture.c_str(), glow ? 1 : 0, data.noiseEnabled,
           data.noiseIntensity, data.noiseScale, data.noiseSpeed);
}
} // namespace VainSabers
