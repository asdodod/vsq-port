#pragma once
#include "VainSabersUI.hpp"
#include "UnityEngine/Color.hpp"
namespace VainSabers::PCUI {
using UnityEngine::Color;
using UnityEngine::Transform;
Transform *Box(Transform *parent, float x, float y, float w, float h);
void Place(UnityEngine::Component *c, float x, float y, float w, float h);
void ConfigureFloatingPanel(UnityEngine::GameObject *panel);
void Back(Transform *parent, Color color = {.1f, .1f, .1f, 1});
void Solid(Transform *parent, Color color);
TMPro::TextMeshProUGUI *Text(Transform *parent, const std::string &text, float size = 3, bool center = false);
TMPro::TextMeshProUGUI *Button(Transform *parent, const std::string &text, std::function<void()> click,
                               Color color = {.3f, .5f, .8f, 1});
Transform *Panel(Transform *parent, float x, float y, float w, float h, const std::string &title);
void Number(Transform *parent, const std::string &label, float value, float lo, float hi, float step, int digits,
            std::function<void(float)> changed, Color tint = {1, 1, 1, 1}, float sensitivityCoef = 1);
Transform *Popup(Transform *parent, float x, float y, float w, float h, Color color = {.1f, .1f, .1f, 1});
void ClosePopup();
void CloseTopPopup();
void SetPopupCleanup(std::function<void()> cleanup);
void Toggle(Transform *parent, const std::string &label, bool value, std::function<void(bool)> changed);
void Dropdown(Transform *parent, const std::string &label, const std::string &value, std::vector<std::string> values,
              std::function<void(std::string)> changed);
void Input(Transform *parent, const std::string &value, std::function<void(std::string)> changed);
struct Form {
    Transform *tr;
    float width, y = 0;
    Transform *Row(float h = 4) {
        auto row = Box(tr, 0, y, width, h);
        auto e = row->get_gameObject()->AddComponent<UnityEngine::UI::LayoutElement *>();
        e->set_preferredHeight(h);
        e->set_minHeight(0);
        e->set_flexibleHeight(0);
        y += h + .5f;
        return row;
    }
    void Space(float height) {
        Row(height);
    }
    void Header(const std::string &name) {
        auto row = Row(6);
        Solid(Box(row, 0, 0, width, .5f), {.7f, .7f, .7f, .1f});
        auto label = Box(row, 0, 2, width, 3.5f);
        Solid(label, {.02f, .02f, .02f, .1f});
        Text(label, name, 3.3f, true)->set_color({.9f, .9f, .9f, .9f});
    }
    void Number(const std::string &label, float value, float lo, float hi, float step, int digits,
                std::function<void(float)> action, Color tint = {1, 1, 1, 1}, float sensitivityCoef = 1) {
        PCUI::Number(Row(), label, value, lo, hi, step, digits, action, tint, sensitivityCoef);
    }
    void Toggle(const std::string &label, bool value, std::function<void(bool)> action) {
        PCUI::Toggle(Row(), label, value, action);
    }
    void Dropdown(const std::string &label, const std::string &value, std::vector<std::string> choices,
                  std::function<void(std::string)> action) {
        PCUI::Dropdown(Row(), label, value, std::move(choices), action);
    }
};
void FitForm(Form &form, float height);
void GradientButton(Transform *parent, rapidjson::Value &keys, float fallback, std::function<void()> action);
void EditFloatGradient(Transform *parent, rapidjson::Value &keys, float fallback, PresetDocument &doc,
                       std::function<void()> changed);
void BuildAnimators(Form &form, rapidjson::Value &part, PresetDocument &doc, std::function<void()> changed,
                    std::function<void()> rebuild);
void TrailGradientFields(Form &form, rapidjson::Value &trail, PresetDocument &doc, std::function<void()> changed);
void AssetDropdown(Transform *parent, const std::string &label, rapidjson::Value &object, const char *key,
                   PresetDocument &doc, bool texture, std::function<void()> changed);
void TextureField(Transform *parent, const std::string &label, rapidjson::Value &object, const char *prefix,
                  PresetDocument &doc, std::function<void()> changed);
} // namespace VainSabers::PCUI
