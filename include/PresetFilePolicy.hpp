#pragma once
namespace VainSabers {
// PC exports are playable, but the editor only accepts source presets.
constexpr bool IsVainSaberExport(const char *path) {
    unsigned length = 0;
    while (path[length])
        ++length;
    constexpr char suffix[] = ".vainsaber";
    constexpr unsigned suffixLength = sizeof(suffix) - 1;
    if (length < suffixLength)
        return false;
    for (unsigned i = 0; i < suffixLength; ++i) {
        char value = path[length - suffixLength + i];
        if (value >= 'A' && value <= 'Z')
            value += 'a' - 'A';
        if (value != suffix[i])
            return false;
    }
    return true;
}
} // namespace VainSabers
