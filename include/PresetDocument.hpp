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
    bool Parse(std::string_view source) {
        json.Parse(source.data(), source.size());
        if (json.HasParseError() || !json.IsObject())
            return false;
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
        return v && v->IsArray() && v->Size() > index && (*v)[index].IsNumber() ? (*v)[index].GetFloat() : fallback;
    }
    void SetComponent(rapidjson::Value &object, const char *key, size_t index, float value, float fallback = 0) {
        auto &v = Ensure(object, key);
        if (!v.IsArray())
            v.SetArray();
        while (v.Size() <= index)
            v.PushBack(fallback, json.GetAllocator());
        v[index].SetFloat(value);
        dirty = true;
    }
    void AddPart(bool copy = false) {
        rapidjson::Value value(rapidjson::kObjectType);
        if (copy && Part())
            value.CopyFrom(*Part(), json.GetAllocator());
        Parts().PushBack(value, json.GetAllocator());
        part = Parts().Size() - 1;
        auto &p = *Part();
        SetText(p, "name", copy ? "Part Copy" : "Part " + std::to_string(part + 1));
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
