#pragma once

#include <string_view>
#include <vector>

namespace VainSabers {

struct EmbeddedPreset {
    std::string_view name;
    std::string_view json;
};

const std::vector<EmbeddedPreset> &GetEmbeddedPresets();
std::string_view GetEmbeddedPresetJson(std::string_view name);

} // namespace VainSabers
