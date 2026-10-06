#pragma once

#include <string>

namespace VainSabers {

struct PluginConfig {
    bool enabled = true;
    std::string currentSaber = "default";
    int menuMode = 1; // PC: Vanilla / same gameplay saber / separate menu preset
    std::string menuSaberPreset = "default";
    int pointerMode = 0, laserMode = 0;
    float menuPointerLaserBlurFactor = 1;
    std::string menuPointerDotPreset = "menupointer-dot";
    int blurMS = 16;
    float blurSoftness = 0.8f;
    int tipTrailMS = 140;
    int bladeTrailMS = 60;
    float saberQuality = 1.0f;
    float zRotationOffset = 0.0f;
    bool hideVanillaSaber = true;
    bool positionSmoothingEnabled = false;
    float positionSmoothingStrength = 0.5f;
    bool rotationSmoothingEnabled = false;
    float rotationSmoothingStrength = 0.5f;

    void Load();
    void Save();

    static std::string GetPresetDirectory();
    static void EnsureDefaultPresetsExist();
    static std::string GetPresetFilePath(const std::string &presetName);
};

PluginConfig &GetPluginConfig();

} // namespace VainSabers
