#include "PcUI.hpp"
#include "PcUiSprite.hpp"
#include "PcNumberDrag.hpp"
#include "PcNumberGesture.hpp"
#include "bsml/shared/BSML-Lite/Creation/Image.hpp"
#include "bsml/shared/Helpers/getters.hpp"
#include "UnityEngine/Sprite.hpp"
#include "UnityEngine/SpriteMeshType.hpp"
#include "UnityEngine/Texture2D.hpp"
#include "UnityEngine/UI/Image.hpp"
#include "UnityEngine/UI/Button.hpp"
#include "UnityEngine/Material.hpp"
#include "TMPro/TMP_FontAsset.hpp"
#include "UnityEngine/Resources.hpp"
#include "UnityEngine/Canvas.hpp"
#include "UnityEngine/RenderMode.hpp"
#include "UnityEngine/AdditionalCanvasShaderChannels.hpp"
#include "UnityEngine/UI/RectMask2D.hpp"
#include "UnityEngine/UI/CanvasScaler.hpp"
#include "VRUIControls/VRGraphicRaycaster.hpp"
#include "UnityEngine/Vector4.hpp"
#include "TMPro/TextAlignmentOptions.hpp"
#include "TMPro/TextOverflowModes.hpp"
#include <cmath>
#include <cstdio>
#include <cctype>
namespace VainSabers::PCUI {
namespace {
UnityW<UnityEngine::Sprite> roundSprite, arrowSprite, solidSprite;
UnityW<UnityEngine::Material> noGlow, fontMaterial;
struct PopupEntry {
    UnityW<UnityEngine::GameObject> object;
    std::function<void()> cleanup;
};
std::vector<PopupEntry> popups;
UnityEngine::Material *FontMaterial() {
    if (!fontMaterial)
        if (auto source = BSML::Helpers::GetMainUIFontMaterial()) {
            fontMaterial = UnityEngine::Object::Instantiate<UnityEngine::Material *>(source);
            fontMaterial->set_name("VainSabers Quest UI text");
        }
    return fontMaterial.unsafePtr();
}
UnityEngine::Material *NoGlow() {
    if (!noGlow)
        if (auto source = BSML::Helpers::GetUINoGlowMat()) {
            noGlow = UnityEngine::Object::Instantiate<UnityEngine::Material *>(source);
            // Canvas sorting order cannot override different material render queues.
            if (auto font = FontMaterial())
                noGlow->set_renderQueue(font->get_renderQueue());
        }
    return noGlow.unsafePtr();
}
UnityEngine::Sprite *Round() {
    if (!roundSprite) {
        auto raw = BSML::Lite::Base64ToSprite(kVainUiRoundBase64);
        auto tex = raw->get_texture();
        float w = tex->get_width(), h = tex->get_height();
        roundSprite =
            UnityEngine::Sprite::Create(tex, {0, 0, w, h}, {.5f, .5f}, 320, 0, UnityEngine::SpriteMeshType::FullRect,
                                        {w * .5f, h * .5f, w * .5f, h * .5f});
    }
    return roundSprite;
}
std::string Format(float value, int digits) {
    char buffer[64];
    std::snprintf(buffer, sizeof(buffer), "%.*f", digits, value);
    return buffer;
}
void Fit(UnityEngine::Component *c) {
    c->get_gameObject()->set_layer(5);
    UI::Fill(c->get_gameObject(), 0);
}
} // namespace
void ConfigureFloatingPanel(UnityEngine::GameObject *panel) {
    panel->set_layer(5);
    // PC SimpleFloatingPanel does not mask its entire curved canvas.
    // Scroll viewports retain their own masks.
    if (auto mask = panel->GetComponent<UnityEngine::UI::RectMask2D *>())
        mask->set_enabled(false);
    if (auto canvas = panel->GetComponent<UnityEngine::Canvas *>()) {
        canvas->set_renderMode(UnityEngine::RenderMode::WorldSpace);
        canvas->set_overrideSorting(true);
        canvas->set_sortingOrder(10);
        canvas->set_additionalShaderChannels(
            UnityEngine::AdditionalCanvasShaderChannels(canvas->get_additionalShaderChannels().value__ | 3));
    }
}
void Place(UnityEngine::Component *c, float x, float y, float w, float h) {
    c->get_gameObject()->set_layer(5);
    if (auto fitter = c->get_gameObject()->GetComponent<UnityEngine::UI::ContentSizeFitter *>())
        fitter->set_enabled(false);
    auto rt = c->get_gameObject()->GetComponent<UnityEngine::RectTransform *>();
    rt->set_anchorMin({0, 1});
    rt->set_anchorMax({0, 1});
    rt->set_pivot({0, 1});
    rt->set_sizeDelta({w, h});
    rt->set_anchoredPosition({x, -y});
}
Transform *Box(Transform *parent, float x, float y, float w, float h) {
    auto go = UnityEngine::GameObject::New_ctor("VainSabers PC UI");
    go->set_layer(5);
    auto rt = go->AddComponent<UnityEngine::RectTransform *>();
    rt->SetParent(parent, false);
    Place(rt, x, y, w, h);
    return rt;
}
void Back(Transform *parent, Color color) {
    auto image = BSML::Lite::CreateImage(parent, Round());
    if (auto mat = NoGlow())
        image->set_material(mat);
    image->set_type(UnityEngine::UI::Image::Type::Sliced);
    image->set_color(color);
    image->set_raycastTarget(false);
    Fit(image);
    image->get_transform()->SetAsFirstSibling();
}
void Solid(Transform *parent, Color color) {
    if (!solidSprite) {
        auto tex = UnityEngine::Texture2D::get_whiteTexture();
        solidSprite = UnityEngine::Sprite::Create(
            tex, {0, 0, static_cast<float>(tex->get_width()), static_cast<float>(tex->get_height())}, {.5f, .5f}, 320);
    }
    auto image = BSML::Lite::CreateImage(parent, solidSprite);
    if (auto mat = NoGlow())
        image->set_material(mat);
    image->set_type(UnityEngine::UI::Image::Type::Simple);
    image->set_color(color);
    image->set_raycastTarget(false);
    Fit(image);
    image->get_transform()->SetAsFirstSibling();
}
TMPro::TextMeshProUGUI *Text(Transform *parent, const std::string &text, float size, bool center) {
    auto t = BSML::Lite::CreateText(parent, StringW(text), TMPro::FontStyles::Normal, size);
    Fit(t);
    t->set_alignment(center ? TMPro::TextAlignmentOptions::Center : TMPro::TextAlignmentOptions::TopLeft);
    if (auto mat = FontMaterial())
        t->set_fontSharedMaterial(mat);
    t->set_margin({0, center ? 0.f : .5f, 0, 0});
    t->set_color({.7f, .7f, .7f, 1});
    t->set_enableWordWrapping(false);
    t->set_raycastTarget(false);
    // PC fields permit vertical overflow at height 4. Ellipsis can delete the
    // entire first line when the font metrics are taller than that rectangle.
    t->set_overflowMode(TMPro::TextOverflowModes::Overflow);
    return t;
}
TMPro::TextMeshProUGUI *Button(Transform *parent, const std::string &text, std::function<void()> action, Color color) {
    auto b = BSML::Lite::CreateClickableImage(parent, Round(), std::move(action));
    Fit(b);
    b->set_type(UnityEngine::UI::Image::Type::Sliced);
    if (auto mat = NoGlow())
        b->set_material(mat);
    b->set_defaultColor({color.r * .85f, color.g * .85f, color.b * .85f, color.a});
    b->set_highlightColor(color);
    b->set_raycastTarget(true);
    auto label = Text(b->get_transform(), text, 3, true);
    label->set_color({.9f, .9f, .9f, 1});
    return label;
}
Transform *Panel(Transform *parent, float x, float y, float w, float h, const std::string &title) {
    auto root = Box(parent, x, y, w, h);
    Back(root);
    if (auto image = root->GetChild(0)->GetComponent<UnityEngine::UI::Image *>())
        image->set_raycastTarget(true);
    Text(Box(root, 0, 0, w, 5), title, 4.5f, true);
    return Box(root, 1, 7, w - 2, h - 8);
}
void ClosePopup() {
    while (!popups.empty())
        CloseTopPopup();
}
void CloseTopPopup() {
    if (popups.empty())
        return;
    auto entry = std::move(popups.back());
    popups.pop_back();
    if (entry.cleanup)
        entry.cleanup();
    if (entry.object) {
        entry.object->SetActive(false);
        UnityEngine::Object::Destroy(entry.object);
    }
}
void SetPopupCleanup(std::function<void()> cleanup) {
    if (!popups.empty())
        popups.back().cleanup = std::move(cleanup);
}
Transform *Popup(Transform *parent, float x, float y, float w, float h, Color color) {
    bool nested = !popups.empty() && popups.back().object &&
                  parent->IsChildOf(popups.back().object->get_transform());
    if (!nested)
        ClosePopup();
    auto root = Box(parent, x, y, w, h);
    auto activePopup = root->get_gameObject();
    popups.push_back({activePopup, {}});
    activePopup->set_name("VainSabers local popup");
    // Popup keys must not be descendants of PcNumberDrag. EventSystem walks
    // ancestors for pointer-down/up; otherwise pressing a key starts another
    // number gesture and its release replaces the keypad before the click.
    // Preserve the field-relative world position on the panel canvas.
    auto parentCanvas = parent->get_gameObject()->GetComponentInParent<UnityEngine::Canvas *>();
    if (parentCanvas)
        root->SetParent(parentCanvas->get_transform(), true);
    auto canvas = activePopup->AddComponent<UnityEngine::Canvas *>();
    canvas->set_renderMode(UnityEngine::RenderMode::WorldSpace);
    canvas->set_overrideSorting(true);
    canvas->set_sortingOrder(parentCanvas ? parentCanvas->get_sortingOrder() + 10 : 20);
    canvas->set_additionalShaderChannels(UnityEngine::AdditionalCanvasShaderChannels(3));
    auto scaler = activePopup->AddComponent<UnityEngine::UI::CanvasScaler *>();
    scaler->set_dynamicPixelsPerUnit(3.44f);
    scaler->set_referencePixelsPerUnit(10);
    auto raycaster = activePopup->AddComponent<VRUIControls::VRGraphicRaycaster *>();
    raycaster->____physicsRaycaster = BSML::Helpers::GetPhysicsRaycasterWithCache();
    Back(root, color);
    root->GetChild(0)->GetComponent<UnityEngine::UI::Image *>()->set_raycastTarget(true);
    auto blocker = Box(root, -200, -200, w + 400, h + 400);
    Button(blocker, "", [] { CloseTopPopup(); }, {0, 0, 0, 0});
    blocker->SetAsFirstSibling();
    return root;
}
void Number(Transform *parent, const std::string &label, float value, float lo, float hi, float step, int digits,
            std::function<void(float)> changed, Color tint, float sensitivityCoef) {
    auto size = parent->get_gameObject()->GetComponent<UnityEngine::RectTransform *>()->get_sizeDelta();
    float split = label.empty() ? 0 : size.x * .5f;
    if (split > 0)
        Text(Box(parent, 0, 0, split, 4), label);
    digits = NumberDecimals(step);
    step = NumberStep(step);
    auto field = Box(parent, split, 0, size.x - split, 4);
    Color color{.15f * tint.r, .15f * tint.g, .15f * tint.b, 1};
    auto hit = BSML::Lite::CreateImage(field, Round());
    Fit(hit);
    hit->set_type(UnityEngine::UI::Image::Type::Sliced);
    if (auto mat = NoGlow())
        hit->set_material(mat);
    hit->set_color({color.r * .85f, color.g * .85f, color.b * .85f, 1});
    hit->set_raycastTarget(true);
    auto current = std::make_shared<float>(SnapNumber(value, lo, hi, step));
    auto displayBox = Box(field, .5f, .5f, size.x - split - 1, 3);
    auto displayRect = displayBox->GetComponent<UnityEngine::RectTransform *>();
    displayRect->set_anchorMin({0, 0});
    displayRect->set_anchorMax({0, 1});
    displayRect->set_sizeDelta({size.x - split - 1, -1});
    displayRect->set_anchoredPosition({.5f, -.5f});
    UnityW<TMPro::TextMeshProUGUI> display = Text(displayBox, Format(*current, digits), 3, true);
    display->set_color({.8f, .8f, .8f, .7f});
    std::function<void(float)> update = [current, display, changed, lo, hi, step, digits](float next) mutable {
        if (!std::isfinite(next))
            return;
        float snapped = SnapNumber(next, lo, hi, step);
        if (std::abs(snapped - *current) < .00001f)
            return;
        *current = snapped;
        if (display)
            display->set_text(StringW(Format(*current, digits)));
        changed(*current);
    };
    if (!arrowSprite)
        arrowSprite = BSML::Lite::Base64ToSprite(kVainDropdownArrowBase64);
    for (int side = 0; side < 2; ++side) {
        auto arrow =
            BSML::Lite::CreateImage(Box(field, side ? size.x - split - 2.75f : 1.25f, 1.25f, 1.5f, 1.5f), arrowSprite);
        Fit(arrow);
        arrow->get_transform()->set_localEulerAngles({0, 0, side ? 90.f : -90.f});
        arrow->set_color({.9f, .9f, .9f, .1f});
        arrow->set_raycastTarget(false);
        if (auto mat = NoGlow())
            arrow->set_material(mat);
    }
    auto input = [field, current, update, tint] {
        auto width = field->get_gameObject()->GetComponent<UnityEngine::RectTransform *>()->get_rect().width;
        auto popup = Popup(field, width * .5f - 10, -11, 20, 30, {.07f * tint.r, .07f * tint.g, .07f * tint.b, 1});
        constexpr float rowHeight = (30.f - 4.f - 2.5f) / 6.f;
        auto buffer = std::make_shared<std::string>();
        auto headerBox = Box(popup, 2, 2, 16, rowHeight);
        Back(headerBox, {.2f, .2f, .2f, .7f});
        UnityW<TMPro::TextMeshProUGUI> header = Text(headerBox, "", 4, true);
        std::function<void()> refresh = [header, buffer]() mutable {
            if (header)
                header->set_text(StringW(*buffer));
        };
        for (int i = 0; i < 12; i++) {
            std::string key = i < 9 ? std::to_string(i + 1) : i == 9 ? "~" : i == 10 ? "0" : ".";
            auto text = Button(Box(popup, 2 + (i % 3) * 5.5f, 2 + (1 + i / 3) * (rowHeight + .5f), 5, rowHeight), key,
                               [buffer, refresh, key] {
                                   if (key == "~") {
                                       if (buffer->starts_with('-'))
                                           buffer->erase(0, 1);
                                       else
                                           buffer->insert(0, "-");
                                   } else if (buffer->size() < 10) {
                                       if (key == ".") {
                                           if (buffer->find('.') == std::string::npos) {
                                               if (buffer->empty() || *buffer == "-")
                                                   *buffer += "0";
                                               *buffer += ".";
                                           }
                                       } else
                                           *buffer += key;
                                   }
                                   refresh();
                               },
                               {.2f, .2f, .25f, 1});
            text->set_fontSize(3.5f);
        }
        // The keypad follows the PC row order: 123,456,789,~0.,backspace/OK.
        Button(Box(popup, 2, 2 + 5 * (rowHeight + .5f), 5, rowHeight), "<",
               [buffer, refresh] {
                   if (!buffer->empty())
                       buffer->pop_back();
                   refresh();
               },
               {.2f, .2f, .25f, 1});
        Button(Box(popup, 7.5f, 2 + 5 * (rowHeight + .5f), 10.5f, rowHeight), "OK",
               [buffer, update] {
                   float v = 0;
                   bool valid = false;
                   try {
                       size_t end = 0;
                       v = std::stof(*buffer, &end);
                       valid = end == buffer->size() && std::isfinite(v);
                   } catch (...) {
                   }
                   CloseTopPopup();
                   if (valid)
                       update(v);
               },
               {.2f, .5f, .2f, 1});
    };
    // PC coefficient defaults to 0.05 units per yaw degree.
    if (sensitivityCoef == 1) {
        if (label == "Blur MS")
            sensitivityCoef = 20;
        else if (label == "Blade Trail MS" || label == "Tip Trail MS" || label == "Length (ms)")
            sensitivityCoef = 100;
        else if (label == "Z Rotation")
            sensitivityCoef = 90;
        else if (label == "Radius" || label == "Width")
            sensitivityCoef = .03f;
        else if (label == "Length")
            sensitivityCoef = .1f;
    }
    field->get_gameObject()->AddComponent<PcNumberDrag *>()->Bind([current] { return *current; }, update, input,
                                                                  .05f * sensitivityCoef);
}
void Toggle(Transform *parent, const std::string &label, bool value, std::function<void(bool)> changed) {
    auto size = parent->get_gameObject()->GetComponent<UnityEngine::RectTransform *>()->get_sizeDelta();
    Text(Box(parent, 0, 0, size.x - 8, 4), label);
    auto current = std::make_shared<bool>(value);
    auto knob = std::make_shared<UnityW<HMUI::ImageView>>();
    auto inner = Box(parent, size.x - 8, 0, 8, 4);
    Back(inner, {.15f, .15f, .15f, 1});
    auto knobBox = Box(inner, value ? 4 : .5f, .5f, 3.5f, 3);
    auto k = BSML::Lite::CreateImage(knobBox, Round());
    Fit(k);
    k->set_type(UnityEngine::UI::Image::Type::Sliced);
    k->set_raycastTarget(false);
    if (auto mat = NoGlow())
        k->set_material(mat);
    k->set_color(value ? Color{.3f, .5f, .8f, 1} : Color{.7f, .15f, .15f, 1});
    *knob = k;
    auto b = BSML::Lite::CreateClickableImage(inner, Round(), [current, knob, knobBox, changed] {
        *current = !*current;
        Place(knobBox, *current ? 4 : .5f, .5f, 3.5f, 3);
        if (*knob)
            (*knob)->set_color(*current ? Color{.3f, .5f, .8f, 1} : Color{.7f, .15f, .15f, 1});
        changed(*current);
    });
    Fit(b);
    b->set_defaultColor({0, 0, 0, 0});
    b->set_highlightColor({.2f, .2f, .2f, .15f});
    b->set_raycastTarget(true);
}
void Dropdown(Transform *parent, const std::string &label, const std::string &value, std::vector<std::string> values,
              std::function<void(std::string)> changed) {
    auto size = parent->get_gameObject()->GetComponent<UnityEngine::RectTransform *>()->get_sizeDelta();
    float split = label.empty() ? 0 : size.x * .5f;
    if (split > 0)
        Text(Box(parent, 0, 0, split, 4), label);
    auto holder = std::make_shared<UnityW<TMPro::TextMeshProUGUI>>();
    auto field = Box(parent, split, 0, size.x - split, 4);
    auto current = std::make_shared<std::string>(value);
    *holder = Button(field, value,
                     [field, values, changed, holder, current] {
                         if (values.empty())
                             return;
                         float width =
                             field->get_gameObject()->GetComponent<UnityEngine::RectTransform *>()->get_rect().width;
                         float height = values.size() * 4.f;
                         auto list = Popup(field, 0, -height - .5f, width, height);
                         for (size_t j = 0; j < values.size(); ++j) {
                             auto item = values[j];
                             auto text = Button(
                                 Box(list, 0, j * 4.f, width, 4), item,
                                 [item, holder, current, changed] {
                                     CloseTopPopup();
                                     *current = item;
                                     if (*holder)
                                         (*holder)->set_text(StringW(item));
                                     changed(item);
                                 },
                                 item == *current ? Color{.3f, .5f, .8f, .5f} : Color{0, 0, 0, 0});
                             text->set_alignment(TMPro::TextAlignmentOptions::Left);
                             text->set_margin({1.5f, 0, 0, 0});
                         }
                     },
                     {.15f, .15f, .15f, 1});
    auto valueLabel = holder->unsafePtr();
    Place(valueLabel, 1.5f, 0, size.x - split - 5, 4);
    valueLabel->set_alignment(TMPro::TextAlignmentOptions::Left);
    if (!arrowSprite)
        arrowSprite = BSML::Lite::Base64ToSprite(kVainDropdownArrowBase64);
    auto arrow = BSML::Lite::CreateImage(
        Box(valueLabel->get_transform()->get_parent(), size.x - split - 3.5f, .5f, 3, 3), arrowSprite);
    Fit(arrow);
    arrow->set_raycastTarget(false);
    arrow->set_color({.7f, .7f, .7f, 1});
    if (auto mat = NoGlow())
        arrow->set_material(mat);
}
void Input(Transform *parent, const std::string &value, std::function<void(std::string)> changed) {
    auto current = std::make_shared<std::string>(value);
    Button(parent, "@",
           [parent, current, changed] {
               float width = parent->get_gameObject()->GetComponent<UnityEngine::RectTransform *>()->get_rect().width;
               auto root = Popup(parent, width * .5f - 20, -16, 40, 40, {.07f, .07f, .07f, 1});
               auto buffer = std::make_shared<std::string>(*current);
               auto shift = std::make_shared<bool>(false);
               auto head = Box(root, 2, 2, 36, 5);
               Back(head, {.2f, .2f, .2f, .7f});
               UnityW<TMPro::TextMeshProUGUI> display = Text(head, *buffer, 4, true);
               std::function<void()> refresh = [display, buffer]() mutable {
                   if (display)
                       display->set_text(StringW(*buffer));
               };
               const std::string rows[] = {"1234567890", "qwertyuiop", "asdfghjkl", "zxcvbnm", "-=[];',."};
               auto labels = std::make_shared<std::vector<std::pair<UnityW<TMPro::TextMeshProUGUI>, char>>>();
               for (int r = 0; r < 5; ++r) {
                   float keyWidth = (36.f - (rows[r].size() - 1) * .4f) / rows[r].size();
                   for (size_t i = 0; i < rows[r].size(); ++i) {
                       char key = rows[r][i];
                       auto text = Button(
                           Box(root, 2 + i * (keyWidth + .4f), 7.4f + r * 4.9f, keyWidth, 4.5f), std::string(1, key),
                           [buffer, shift, key, refresh] {
                               if (buffer->size() < 40)
                                   *buffer +=
                                       *shift ? static_cast<char>(std::toupper(static_cast<unsigned char>(key))) : key;
                               refresh();
                           },
                           {.2f, .2f, .25f, 1});
                       labels->emplace_back(text, key);
                   }
               }
               Button(Box(root, 2, 31.9f, 7, 4.5f), "SHIFT",
                      [shift, labels] {
                          *shift = !*shift;
                          for (auto &[text, key] : *labels)
                              if (text)
                                  text->set_text(StringW(std::string(
                                      1, *shift ? static_cast<char>(std::toupper(static_cast<unsigned char>(key)))
                                                : key)));
                      },
                      {.2f, .2f, .25f, 1})
                   ->set_fontSize(2.5f);
               Button(Box(root, 9.4f, 31.9f, 13, 4.5f), "_",
                      [buffer, refresh] {
                          if (buffer->size() < 40)
                              *buffer += " ";
                          refresh();
                      },
                      {.2f, .2f, .25f, 1});
               Button(Box(root, 22.8f, 31.9f, 6, 4.5f), "<",
                      [buffer, refresh] {
                          if (!buffer->empty())
                              buffer->pop_back();
                          refresh();
                      },
                      {.2f, .2f, .25f, 1});
               Button(Box(root, 29.2f, 31.9f, 8.8f, 4.5f), "OK",
                      [buffer, current, changed] {
                          auto next = *buffer;
                          ClosePopup();
                          if (*current != next) {
                              *current = next;
                              changed(next);
                          }
                      },
                      {.2f, .5f, .2f, 1});
           },
           {.15f, .15f, .15f, 1})
        ->set_fontSize(4);
}
} // namespace VainSabers::PCUI
