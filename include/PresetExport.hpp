#pragma once
#include "PresetDocument.hpp"
#include <filesystem>
#include <fstream>
#include <stdexcept>
namespace VainSabers {
inline bool ValidPresetName(const std::string &n) {
    return !n.empty() && n.size() <= 64 && n != "." && n != ".." &&
           n.find_first_of("/\\:*?\"<>|\r\n") == std::string::npos && n.back() != '.' && n.back() != ' ';
}
inline std::string Base64(const std::string &bytes) {
    static constexpr char table[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    std::string out;
    out.reserve((bytes.size() + 2) / 3 * 4);
    for (size_t i = 0; i < bytes.size(); i += 3) {
        unsigned a = static_cast<unsigned char>(bytes[i]);
        unsigned b = i + 1 < bytes.size() ? static_cast<unsigned char>(bytes[i + 1]) : 0;
        unsigned c = i + 2 < bytes.size() ? static_cast<unsigned char>(bytes[i + 2]) : 0;
        out += table[a >> 2];
        out += table[((a & 3) << 4) | (b >> 4)];
        out += i + 1 < bytes.size() ? table[((b & 15) << 2) | (c >> 6)] : '=';
        out += i + 2 < bytes.size() ? table[c & 63] : '=';
    }
    return out;
}
inline void EmbedPresetAssets(PresetDocument &d, rapidjson::Value &object, const std::filesystem::path &directory) {
    if (object.IsArray()) {
        for (auto &v : object.GetArray())
            EmbedPresetAssets(d, v, directory);
        return;
    }
    if (!object.IsObject())
        return;
    for (const auto &pair : {std::pair{"colorTexture", "colorTextureBase64"},
                             std::pair{"glowTexture", "glowTextureBase64"}, std::pair{"objFile", "objBase64"}}) {
        auto name = PresetDocument::Text(object, pair.first, "");
        if (name.empty() || !PresetDocument::Text(object, pair.second, "").empty())
            continue;
        auto rel = std::filesystem::path(name).lexically_normal();
        if (rel.is_absolute() || rel.has_root_name())
            throw std::runtime_error("Asset path must be relative: " + name);
        for (const auto &part : rel)
            if (part == "..")
                throw std::runtime_error("Asset path escapes preset folder: " + name);
        auto path = directory / rel;
        std::error_code ec;
        auto size = std::filesystem::file_size(path, ec);
        if (ec || size > 16 * 1024 * 1024)
            throw std::runtime_error("Missing or oversized asset: " + name);
        std::ifstream file(path, std::ios::binary);
        std::string bytes{std::istreambuf_iterator<char>(file), {}};
        if (bytes.size() != size)
            throw std::runtime_error("Cannot read asset: " + name);
        d.SetText(object, pair.second, Base64(bytes));
    }
    for (auto it = object.MemberBegin(); it != object.MemberEnd(); ++it)
        if (it->value.IsObject() || it->value.IsArray())
            EmbedPresetAssets(d, it->value, directory);
}
inline void WritePresetFile(const std::filesystem::path &path, const char *data, size_t count) {
    auto temp = path;
    temp += ".tmp";
    {
        std::ofstream f(temp, std::ios::binary | std::ios::trunc);
        f.write(data, count);
        f.flush();
        if (!f)
            throw std::runtime_error("Cannot write " + path.filename().string());
    }
    std::error_code ec;
    std::filesystem::rename(temp, path, ec);
    if (ec)
        throw std::runtime_error("Cannot finish " + path.filename().string() + ": " + ec.message());
}
} // namespace VainSabers
