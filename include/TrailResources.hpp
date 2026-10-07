#pragma once
#include "PresetData.hpp"
#include "UnityEngine/Material.hpp"
#include "UnityEngine/Texture2D.hpp"
namespace VainSabers {
UnityEngine::Texture2D *LoadPresetTexture(const std::string &name, const std::string &base64, int wrap);
void ApplyTrailResources(UnityEngine::Material *material, const SaberTrailData &data,
                         UnityW<UnityEngine::Texture2D> &color, UnityW<UnityEngine::Texture2D> &glow,
                         bool gpuNoise = true);
} // namespace VainSabers
