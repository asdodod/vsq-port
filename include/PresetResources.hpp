#pragma once
#include "PluginConfig.hpp"
#include <filesystem>
#include <fstream>
#include <string>
namespace VainSabers {
inline std::string DecodePresetAsset(const std::string &name, const std::string &embedded) {
    if (!embedded.empty()) {
        if (embedded.size() > 24 * 1024 * 1024)
            return {};
        std::string bytes;
        bytes.reserve(embedded.size() * 3 / 4);
        unsigned bits = 0;
        int count = 0;
        const std::string alphabet = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
        for (unsigned char c : embedded) {
            if (c == '=')
                break;
            if (c == '\r' || c == '\n' || c == ' ' || c == '\t')
                continue;
            auto index = alphabet.find(c);
            if (index == std::string::npos)
                return {};
            bits = (bits << 6) | static_cast<unsigned>(index);
            count += 6;
            if (count >= 8) {
                count -= 8;
                bytes.push_back(static_cast<char>((bits >> count) & 255));
            }
        }
        return bytes;
    }
    if (name.empty())
        return {};
    auto rel = std::filesystem::path(name).lexically_normal();
    if (rel.is_absolute() || rel.has_root_name())
        return {};
    for (const auto &p : rel)
        if (p == "..")
            return {};
    auto path = std::filesystem::path(PluginConfig::GetPresetDirectory()) / rel;
    std::error_code ec;
    auto size = std::filesystem::file_size(path, ec);
    if (ec || size > 16 * 1024 * 1024)
        return {};
    std::ifstream file(path, std::ios::binary);
    std::string bytes{std::istreambuf_iterator<char>(file), {}};
    return bytes.size() == size ? bytes : std::string{};
}
} // namespace VainSabers
