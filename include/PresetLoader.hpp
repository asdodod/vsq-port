#pragma once

#include "PresetData.hpp"
#include <string>
#include <string_view>

namespace VainSabers {

class PresetLoader {
  public:
    static bool LoadFromJsonString(std::string_view json, Preset &outPreset);
    static bool LoadFromFile(const std::string &filePath, Preset &outPreset);
    static bool LoadByName(const std::string &presetName, Preset &outPreset);
};

} // namespace VainSabers
