#pragma once

#include "main.hpp"
#include "UnityEngine/AssetBundle.hpp"
#include "UnityEngine/Shader.hpp"
#include "UnityEngine/Material.hpp"
#include "UnityEngine/GameObject.hpp"
#include <string>

namespace VainSabers {

class Assets {
  public:
    static UnityW<UnityEngine::Shader> TestShader;
    static UnityW<UnityEngine::Shader> SaberShader;
    static UnityW<UnityEngine::Shader> VertexGlowShaderUntextured;
    static UnityW<UnityEngine::Shader> VertexGlowShader;
    static UnityW<UnityEngine::Shader> BlurPartShader;

    static UnityW<UnityEngine::Material> NormalSaberMaterial;
    static UnityW<UnityEngine::Material> InvertedSaberMaterial;
    static UnityW<UnityEngine::Material> NormalLitSaberMaterial;
    static UnityW<UnityEngine::Material> InvertedLitSaberMaterial;

    static UnityW<UnityEngine::GameObject> BlurSaberPrefab;

    static bool LoadAssets();
    static bool IsLoaded();
};

} // namespace VainSabers
