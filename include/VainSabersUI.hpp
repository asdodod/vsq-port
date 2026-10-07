#pragma once
#include "VainSabersSettingsMenu.hpp"
#include "bsml/shared/BSML-Lite/Creation/Layout.hpp"
#include "bsml/shared/BSML-Lite/Creation/Settings.hpp"
#include "bsml/shared/BSML-Lite/Creation/Text.hpp"
#include "bsml/shared/BSML-Lite/Creation/Buttons.hpp"
#include "bsml/shared/BSML-Lite/Creation/Misc.hpp"
#include "bsml/shared/BSML/Components/ExternalComponents.hpp"
#include "UnityEngine/GameObject.hpp"
#include "UnityEngine/Object.hpp"
#include "UnityEngine/RectTransform.hpp"
#include "UnityEngine/TextAnchor.hpp"
#include "UnityEngine/UI/LayoutElement.hpp"
#include "UnityEngine/UI/LayoutRebuilder.hpp"
#include "UnityEngine/UI/ContentSizeFitter.hpp"
#include "TMPro/FontStyles.hpp"
#include <functional>
#include <memory>

namespace VainSabers {
struct MenuEditorState {
    std::shared_ptr<BSML::BSMLParser> homeParser;
    PresetDocument document;
    std::string status, saveAs, exportConfirmation;
    bool editing = false, preview = true, holdSabers = true;
    size_t trail = 0, ring = 0;
    bool bladeTrails = false;
    int deleteConfirm = 0, revertConfirm = 0;
    float previewLast = -1;
    bool previewQueued = false;
    int previewEpoch = 0;
};
namespace UI {
inline void Size(UnityEngine::Component *c, float height = 7, float width = -1) {
    auto go = c->get_gameObject();
    auto e = go->GetComponent<UnityEngine::UI::LayoutElement *>();
    auto fitter = go->GetComponent<UnityEngine::UI::ContentSizeFitter *>();
    if (fitter)
        fitter->set_enabled(false);
    if (!e)
        e = go->AddComponent<UnityEngine::UI::LayoutElement *>();
    e->set_minHeight(height);
    e->set_preferredHeight(height);
    e->set_flexibleHeight(0);
    if (width >= 0) {
        e->set_minWidth(width);
        e->set_preferredWidth(width);
        e->set_flexibleWidth(0);
    }
}
inline void Configure(UnityEngine::UI::HorizontalOrVerticalLayoutGroup *g) {
    g->set_childControlHeight(true);
    g->set_childControlWidth(true);
    g->set_childForceExpandHeight(false);
    g->set_childForceExpandWidth(true);
    g->set_childAlignment(UnityEngine::TextAnchor::UpperLeft);
    g->set_spacing(1);
}
inline void Fill(UnityEngine::GameObject *go, float margin = 3) {
    auto rt = go->GetComponent<UnityEngine::RectTransform *>();
    rt->set_anchorMin({0, 0});
    rt->set_anchorMax({1, 1});
    rt->set_sizeDelta({-margin * 2, -margin * 2});
    rt->set_anchoredPosition({0, 0});
    auto fitter = go->GetComponent<UnityEngine::UI::ContentSizeFitter *>();
    if (fitter)
        fitter->set_enabled(false);
}
inline UnityEngine::Transform *Column(UnityEngine::Transform *parent) {
    auto g = BSML::Lite::CreateVerticalLayoutGroup(parent);
    Configure(g);
    return g->get_transform();
}
inline UnityEngine::Transform *Row(UnityEngine::Transform *parent, float height = 7) {
    auto g = BSML::Lite::CreateHorizontalLayoutGroup(parent);
    Configure(g);
    Size(g, height);
    return g->get_transform();
}
inline UnityEngine::Transform *Scroll(UnityEngine::Transform *parent, float width, float height = 88) {
    auto go = BSML::Lite::CreateScrollView(parent);
    Configure(go->GetComponent<UnityEngine::UI::VerticalLayoutGroup *>());
    auto e = go->GetComponent<BSML::ExternalComponents *>();
    if (e) {
        auto rt = e->Get<UnityEngine::RectTransform *>();
        if (rt) {
            Fill(rt->get_gameObject(), 2);
            Size(rt, height);
        }
    }
    auto le = go->GetComponent<UnityEngine::UI::LayoutElement *>();
    if (!le)
        le = go->AddComponent<UnityEngine::UI::LayoutElement *>();
    le->set_preferredWidth(width);
    le->set_minWidth(width);
    return go->get_transform();
}
inline void Label(UnityEngine::Transform *parent, const std::string &text, bool title = false) {
    auto t = BSML::Lite::CreateText(parent, StringW(text), title ? TMPro::FontStyles::Bold : TMPro::FontStyles::Normal,
                                    title ? 3.2f : 2.7f);
    Size(t, title ? 6.5f : text.size() > 70 ? 12 : 6);
    t->set_enableWordWrapping(!title && text.size() > 70);
}
inline void Button(UnityEngine::Transform *tr, const std::string &name, std::function<void()> action,
                   float width = -1) {
    auto b = BSML::Lite::CreateUIButton(tr, StringW(name), action);
    Size(b, 7, width);
    BSML::Lite::SetButtonTextSize(b, 2.7f);
}
inline BSML::DropdownListSetting *Dropdown(UnityEngine::Transform *tr, const std::string &label,
                                           const std::string &value, std::vector<std::string> items,
                                           std::function<void(std::string)> action) {
    auto names = std::make_shared<std::vector<std::string>>(std::move(items));
    std::vector<std::string_view> views;
    for (auto &n : *names)
        views.emplace_back(n);
    auto d = BSML::Lite::CreateDropdown(tr, StringW(label), StringW(value), std::span<std::string_view>(views),
                                        [names, action](StringW v) { action(static_cast<std::string>(v)); });
    Size(d);
    return d;
}
inline void Clear(UnityW<UnityEngine::GameObject> &go) {
    if (go) {
        go->SetActive(false);
        UnityEngine::Object::Destroy(go);
        go = nullptr;
    }
}
} // namespace UI
} // namespace VainSabers
