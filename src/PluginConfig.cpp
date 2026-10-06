#include "PluginConfig.hpp"
#include "DefaultPresets.hpp"
#include "main.hpp"
#include <filesystem>
#include <fstream>
#include <algorithm>
#include <cmath>

namespace VainSabers {

static PluginConfig s_config;

PluginConfig &GetPluginConfig() {
    return s_config;
}

std::string PluginConfig::GetPresetDirectory() {
    const std::string presetDir = "/sdcard/VainSabers";
    static bool migrated = false;
    if (migrated)
        return presetDir;
    std::error_code ec;
    std::filesystem::create_directories(presetDir, ec);
    if (ec) {
        VS_LOG("Cannot access public preset folder %s: %s", presetDir.c_str(), ec.message().c_str());
        return presetDir;
    }
    const std::vector<std::string> oldDirs = {
        "/sdcard/ModData/com.beatgames.beatsaber/Mods/VainSabers/Presets",
        "/sdcard/Android/data/com.beatgames.beatsaber/files/mods/VainSabers/Presets",
        "/data/data/com.beatgames.beatsaber/files/mods/VainSabers/Presets"};

    const auto marker = std::filesystem::path(presetDir) / ".legacy-presets-copied";
    if (!std::filesystem::exists(marker, ec)) {
        bool complete = true;
        for (const auto &old : oldDirs) {
            ec.clear();
            if (!std::filesystem::exists(old, ec))
                continue;
            for (auto it = std::filesystem::recursive_directory_iterator(
                     old, std::filesystem::directory_options::skip_permission_denied, ec);
                 !ec && it != std::filesystem::recursive_directory_iterator(); it.increment(ec)) {
                if (it->is_symlink())
                    continue;
                const auto target = std::filesystem::path(presetDir) / it->path().lexically_relative(old);
                std::error_code copyError;
                if (it->is_directory())
                    std::filesystem::create_directories(target, copyError);
                else if (it->is_regular_file())
                    std::filesystem::copy_file(it->path(), target, std::filesystem::copy_options::skip_existing,
                                               copyError);
                if (copyError) {
                    complete = false;
                    VS_LOG("Preset migration: %s", copyError.message().c_str());
                }
            }
            if (ec)
                complete = false;
        }
        if (complete) {
            std::ofstream mark(marker);
            mark << "Presets copied to /sdcard/VainSabers; originals retained.\n";
        }
    }
    migrated = true;
    VS_LOG("Using shared preset/import/export folder: %s", presetDir.c_str());
    return presetDir;
}

std::string PluginConfig::GetPresetFilePath(const std::string &presetName) {
    std::string dir = GetPresetDirectory();
    std::string path = dir + "/" + presetName + ".json";
    std::error_code ec;
    if (std::filesystem::exists(path, ec))
        return path;

    // Check .txt fallback
    std::string txtPath = dir + "/" + presetName + ".txt";
    if (std::filesystem::exists(txtPath, ec))
        return txtPath;

    // Check .vainsaber fallback
    std::string vsPath = dir + "/" + presetName + ".vainsaber";
    if (std::filesystem::exists(vsPath, ec))
        return vsPath;

    return path;
}

void PluginConfig::EnsureDefaultPresetsExist() {
    std::string dir = GetPresetDirectory();
    std::error_code ec;
    std::filesystem::create_directories(dir, ec);

    const auto &embedded = GetEmbeddedPresets();
    for (const auto &ep : embedded) {
        std::string targetPath = dir + "/" + std::string(ep.name) + ".json";
        if (!std::filesystem::exists(GetPresetFilePath(std::string(ep.name)), ec)) {
            std::ofstream out(targetPath);
            if (out.is_open()) {
                out << ep.json;
                out.close();
                VS_LOG("Extracted default preset '%s' to %s", ep.name.data(), targetPath.c_str());
            }
        }
    }
}

void PluginConfig::Load() {
    auto &doc = getConfig().config;
    if (!doc.IsObject()) {
        VS_LOG("Config doc is not an object, saving defaults...");
        Save();
        return;
    }

    if (doc.HasMember("enabled") && doc["enabled"].IsBool()) {
        enabled = doc["enabled"].GetBool();
    }
    if (doc.HasMember("currentSaber") && doc["currentSaber"].IsString()) {
        currentSaber = doc["currentSaber"].GetString();
    }
    if (doc.HasMember("blurMS") && doc["blurMS"].IsInt()) {
        blurMS = std::clamp(doc["blurMS"].GetInt(), 0, 25);
    }
    if (doc.HasMember("menuMode") && doc["menuMode"].IsInt())
        menuMode = std::clamp(doc["menuMode"].GetInt(), 0, 2);
    if (doc.HasMember("menuSaberPreset") && doc["menuSaberPreset"].IsString())
        menuSaberPreset = doc["menuSaberPreset"].GetString();
    if (doc.HasMember("pointerMode") && doc["pointerMode"].IsInt())
        pointerMode = std::clamp(doc["pointerMode"].GetInt(), 0, 2);
    if (doc.HasMember("laserMode") && doc["laserMode"].IsInt())
        laserMode = std::clamp(doc["laserMode"].GetInt(), 0, 2);
    if (doc.HasMember("menuPointerLaserBlurFactor") && doc["menuPointerLaserBlurFactor"].IsNumber())
        menuPointerLaserBlurFactor = doc["menuPointerLaserBlurFactor"].GetFloat();
    if (doc.HasMember("menuPointerDotPreset") && doc["menuPointerDotPreset"].IsString())
        menuPointerDotPreset = doc["menuPointerDotPreset"].GetString();
    if (doc.HasMember("blurSoftness") && doc["blurSoftness"].IsNumber()) {
        blurSoftness = doc["blurSoftness"].GetFloat();
    }
    if (doc.HasMember("tipTrailMS") && doc["tipTrailMS"].IsInt()) {
        tipTrailMS = doc["tipTrailMS"].GetInt();
    }
    if (doc.HasMember("bladeTrailMS") && doc["bladeTrailMS"].IsInt()) {
        bladeTrailMS = doc["bladeTrailMS"].GetInt();
    }
    if (doc.HasMember("saberQuality") && doc["saberQuality"].IsNumber()) {
        saberQuality = doc["saberQuality"].GetFloat();
    }
    if (doc.HasMember("zRotationOffset") && doc["zRotationOffset"].IsNumber()) {
        zRotationOffset = doc["zRotationOffset"].GetFloat();
    }
    if (doc.HasMember("hideVanillaSaber") && doc["hideVanillaSaber"].IsBool()) {
        hideVanillaSaber = doc["hideVanillaSaber"].GetBool();
    }
    if (doc.HasMember("positionSmoothingEnabled") && doc["positionSmoothingEnabled"].IsBool()) {
        positionSmoothingEnabled = doc["positionSmoothingEnabled"].GetBool();
    }
    if (doc.HasMember("positionSmoothingStrength") && doc["positionSmoothingStrength"].IsNumber()) {
        positionSmoothingStrength = doc["positionSmoothingStrength"].GetFloat();
    }
    if (doc.HasMember("rotationSmoothingEnabled") && doc["rotationSmoothingEnabled"].IsBool()) {
        rotationSmoothingEnabled = doc["rotationSmoothingEnabled"].GetBool();
    }
    if (doc.HasMember("rotationSmoothingStrength") && doc["rotationSmoothingStrength"].IsNumber()) {
        rotationSmoothingStrength = doc["rotationSmoothingStrength"].GetFloat();
    }

    auto finiteClamp = [](float value, float minimum, float maximum, float fallback) {
        return std::isfinite(value) ? std::clamp(value, minimum, maximum) : fallback;
    };
    blurSoftness = finiteClamp(blurSoftness, 0, 1, .8f);
    tipTrailMS = std::clamp(tipTrailMS, 0, 500);
    bladeTrailMS = std::clamp(bladeTrailMS, 0, 500);
    saberQuality = finiteClamp(saberQuality, .01f, 1.5f, 1);
    menuPointerLaserBlurFactor = finiteClamp(menuPointerLaserBlurFactor, 0, 1, 1);
    zRotationOffset = finiteClamp(zRotationOffset, -180, 180, 0);
    positionSmoothingStrength = finiteClamp(positionSmoothingStrength, 0, 1, .5f);
    rotationSmoothingStrength = finiteClamp(rotationSmoothingStrength, 0, 1, .5f);

    VS_LOG("Loaded PluginConfig: enabled=%d, currentSaber=%s, blurMS=%d, blurSoftness=%.2f, hideVanilla=%d",
           enabled ? 1 : 0, currentSaber.c_str(), blurMS, blurSoftness, hideVanillaSaber ? 1 : 0);
}

void PluginConfig::Save() {
    auto &doc = getConfig().config;
    if (!doc.IsObject()) {
        doc.SetObject();
    }
    auto &alloc = doc.GetAllocator();

    auto setVal = [&](const char *name, auto val) {
        rapidjson::Value key(name, alloc);
        if (doc.HasMember(key)) {
            doc[key] = val;
        } else {
            doc.AddMember(key, val, alloc);
        }
    };

    setVal("enabled", enabled);

    rapidjson::Value saberVal;
    saberVal.SetString(currentSaber.c_str(), static_cast<rapidjson::SizeType>(currentSaber.length()), alloc);
    rapidjson::Value saberKey("currentSaber", alloc);
    if (doc.HasMember(saberKey)) {
        doc[saberKey] = saberVal;
    } else {
        doc.AddMember(saberKey, saberVal, alloc);
    }

    setVal("blurMS", blurMS);
    setVal("menuMode", menuMode);
    setVal("pointerMode", pointerMode);
    setVal("laserMode", laserMode);
    setVal("menuPointerLaserBlurFactor", menuPointerLaserBlurFactor);
    rapidjson::Value dotName;
    dotName.SetString(menuPointerDotPreset.c_str(), menuPointerDotPreset.size(), alloc);
    if (doc.HasMember("menuPointerDotPreset"))
        doc["menuPointerDotPreset"] = dotName;
    else
        doc.AddMember("menuPointerDotPreset", dotName, alloc);
    rapidjson::Value menuName;
    menuName.SetString(menuSaberPreset.c_str(), menuSaberPreset.size(), alloc);
    if (doc.HasMember("menuSaberPreset"))
        doc["menuSaberPreset"] = menuName;
    else
        doc.AddMember("menuSaberPreset", menuName, alloc);
    setVal("blurSoftness", blurSoftness);
    setVal("tipTrailMS", tipTrailMS);
    setVal("bladeTrailMS", bladeTrailMS);
    setVal("saberQuality", saberQuality);
    setVal("zRotationOffset", zRotationOffset);
    setVal("hideVanillaSaber", hideVanillaSaber);
    setVal("positionSmoothingEnabled", positionSmoothingEnabled);
    setVal("positionSmoothingStrength", positionSmoothingStrength);
    setVal("rotationSmoothingEnabled", rotationSmoothingEnabled);
    setVal("rotationSmoothingStrength", rotationSmoothingStrength);

    getConfig().Write();
    VS_LOG("PluginConfig saved successfully!");
}

} // namespace VainSabers
