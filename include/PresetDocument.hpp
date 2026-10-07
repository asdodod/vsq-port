#pragma once
#include "beatsaber-hook/shared/rapidjson/include/rapidjson/document.h"
#include "beatsaber-hook/shared/rapidjson/include/rapidjson/stringbuffer.h"
#include "beatsaber-hook/shared/rapidjson/include/rapidjson/prettywriter.h"
#include <string>
#include <string_view>
#include <algorithm>
#include <cctype>

namespace VainSabers {
// Edit the original document, so unknown PC fields survive a Quest edit/save.
struct PresetDocument {
    rapidjson::Document json;
    std::string path, name, original;
    size_t part = 0;
    int section = 0;
    bool dirty = false;

    static bool SameKey(std::string_view a, std::string_view b) {
        return a.size() == b.size() && std::equal(a.begin(), a.end(), b.begin(), [](unsigned char x, unsigned char y) {
                   return std::tolower(x) == std::tolower(y);
               });
    }
    static rapidjson::Value *Find(rapidjson::Value &object, std::string_view key) {
        if (!object.IsObject())
            return nullptr;
        for (auto it = object.MemberBegin(); it != object.MemberEnd(); ++it)
            if (SameKey({it->name.GetString(), it->name.GetStringLength()}, key))
                return &it->value;
        return nullptr;
    }
    rapidjson::Value &Ensure(rapidjson::Value &object, const char *key) {
        if (auto v = Find(object, key))
            return *v;
        rapidjson::Value k(key, json.GetAllocator());
        object.AddMember(k, rapidjson::Value(), json.GetAllocator());
        return *Find(object, key);
    }
    static void NormalizeLegacyTrails(rapidjson::Document &document) {
        auto list = Find(document, "bladeTrails"), legacy = Find(document, "bladeTrail");
        if (!legacy || !legacy->IsObject() || (list && list->IsArray() && !list->Empty()))
            return;
        rapidjson::Value trails(rapidjson::kArrayType), trail;
        trail.CopyFrom(*legacy, document.GetAllocator());
        trails.PushBack(trail, document.GetAllocator());
        if (list)
            list->Swap(trails);
        else
            document.AddMember(rapidjson::Value("bladeTrails", document.GetAllocator()), trails, document.GetAllocator());
    }
    bool Parse(std::string_view source) {
        json.Parse(source.data(), source.size());
        if (json.HasParseError() || !json.IsObject())
            return false;
        NormalizeLegacyTrails(json);
        auto parts = Find(json, "parts");
        if (!parts || !parts->IsArray())
            return false;
        for (auto &p : parts->GetArray())
            if (!p.IsObject())
                return false;
        auto version = Find(json, "version");
        int fileVersion = version && version->IsInt() ? version->GetInt() : 2;
        if (fileVersion > 2)
            return false;
        if (fileVersion < 2) {
            for (auto &p : parts->GetArray())
                if (Flag(p, "lit", false)) {
                    SetNumber(p, "startGlow", 0);
                    SetNumber(p, "endGlow", 0);
                    auto gradient = Find(p, "glowAddendGradient");
                    if (gradient && gradient->IsArray())
                        gradient->Clear();
                    if (auto rings = Find(p, "rings"); rings && rings->IsArray())
                        for (auto &ring : rings->GetArray())
                            if (ring.IsObject())
                                SetNumber(ring, "glow", 0);
                }
            // Changing only Version would flip legacy OBJ resources on the next load.
            // Keep their coordinate convention until those resources are migrated too.
            bool legacyObj = false;
            for (auto &p : parts->GetArray()) {
                auto mode = Find(p, "geometryMode");
                legacyObj |= mode && ((mode->IsInt() && mode->GetInt() == 3) ||
                                      (mode->IsString() && std::string_view(mode->GetString()) == "Obj"));
            }
            if (!legacyObj)
                SetNumber(json, "version", 2, true);
        }
        original = std::string(source);
        part = 0;
        section = 0;
        dirty = false;
        return true;
    }
    rapidjson::Value &Parts() {
        return *Find(json, "parts");
    }
    rapidjson::Value *Part() {
        return part < Parts().Size() ? &Parts()[part] : nullptr;
    }
    static float Number(rapidjson::Value &object, const char *key, float fallback) {
        auto v = Find(object, key);
        return v && v->IsNumber() ? v->GetFloat() : fallback;
    }
    static bool Flag(rapidjson::Value &object, const char *key, bool fallback) {
        auto v = Find(object, key);
        return v && v->IsBool() ? v->GetBool() : fallback;
    }
    static std::string Text(rapidjson::Value &object, const char *key, const char *fallback) {
        auto v = Find(object, key);
        return v && v->IsString() ? v->GetString() : fallback;
    }
    void SetNumber(rapidjson::Value &object, const char *key, float value, bool integer = false) {
        auto &v = Ensure(object, key);
        if (integer)
            v.SetInt(static_cast<int>(value));
        else
            v.SetFloat(value);
        dirty = true;
    }
    void SetFlag(rapidjson::Value &object, const char *key, bool value) {
        Ensure(object, key).SetBool(value);
        dirty = true;
    }
    void SetText(rapidjson::Value &object, const char *key, const std::string &value) {
        Ensure(object, key).SetString(value.data(), value.size(), json.GetAllocator());
        dirty = true;
    }
    static float Component(rapidjson::Value &object, const char *key, size_t index, float fallback) {
        auto v = Find(object, key);
        if (v && v->IsObject() && index < 3) {
            const char *axis[] = {"x", "y", "z"};
            return Number(*v, axis[index], fallback);
        }
        return v && v->IsArray() && v->Size() > index && (*v)[index].IsNumber() ? (*v)[index].GetFloat() : fallback;
    }
    void SetComponent(rapidjson::Value &object, const char *key, size_t index, float value, float fallback = 0) {
        auto &v = Ensure(object, key);
        if (v.IsObject() && index < 3) {
            const char *axis[] = {"x", "y", "z"};
            SetNumber(v, axis[index], value);
            return;
        }
        if (!v.IsArray())
            v.SetArray();
        while (v.Size() <= index)
            v.PushBack(fallback, json.GetAllocator());
        v[index].SetFloat(value);
        dirty = true;
    }
    void SetAsset(rapidjson::Value &object, const char *key, const std::string &filename) {
        // An imported preset's embedded bytes must not override a new selection.
        if (filename != "None" && filename == Text(object, key, ""))
            return;
        SetText(object, key, filename == "None" ? "" : filename);
        std::string embeddedKey = std::string_view(key) == "objFile" ? "objBase64" : std::string(key) + "Base64";
        SetText(object, embeddedKey.c_str(), "");
        if (auto alias = Find(object, std::string(key) + "Name"))
            alias->SetString(filename == "None" ? "" : filename.c_str(), json.GetAllocator());
    }
    void EnsureAdvancedRings(rapidjson::Value &p) {
        auto &rings = Ensure(p, "rings");
        if (!rings.IsArray())
            rings.SetArray();
        if (!rings.Empty())
            return;
        for (const char *prefix : {"start", "end"}) {
            std::string pre = prefix;
            rapidjson::Value ring(rapidjson::kObjectType);
            SetNumber(ring, "position", pre == "start" ? 0 : 1);
            SetNumber(ring, "radius", Number(p, (pre + "Radius").c_str(), .03f));
            for (size_t i = 0; i < 3; ++i)
                SetComponent(ring, "color", i, Component(p, (pre + "Color").c_str(), i, 1), 1);
            SetNumber(ring, "customWeight", Number(p, (pre + "CustomWeight").c_str(), 1));
            SetNumber(ring, "glow", Number(p, (pre + "Glow").c_str(), 1));
            SetNumber(ring, "opacity", Number(p, (pre + "Opacity").c_str(), 1));
            SetFlag(ring, "inverted", Flag(p, "inverted", false));
            SetNumber(ring, "offsetX", 0);
            SetNumber(ring, "offsetY", 0);
            SetNumber(ring, "uvOffset", 0);
            rings.PushBack(ring, json.GetAllocator());
        }
    }
    void AddPart(bool copy = false) {
        rapidjson::Value value(rapidjson::kObjectType);
        copy = copy && Part();
        std::string newName = copy ? Text(*Part(), "name", "Part") + " Copy"
                                   : "Part " + std::to_string(Parts().Size() + 1);
        if (copy)
            value.CopyFrom(*Part(), json.GetAllocator());
        Parts().PushBack(value, json.GetAllocator());
        part = Parts().Size() - 1;
        auto &p = *Part();
        SetText(p, "name", newName);
        if (!copy) {
            // Match BlurSaberData.AddComponent in the PC 0.0.5 release.
            SetNumber(p, "length", .1f);
            SetNumber(p, "startRadius", .03f);
            SetNumber(p, "endRadius", .03f);
            SetNumber(p, "startCustomWeight", 1);
            SetNumber(p, "endCustomWeight", 1);
            SetNumber(p, "blur", 1);
            SetNumber(p, "blurFade", 1);
            SetText(p, "geometryMode", "Simple");
        }
        dirty = true;
    }
    void RemovePart() {
        if (Part()) {
            Parts().Erase(Parts().Begin() + part);
            if (part && part >= Parts().Size())
                --part;
            dirty = true;
        }
    }
    std::string Serialize() const {
        rapidjson::StringBuffer buffer;
        rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buffer);
        json.Accept(writer);
        return {buffer.GetString(), buffer.GetSize()};
    }
};
} // namespace VainSabers
