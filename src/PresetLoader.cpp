#include "PresetLoader.hpp"
#include "PluginConfig.hpp"
#include "DefaultPresets.hpp"
#include "main.hpp"
#include "beatsaber-hook/shared/config/rapidjson-utils.hpp"
#include "PresetDocument.hpp"
#include <fstream>
#include <sstream>
#include <filesystem>
#include <algorithm>

namespace VainSabers {

static const rapidjson::Value *FindMemberCaseInsensitive(const rapidjson::Value &obj, const char *key) {
    if (!obj.IsObject())
        return nullptr;
    if (obj.HasMember(key))
        return &obj[key];
    std::string alt = key;
    if (!alt.empty()) {
        if (std::islower(static_cast<unsigned char>(alt[0])))
            alt[0] = std::toupper(static_cast<unsigned char>(alt[0]));
        else
            alt[0] = std::tolower(static_cast<unsigned char>(alt[0]));
        if (obj.HasMember(alt.c_str()))
            return &obj[alt.c_str()];
    }
    for (auto it = obj.MemberBegin(); it != obj.MemberEnd(); ++it) {
        if (strcasecmp(it->name.GetString(), key) == 0) {
            return &it->value;
        }
    }
    return nullptr;
}

static float GetFloat(const rapidjson::Value &obj, const char *key, float defVal = 0.0f) {
    const auto *v = FindMemberCaseInsensitive(obj, key);
    if (v && v->IsNumber()) {
        const float result = v->GetFloat();
        return std::isfinite(result) ? result : defVal;
    }
    return defVal;
}

static std::vector<FloatGradientKey> GetGradient(const rapidjson::Value &obj, const char *name) {
    std::vector<FloatGradientKey> keys;
    const auto *array = FindMemberCaseInsensitive(obj, name);
    if (!array || !array->IsArray())
        return keys;
    for (const auto &key : array->GetArray()) {
        if (!key.IsObject() || keys.size() >= 128)
            continue;
        keys.push_back({std::clamp(GetFloat(key, "time"), 0.0f, 1.0f), GetFloat(key, "value"),
                        static_cast<int>(GetFloat(key, "easing"))});
    }
    std::stable_sort(keys.begin(), keys.end(), [](const auto &a, const auto &b) { return a.time < b.time; });
    return keys;
}

static int GetInt(const rapidjson::Value &obj, const char *key, int defVal = 0) {
    const auto *v = FindMemberCaseInsensitive(obj, key);
    if (v && v->IsInt()) {
        return v->GetInt();
    }
    return defVal;
}

static bool GetBool(const rapidjson::Value &obj, const char *key, bool defVal = false) {
    const auto *v = FindMemberCaseInsensitive(obj, key);
    if (v && v->IsBool()) {
        return v->GetBool();
    }
    return defVal;
}

static std::string GetString(const rapidjson::Value &obj, const char *key, const std::string &defVal = "") {
    const auto *v = FindMemberCaseInsensitive(obj, key);
    if (v && v->IsString()) {
        return std::string(v->GetString(), v->GetStringLength());
    }
    return defVal;
}

static UnityEngine::Vector3 GetVector3(const rapidjson::Value &obj, const char *key,
                                       UnityEngine::Vector3 defVal = {0.0f, 0.0f, 0.0f}) {
    const auto *v = FindMemberCaseInsensitive(obj, key);
    if (v && v->IsArray() && v->Size() >= 3) {
        return UnityEngine::Vector3{(*v)[0].IsNumber() ? (*v)[0].GetFloat() : 0.0f,
                                    (*v)[1].IsNumber() ? (*v)[1].GetFloat() : 0.0f,
                                    (*v)[2].IsNumber() ? (*v)[2].GetFloat() : 0.0f};
    }
    if (v && v->IsObject())
        return {GetFloat(*v, "x", defVal.x), GetFloat(*v, "y", defVal.y), GetFloat(*v, "z", defVal.z)};
    return defVal;
}

static UnityEngine::Vector2 GetVector2(const rapidjson::Value &obj, const char *key,
                                       UnityEngine::Vector2 fallback = {1, 1}) {
    auto v = FindMemberCaseInsensitive(obj, key);
    if (v && v->IsObject())
        return {GetFloat(*v, "x", fallback.x), GetFloat(*v, "y", fallback.y)};
    if (v && v->IsArray() && v->Size() >= 2)
        return {(*v)[0].IsNumber() ? (*v)[0].GetFloat() : fallback.x,
                (*v)[1].IsNumber() ? (*v)[1].GetFloat() : fallback.y};
    return fallback;
}

static void ParseTrailResources(const rapidjson::Value &obj, SaberTrailData &trail) {
    trail.colorTexture = GetString(obj, "colorTexture", GetString(obj, "colorTextureName"));
    trail.glowTexture = GetString(obj, "glowTexture", GetString(obj, "glowTextureName"));
    trail.colorTextureBase64 = GetString(obj, "colorTextureBase64");
    trail.glowTextureBase64 = GetString(obj, "glowTextureBase64");
    trail.textureWrap = std::clamp(GetInt(obj, "textureWrap", 1), 0, 3);
    auto count = [](UnityEngine::Vector2 value) {
        return UnityEngine::Vector2{std::clamp(value.x, 1.f, 16.f), std::clamp(value.y, 1.f, 16.f)};
    };
    auto speed = [](UnityEngine::Vector3 value) {
        return UnityEngine::Vector3{std::clamp(value.x, 0.f, 120.f), value.y > .5f ? 1.f : 0.f,
                                    value.z > .5f ? 1.f : 0.f};
    };
    trail.colorAtlasCount = count(GetVector2(obj, "colorAtlasCount"));
    trail.glowAtlasCount = count(GetVector2(obj, "glowAtlasCount"));
    trail.colorAtlasSpeedFlip = speed(GetVector3(obj, "colorAtlasSpeedFlip", {1, 0, 0}));
    trail.glowAtlasSpeedFlip = speed(GetVector3(obj, "glowAtlasSpeedFlip", {1, 0, 0}));
    trail.noiseEnabled = GetBool(obj, "noiseEnabled", false);
    trail.noiseIntensity = GetFloat(obj, "noiseIntensity", .02f);
    trail.noiseScale = GetFloat(obj, "noiseScale", 2);
    trail.noiseSpeed = GetFloat(obj, "noiseSpeed", 1);
}

static UnityEngine::Color GetColor(const rapidjson::Value &obj, const char *key,
                                   UnityEngine::Color defVal = {1.0f, 1.0f, 1.0f, 1.0f}) {
    const auto *v = FindMemberCaseInsensitive(obj, key);
    if (v && v->IsArray() && v->Size() >= 3) {
        float a = (v->Size() >= 4 && (*v)[3].IsNumber()) ? (*v)[3].GetFloat() : 1.0f;
        return UnityEngine::Color{(*v)[0].IsNumber() ? (*v)[0].GetFloat() : 1.0f,
                                  (*v)[1].IsNumber() ? (*v)[1].GetFloat() : 1.0f,
                                  (*v)[2].IsNumber() ? (*v)[2].GetFloat() : 1.0f, a};
    }
    return defVal;
}

static GeometryType ParseGeometryMode(const rapidjson::Value &obj) {
    const auto *v = FindMemberCaseInsensitive(obj, "geometryMode");
    if (!v)
        return GeometryType::Simple;
    if (v->IsInt()) {
        int val = v->GetInt();
        if (val >= 0 && val <= 3)
            return static_cast<GeometryType>(val);
    } else if (v->IsString()) {
        std::string s = v->GetString();
        if (s == "Advanced")
            return GeometryType::Advanced;
        if (s == "Sprite")
            return GeometryType::Sprite;
        if (s == "Obj")
            return GeometryType::Obj;
    }
    return GeometryType::Simple;
}
static void ParseTrailGradients(const rapidjson::Value &obj, SaberTrailData &trail) {
    trail.customBlendGradient = GetGradient(obj, "customBlendGradient");
    if (auto arr = FindMemberCaseInsensitive(obj, "colorGradient"); arr && arr->IsArray())
        for (auto &k : arr->GetArray())
            if (k.IsObject())
                trail.colorGradient.push_back(
                    {std::clamp(GetFloat(k, "time"), 0.f, 1.f), GetColor(k, "color"), GetInt(k, "easing")});
    std::stable_sort(trail.colorGradient.begin(), trail.colorGradient.end(),
                     [](auto a, auto b) { return a.time < b.time; });
}

static SaberSide ParseSaberSide(const rapidjson::Value &obj) {
    const auto *v = FindMemberCaseInsensitive(obj, "side");
    if (!v)
        return SaberSide::Both;
    if (v->IsInt()) {
        int val = v->GetInt();
        if (val >= 0 && val <= 2)
            return static_cast<SaberSide>(val);
    } else if (v->IsString()) {
        std::string s = v->GetString();
        if (s == "LeftOnly" || s == "Left")
            return SaberSide::LeftOnly;
        if (s == "RightOnly" || s == "Right")
            return SaberSide::RightOnly;
    }
    return SaberSide::Both;
}

bool PresetLoader::LoadFromJsonString(std::string_view json, Preset &outPreset) {
    rapidjson::Document doc;
    if (doc.Parse(json.data(), json.size()).HasParseError()) {
        VS_LOG("ERROR: Failed to parse preset JSON (error offset: %zu)", doc.GetErrorOffset());
        return false;
    }

    if (!doc.IsObject()) {
        VS_LOG("ERROR: Preset JSON root is not an object");
        return false;
    }

    PresetDocument::NormalizeLegacyTrails(doc);
    outPreset.version = GetInt(doc, "version", 1);
    if (outPreset.version > 2) {
        VS_LOG("Preset version %d is newer than supported version 2", outPreset.version);
        return false;
    }
    outPreset.useCustomTrails = GetBool(doc, "useCustomTrails", false);
    outPreset.parts.clear();

    const auto *partsVal = FindMemberCaseInsensitive(doc, "parts");
    if (partsVal && partsVal->IsArray()) {
        const auto &partsArr = *partsVal;
        for (rapidjson::SizeType i = 0; i < partsArr.Size(); i++) {
            const auto &pObj = partsArr[i];
            if (!pObj.IsObject())
                continue;

            PartData part;
            part.name = GetString(pObj, "name", "Part_" + std::to_string(i));
            part.position = GetVector3(pObj, "position", UnityEngine::Vector3{0.0f, 0.0f, 0.0f});
            part.rotation = GetVector3(pObj, "rotation", UnityEngine::Vector3{0.0f, 0.0f, 0.0f});
            part.length = GetFloat(pObj, "length", 1.0f);
            part.geometryMode = ParseGeometryMode(pObj);
            part.presetVersion = outPreset.version;
            part.linkedPartIndex = GetInt(pObj, "linkedPartIndex", -1);
            part.spriteSizeX = GetFloat(pObj, "spriteSizeX", .2f);
            part.spriteSizeY = GetFloat(pObj, "spriteSizeY", .2f);
            part.spriteDivisionsX = std::clamp(GetInt(pObj, "spriteDivisionsX", 1), 1, 20);
            part.spriteDivisionsY = std::clamp(GetInt(pObj, "spriteDivisionsY", 1), 1, 20);
            part.doubleSided = GetBool(pObj, "doubleSided");
            part.objScale = GetFloat(pObj, "objScale", 1);
            part.objFile = GetString(pObj, "objFile");
            part.objBase64 = GetString(pObj, "objBase64");
            part.colorTexture = GetString(pObj, "colorTexture");
            part.glowTexture = GetString(pObj, "glowTexture");
            part.colorTextureBase64 = GetString(pObj, "colorTextureBase64");
            part.glowTextureBase64 = GetString(pObj, "glowTextureBase64");
            part.textureWrap = std::clamp(GetInt(pObj, "textureWrap", 0), 0, 3);
            SaberTrailData atlas;
            ParseTrailResources(pObj, atlas);
            part.colorAtlasCount = atlas.colorAtlasCount;
            part.glowAtlasCount = atlas.glowAtlasCount;
            part.colorAtlasSpeedFlip = atlas.colorAtlasSpeedFlip;
            part.glowAtlasSpeedFlip = atlas.glowAtlasSpeedFlip;
            part.lookDir = GetVector3(pObj, "lookDir");
            part.useLookDir = GetBool(pObj, "useLookDir");
            part.disableGlowPass = GetBool(pObj, "disableGlowPass");
            if (auto list = FindMemberCaseInsensitive(pObj, "animators"); list && list->IsArray())
                for (auto &a : list->GetArray()) {
                    if (!a.IsObject())
                        continue;
                    PartData::Animator anim;
                    anim.type = GetString(a, "$type", GetString(a, "type"));
                    auto end = anim.type.find(',');
                    if (end != std::string::npos)
                        anim.type.resize(end);
                    auto ns = anim.type.find_last_of('.');
                    if (ns != std::string::npos)
                        anim.type = anim.type.substr(ns + 1);
                    anim.speed = GetFloat(a, "speed", anim.type == "RotationAdder" ? 30 : .5f);
                    anim.amplitude = GetFloat(a, "amplitude", .5f);
                    anim.frequency = GetFloat(a, "frequency", .5f);
                    anim.axis = std::clamp(GetInt(a, "axis", 0), 0, 2);
                    auto axis = GetString(a, "axis");
                    if (!axis.empty())
                        anim.axis = axis == "Y" ? 1 : axis == "Z" ? 2 : 0;
                    part.animators.push_back(anim);
                }

            part.hueShift = GetFloat(pObj, "hueShift", 0.0f);

            part.startRadius = GetFloat(pObj, "startRadius", 0.015f);
            part.startColor = GetColor(pObj, "startColor", UnityEngine::Color{1.0f, 1.0f, 1.0f, 1.0f});
            part.startCustomWeight = GetFloat(pObj, "startCustomWeight", 1.0f);
            part.startGlow = GetFloat(pObj, "startGlow", 1.0f);
            part.startOpacity = GetFloat(pObj, "startOpacity", 1.0f);

            part.endRadius = GetFloat(pObj, "endRadius", 0.015f);
            part.endColor = GetColor(pObj, "endColor", UnityEngine::Color{1.0f, 1.0f, 1.0f, 1.0f});
            part.endCustomWeight = GetFloat(pObj, "endCustomWeight", 1.0f);
            part.endGlow = GetFloat(pObj, "endGlow", 1.0f);
            part.endOpacity = GetFloat(pObj, "endOpacity", 1.0f);

            part.inverted = GetBool(pObj, "inverted", false);
            part.lit = GetBool(pObj, "lit", false);
            part.blur = GetFloat(pObj, "blur", 1.0f);
            part.blurFade = GetFloat(pObj, "blurFade", 1.0f);
            part.enableEndCaps = GetBool(pObj, "enableEndCaps", true);
            part.enableRoundedNormals = GetBool(pObj, "enableRoundedNormals", true);
            part.endCapExtension = GetFloat(pObj, "endCapExtension", 0.25f);

            part.bulgeAmount = GetFloat(pObj, "bulgeAmount", 0.0f);
            part.minimumRings = GetInt(pObj, "minimumRings", 4);
            part.renderQueueOffset = GetInt(pObj, "renderQueueOffset", 0);
            part.depthOffset = GetFloat(pObj, "depthOffset", 0.0f);
            part.disableDepthPrepass = GetBool(pObj, "disableDepthPrepass");
            part.rimFactor = GetFloat(pObj, "rimFactor");
            part.rimPower = GetFloat(pObj, "rimPower", 3);
            part.rimPerpendicular = GetFloat(pObj, "rimPerpendicular");
            part.rimGradient = GetGradient(pObj, "rimPowerGradient");
            part.glowGradient = GetGradient(pObj, "glowAddendGradient");
            part.opacityGradient = GetGradient(pObj, "opacityMultiplierGradient");
            part.specularStrength = GetFloat(pObj, "specularStrength", .41f);
            part.specularPower = GetFloat(pObj, "specularPower", 48);
            part.metallic = GetFloat(pObj, "metallic");
            part.smoothness = GetFloat(pObj, "smoothness");
            part.cubemapStrength = GetFloat(pObj, "cubemapStrength", .78f);
            part.cubemapRotation = GetFloat(pObj, "cubemapRotation");
            part.fresnelStrength = GetFloat(pObj, "fresnelStrength", .6f);
            part.fresnelPower = GetFloat(pObj, "fresnelPower", 2.89f);
            part.rimColor = GetColor(pObj, "rimColor", part.rimColor);
            part.fresnelCustomBlend = GetFloat(pObj, "fresnelCustomBlend");

            part.side = ParseSaberSide(pObj);
            part.mirrorOnLeft = GetBool(pObj, "mirrorOnLeft", false);
            part.manualRingVerts = GetBool(pObj, "manualRingVerts", false);
            part.ringVertsManual = GetInt(pObj, "ringVertsManual", 20);

            // Parse optional rings array for Advanced geometry mode
            const auto *ringsVal = FindMemberCaseInsensitive(pObj, "rings");
            if (ringsVal && ringsVal->IsArray()) {
                const auto &ringsArr = *ringsVal;
                for (rapidjson::SizeType r = 0; r < ringsArr.Size(); r++) {
                    const auto &rObj = ringsArr[r];
                    if (!rObj.IsObject())
                        continue;

                    RingData ring;
                    ring.position = GetFloat(rObj, "position", 0.0f);
                    ring.radius = GetFloat(rObj, "radius", 0.015f);
                    ring.color = GetColor(rObj, "color", UnityEngine::Color{1.0f, 1.0f, 1.0f, 1.0f});
                    ring.customWeight = GetFloat(rObj, "customWeight", 1.0f);
                    ring.glow = GetFloat(rObj, "glow", 1.0f);
                    ring.opacity = GetFloat(rObj, "opacity", 1.0f);
                    ring.inverted = GetBool(rObj, "inverted", false);
                    ring.offsetX = GetFloat(rObj, "offsetX", 0.0f);
                    ring.offsetY = GetFloat(rObj, "offsetY", 0.0f);
                    ring.uvOffset = GetFloat(rObj, "uvOffset", 0.0f);

                    part.rings.push_back(ring);
                }
            }

            // The PC release migrates the legacy lit glow representation.
            if (outPreset.version < 2 && part.lit) {
                part.startGlow = part.endGlow = 0;
                part.glowGradient.clear();
                for (auto &ring : part.rings)
                    ring.glow = 0;
            }
            outPreset.parts.push_back(part);
        }
    }

    // Resolve linked geometry/material while preserving each instance's transform.
    auto originalParts = outPreset.parts;
    for (size_t i = 0; i < originalParts.size(); ++i) {
        size_t source = i;
        std::vector<bool> seen(originalParts.size());
        while (originalParts[source].linkedPartIndex >= 0 &&
               originalParts[source].linkedPartIndex < originalParts.size()) {
            seen[source] = true;
            size_t next = originalParts[source].linkedPartIndex;
            if (seen[next]) {
                source = i;
                break;
            }
            source = next;
        }
        if (source == i)
            continue;
        auto own = originalParts[i];
        outPreset.parts[i] = originalParts[source];
        auto &target = outPreset.parts[i];
        target.name = own.name;
        target.position = own.position;
        target.rotation = own.rotation;
        target.mirrorOnLeft = own.mirrorOnLeft;
        target.side = own.side;
        target.linkedPartIndex = own.linkedPartIndex;
    }
    outPreset.tipTrails.clear();
    const auto *tipTrailsVal = FindMemberCaseInsensitive(doc, "tipTrails");
    if (tipTrailsVal && tipTrailsVal->IsArray()) {
        for (rapidjson::SizeType i = 0; i < tipTrailsVal->Size(); i++) {
            const auto &tObj = (*tipTrailsVal)[i];
            if (!tObj.IsObject())
                continue;
            SaberTrailData td;
            td.position = GetVector3(tObj, "position", UnityEngine::Vector3{0.0f, 0.0f, 1.0f});
            td.color = GetColor(tObj, "color", UnityEngine::Color{1.0f, 1.0f, 1.0f, 1.0f});
            td.customBlend = GetFloat(tObj, "customBlend", 1.0f);
            td.glow = GetFloat(tObj, "glow", 1.0f);
            td.opacity = GetFloat(tObj, "opacity", 1.0f);
            td.width = GetFloat(tObj, "width", 0.008f);
            td.length = GetInt(tObj, "length", 60);
            td.queueOffset = GetInt(tObj, "queueOffset", 0);
            td.depthOffset = GetFloat(tObj, "depthOffset", 0.0f);
            td.fade = GetFloat(tObj, "fade", 1.0f);
            td.motionActivation = GetFloat(tObj, "motionActivation", 1.0f);
            td.motionFadePower = GetFloat(tObj, "motionFadePower", 0.0f);
            ParseTrailGradients(tObj, td);
            ParseTrailResources(tObj, td);
            outPreset.tipTrails.push_back(td);
        }
    }

    outPreset.bladeTrails.clear();
    const auto *bladeTrailsVal = FindMemberCaseInsensitive(doc, "bladeTrails");
    if (bladeTrailsVal && bladeTrailsVal->IsArray()) {
        for (rapidjson::SizeType i = 0; i < bladeTrailsVal->Size(); i++) {
            const auto &tObj = (*bladeTrailsVal)[i];
            if (!tObj.IsObject())
                continue;
            SaberTrailData td;
            td.position = GetVector3(tObj, "position", UnityEngine::Vector3{0.0f, 0.0f, 1.0f});
            td.color = GetColor(tObj, "color", UnityEngine::Color{1.0f, 1.0f, 1.0f, 1.0f});
            td.customBlend = GetFloat(tObj, "customBlend", 1.0f);
            td.glow = GetFloat(tObj, "glow", 1.0f);
            td.opacity = GetFloat(tObj, "opacity", 0.3f);
            td.width = GetFloat(tObj, "width", 0.01f);
            td.length = GetInt(tObj, "length", 60);
            td.queueOffset = GetInt(tObj, "queueOffset", 0);
            td.depthOffset = GetFloat(tObj, "depthOffset", 0.0f);
            td.fade = GetFloat(tObj, "fade", 1.0f);
            td.motionActivation = GetFloat(tObj, "motionActivation", 1.0f);
            td.motionFadePower = GetFloat(tObj, "motionFadePower", 0.0f);
            ParseTrailGradients(tObj, td);
            ParseTrailResources(tObj, td);
            outPreset.bladeTrails.push_back(td);
        }
    }

    VS_LOG("Loaded preset (version %d) with %zu parts, %zu tipTrails, %zu bladeTrails (useCustomTrails=%d)",
           outPreset.version, outPreset.parts.size(), outPreset.tipTrails.size(), outPreset.bladeTrails.size(),
           outPreset.useCustomTrails ? 1 : 0);
    return true;
}

bool PresetLoader::LoadFromFile(const std::string &filePath, Preset &outPreset) {
    std::error_code ec;
    if (!std::filesystem::exists(filePath, ec)) {
        return false;
    }

    std::ifstream file(filePath);
    if (!file.is_open()) {
        VS_LOG("ERROR: Could not open preset file: %s", filePath.c_str());
        return false;
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string content = buffer.str();
    file.close();

    VS_LOG("Reading preset from file: %s (%zu bytes)", filePath.c_str(), content.size());
    return LoadFromJsonString(content, outPreset);
}

bool PresetLoader::LoadByName(const std::string &presetName, Preset &outPreset) {
    // 1. Try file from preset directory
    std::string filePath = PluginConfig::GetPresetFilePath(presetName);
    if (LoadFromFile(filePath, outPreset)) {
        return true;
    }

    // 2. Try embedded preset
    std::string_view embeddedJson = GetEmbeddedPresetJson(presetName);
    if (!embeddedJson.empty()) {
        VS_LOG("Loading embedded preset '%s'...", presetName.c_str());
        return LoadFromJsonString(embeddedJson, outPreset);
    }

    // 3. Fallback to embedded default
    VS_LOG("Preset '%s' not found, falling back to embedded 'default'...", presetName.c_str());
    std::string_view defaultJson = GetEmbeddedPresetJson("default");
    return LoadFromJsonString(defaultJson, outPreset);
}

} // namespace VainSabers
