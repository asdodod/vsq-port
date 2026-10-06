#include "PcUI.hpp"
#include "FloatGradient.hpp"
#include "PresetData.hpp"
namespace VainSabers::PCUI {
std::string AnimatorName(rapidjson::Value &a) {
    auto v = PresetDocument::Text(a, "$type", PresetDocument::Text(a, "type", "HueShiftAdder").c_str());
    auto end = v.find(',');
    if (end != std::string::npos)
        v.resize(end);
    auto ns = v.find_last_of('.');
    return ns == std::string::npos ? v : v.substr(ns + 1);
}
void FitForm(Form &f, float height) {
    auto rect = f.tr->get_gameObject()->GetComponent<UnityEngine::RectTransform *>();
    auto size = rect->get_sizeDelta();
    rect->set_sizeDelta({size.x, height});
    auto layout = f.tr->get_gameObject()->AddComponent<UnityEngine::UI::VerticalLayoutGroup *>();
    UI::Configure(layout);
    layout->set_spacing(.5f);
    // PC uses preferred heights in a fixed-height layout. Unity distributes
    // insufficient space between the fields instead of adding a ScrollView.
    for (int i = 0; i < f.tr->get_childCount(); ++i) {
        auto row = f.tr->GetChild(i);
        auto rt = row->GetComponent<UnityEngine::RectTransform *>();
        if (!rt)
            continue;
        auto preferred = row->get_gameObject()->GetComponent<UnityEngine::UI::LayoutElement *>();
        if (!preferred)
            preferred = row->get_gameObject()->AddComponent<UnityEngine::UI::LayoutElement *>();
        preferred->set_preferredHeight(rt->get_sizeDelta().y);
        preferred->set_minHeight(0);
        preferred->set_flexibleHeight(0);
        // Field controls fill their row after the layout allocates its height.
        for (int j = 0; j < row->get_childCount(); ++j) {
            auto child = row->GetChild(j)->GetComponent<UnityEngine::RectTransform *>();
            if (!child)
                continue;
            auto dimensions = child->get_sizeDelta(), pos = child->get_anchoredPosition();
            if (std::abs(dimensions.y - rt->get_sizeDelta().y) < .01f && std::abs(pos.y) < .01f) {
                child->set_anchorMin({0, 0});
                child->set_anchorMax({0, 1});
                child->set_sizeDelta({dimensions.x, 0});
                child->set_anchoredPosition({pos.x, 0});
            }
        }
    }
    UnityEngine::UI::LayoutRebuilder::ForceRebuildLayoutImmediate(rect);
}
void GradientButton(Transform *parent, rapidjson::Value &keys, float fallback, std::function<void()> action) {
    auto size = parent->get_gameObject()->GetComponent<UnityEngine::RectTransform *>()->get_sizeDelta();
    Button(parent, "", std::move(action), {.15f, .15f, .15f, 1});
    std::vector<FloatGradientKey> values;
    for (auto &k : keys.GetArray())
        if (k.IsObject())
            values.push_back({PresetDocument::Number(k, "time", 0), PresetDocument::Number(k, "value", fallback),
                              static_cast<int>(PresetDocument::Number(k, "easing", 0))});
    std::stable_sort(values.begin(), values.end(), [](auto a, auto b) { return a.time < b.time; });
    for (int i = 0; i < 48; ++i) {
        float v = std::clamp(EvaluateGradient(values, i / 47.f, fallback), 0.f, 1.f);
        Solid(Box(parent, size.x * i / 48.f, .5f, size.x / 48.f + .03f, 3), {v, v, v, 1});
    }
}
void EditFloatGradient(Transform *parent, rapidjson::Value &keys, float fallback, PresetDocument &doc,
                       std::function<void()> changed) {
    auto modal = BSML::Lite::CreateModal(parent, {76, 68}, nullptr, true);
    modal->moveToCenter = true;
    auto arr = &keys;
    auto d = &doc;
    auto index = std::make_shared<size_t>(0);
    auto root = std::make_shared<UnityW<UnityEngine::GameObject>>();
    auto build = std::make_shared<std::function<void()>>();
    std::weak_ptr<std::function<void()>> weak = build;
    *build = [modal, arr, d, index, root, weak, fallback, changed] {
        UI::Clear(*root);
        auto tr = Box(modal->get_transform(), -37, -32, 74, 64);
        *root = tr->get_gameObject();
        Form f{tr, 74};
        Text(f.Row(), "Gradient", 4, true);
        GradientButton(f.Row(7), *arr, fallback, [] {});
        auto row = f.Row();
        if (arr->Size())
            *index = std::min(*index, static_cast<size_t>(arr->Size() - 1));
        Button(Box(row, 0, 0, 12, 4), "<", [weak, index] {
            if (*index)
                --*index;
            if (auto r = weak.lock())
                (*r)();
        });
        Text(Box(row, 13, 0, 20, 4), std::to_string(arr->Size() ? *index + 1 : 0) + "/" + std::to_string(arr->Size()),
             3, true);
        Button(Box(row, 34, 0, 12, 4), ">", [weak, index, arr] {
            if (*index + 1 < arr->Size())
                ++*index;
            if (auto r = weak.lock())
                (*r)();
        });
        Button(Box(row, 47, 0, 12, 4), "-",
               [weak, index, arr, d, changed] {
                   if (arr->Size())
                       arr->Erase(arr->Begin() + *index);
                   d->dirty = true;
                   changed();
                   if (auto r = weak.lock())
                       (*r)();
               },
               {.7f, .15f, .25f, 1});
        Button(Box(row, 60, 0, 12, 4), "+",
               [weak, index, arr, d, fallback, changed] {
                   rapidjson::Value k(rapidjson::kObjectType);
                   d->SetNumber(k, "time", arr->Size() ? 1 : 0);
                   d->SetNumber(k, "value", fallback);
                   arr->PushBack(k, d->json.GetAllocator());
                   *index = arr->Size() - 1;
                   changed();
                   if (auto r = weak.lock())
                       (*r)();
               },
               {.15f, .6f, .25f, 1});
        if (arr->Size()) {
            auto k = &(*arr)[*index];
            f.Number("Position", PresetDocument::Number(*k, "time", 0), 0, 1, .01f, 2, [k, d, changed](float v) {
                d->SetNumber(*k, "time", v);
                changed();
            });
            f.Number("Value", PresetDocument::Number(*k, "value", fallback), -5, 16, .01f, 2, [k, d, changed](float v) {
                d->SetNumber(*k, "value", v);
                changed();
            });
            int easing = std::clamp(static_cast<int>(PresetDocument::Number(*k, "easing", 0)), 0, 3);
            std::vector<std::string> names{"Linear", "Ease In", "Ease Out", "Ease In Out"};
            f.Dropdown("Easing", names[easing], names, [k, d, changed, names](std::string v) {
                d->SetNumber(*k, "easing", std::find(names.begin(), names.end(), v) - names.begin(), true);
                changed();
            });
        }
        Button(f.Row(), "Close", [modal] { modal->Hide(); });
    };
    modal->onHide = [build, modal] { UnityEngine::Object::Destroy(modal->get_gameObject()); };
    (*build)();
    modal->Show();
}
void BuildAnimators(Form &f, rapidjson::Value &part, PresetDocument &doc, std::function<void()> changed,
                    std::function<void()> rebuild) {
    auto p = &part;
    auto d = &doc;
    auto type = std::make_shared<std::string>("HueShiftAdder");
    auto row = f.Row();
    Dropdown(Box(row, 0, 0, f.width - 7, 4), "", *type,
             {"HueShiftAdder", "HueShiftOscillator", "GlowOscillator", "OpacityOscillator", "PositionOscillator",
              "RotationAdder", "RotationOscillator"},
             [type](std::string v) { *type = v; });
    Button(Box(row, f.width - 5, 0, 5, 4), "+",
           [p, d, type, changed, rebuild] {
               auto &list = d->Ensure(*p, "animators");
               if (!list.IsArray())
                   list.SetArray();
               rapidjson::Value a(rapidjson::kObjectType);
               d->SetText(a, "$type", *type + ", VainSabers");
               list.PushBack(a, d->json.GetAllocator());
               changed();
               rebuild();
           },
           {.15f, .6f, .25f, 1});
    auto list = PresetDocument::Find(part, "animators");
    if (!list || !list->IsArray())
        return;
    for (size_t i = 0; i < list->Size(); ++i) {
        auto a = &(*list)[i];
        if (!a->IsObject())
            continue;
        row = f.Row();
        Button(Box(row, 0, 0, f.width - 7, 4), AnimatorName(*a),
               [a, d, row, changed] {
                   auto modal = BSML::Lite::CreateModal(row, {66, 44}, nullptr, true);
                   modal->moveToCenter = true;
                   Form af{Box(modal->get_transform(), -32, -20, 64, 40), 64};
                   auto name = AnimatorName(*a);
                   Text(af.Row(), name, 3.5f, true);
                   auto n = [a, d, changed, &af](const char *key, float def, float lo, float hi, float step) {
                       af.Number(key, PresetDocument::Number(*a, key, def), lo, hi, step, 2,
                                 [a, d, key, changed](float v) {
                                     d->SetNumber(*a, key, v);
                                     changed();
                                 });
                   };
                   if (name.find("Adder") != std::string::npos)
                       n("speed", name == "HueShiftAdder" ? .5f : 30, name == "HueShiftAdder" ? -3 : -180,
                         name == "HueShiftAdder" ? 3 : 180, .01f);
                   else {
                       n("amplitude", .5f, -3, 3, .01f);
                       n("frequency", .5f, 0, 10, .01f);
                   }
                   if (name.find("Rotation") != std::string::npos || name.find("Position") != std::string::npos)
                       af.Dropdown("Axis",
                                   PresetDocument::Number(*a, "axis", 0) == 0   ? "X"
                                   : PresetDocument::Number(*a, "axis", 0) == 1 ? "Y"
                                                                                : "Z",
                                   {"X", "Y", "Z"}, [a, d, changed](std::string v) {
                                       d->SetNumber(*a, "axis", v == "X" ? 0 : v == "Y" ? 1 : 2, true);
                                       changed();
                                   });
                   Button(af.Row(), "Close", [modal] { modal->Hide(); });
                   modal->onHide = [modal] { UnityEngine::Object::Destroy(modal->get_gameObject()); };
                   modal->Show();
               },
               {.15f, .15f, .15f, 1});
        Button(Box(row, f.width - 5, 0, 5, 4), "-",
               [list, i, d, changed, rebuild] {
                   list->Erase(list->Begin() + i);
                   d->dirty = true;
                   changed();
                   rebuild();
               },
               {.7f, .15f, .25f, 1});
    }
}
void TrailGradientFields(Form &f, rapidjson::Value &trail, PresetDocument &doc, std::function<void()> changed) {
    trail.MemberReserve(trail.MemberCount() + 32, doc.json.GetAllocator());
    auto &colors = doc.Ensure(trail, "colorGradient");
    if (!colors.IsArray())
        colors.SetArray();
    auto &blends = doc.Ensure(trail, "customBlendGradient");
    if (!blends.IsArray())
        blends.SetArray();
    auto d = &doc;
    auto colorArray = &colors, blendArray = &blends;
    UnityEngine::Color fallback{PresetDocument::Component(trail, "color", 0, 1),
                                PresetDocument::Component(trail, "color", 1, 1),
                                PresetDocument::Component(trail, "color", 2, 1), 1};
    auto row = f.Row();
    Text(Box(row, 0, 0, f.width / 2, 4), "Gradient");
    auto field = Box(row, f.width / 2, 0, f.width / 2, 4);
    Button(field, "",
           [field, colorArray, d, changed, fallback] {
               auto modal = BSML::Lite::CreateModal(field, {76, 68}, nullptr, true);
               modal->moveToCenter = true;
               auto index = std::make_shared<size_t>(0);
               auto root = std::make_shared<UnityW<UnityEngine::GameObject>>();
               auto rebuild = std::make_shared<std::function<void()>>();
               std::weak_ptr<std::function<void()>> weak = rebuild;
               *rebuild = [modal, index, colorArray, d, changed, fallback, root, weak] {
                   UI::Clear(*root);
                   auto tr = Box(modal->get_transform(), -37, -32, 74, 64);
                   *root = tr->get_gameObject();
                   Form form{tr, 74};
                   Text(form.Row(), "Color Gradient", 4, true);
                   if (colorArray->Size())
                       *index = std::min(*index, static_cast<size_t>(colorArray->Size() - 1));
                   auto row = form.Row();
                   Button(Box(row, 0, 0, 12, 4), "<", [index, weak] {
                       if (*index)
                           --*index;
                       if (auto r = weak.lock())
                           (*r)();
                   });
                   Text(Box(row, 13, 0, 20, 4),
                        std::to_string(colorArray->Size() ? *index + 1 : 0) + "/" + std::to_string(colorArray->Size()),
                        3, true);
                   Button(Box(row, 34, 0, 12, 4), ">", [index, weak, colorArray] {
                       if (*index + 1 < colorArray->Size())
                           ++*index;
                       if (auto r = weak.lock())
                           (*r)();
                   });
                   Button(Box(row, 47, 0, 12, 4), "-",
                          [index, weak, colorArray, d, changed] {
                              if (colorArray->Size())
                                  colorArray->Erase(colorArray->Begin() + *index);
                              d->dirty = true;
                              changed();
                              if (auto r = weak.lock())
                                  (*r)();
                          },
                          {.7f, .15f, .25f, 1});
                   Button(Box(row, 60, 0, 12, 4), "+",
                          [index, weak, colorArray, d, changed, fallback] {
                              rapidjson::Value k(rapidjson::kObjectType);
                              d->SetNumber(k, "time", colorArray->Size() ? 1 : 0);
                              d->SetComponent(k, "color", 0, fallback.r, 1);
                              d->SetComponent(k, "color", 1, fallback.g, 1);
                              d->SetComponent(k, "color", 2, fallback.b, 1);
                              colorArray->PushBack(k, d->json.GetAllocator());
                              *index = colorArray->Size() - 1;
                              changed();
                              if (auto r = weak.lock())
                                  (*r)();
                          },
                          {.15f, .6f, .25f, 1});
                   if (colorArray->Size()) {
                       auto k = &(*colorArray)[*index];
                       form.Number("Position", PresetDocument::Number(*k, "time", 0), 0, 1, .01f, 2,
                                   [k, d, changed](float v) {
                                       d->SetNumber(*k, "time", v);
                                       changed();
                                   });
                       const Color tints[] = {{243.f / 255 * 1.3f, 118.f / 255 * 1.3f, 156.f / 255 * 1.3f, 1},
                                              {114.f / 255 * 1.3f, 238.f / 255 * 1.3f, 145.f / 255 * 1.3f, 1},
                                              {121.f / 255 * 1.3f, 167.f / 255 * 1.3f, 247.f / 255 * 1.3f, 1}};
                       for (size_t i = 0; i < 3; i++)
                           form.Number(
                               std::string(1, "RGB"[i]), PresetDocument::Component(*k, "color", i, 1), 0, 1, .005f, 3,
                               [k, d, changed, i](float v) {
                                   d->SetComponent(*k, "color", i, v, 1);
                                   changed();
                               },
                               tints[i]);
                       std::vector<std::string> names{"Linear", "Ease In", "Ease Out", "Ease In Out"};
                       int easing = std::clamp(static_cast<int>(PresetDocument::Number(*k, "easing", 0)), 0, 3);
                       form.Dropdown("Easing", names[easing], names, [k, d, changed, names](std::string v) {
                           d->SetNumber(*k, "easing", std::find(names.begin(), names.end(), v) - names.begin(), true);
                           changed();
                       });
                   }
                   Button(form.Row(), "Close", [modal] { modal->Hide(); });
               };
               modal->onHide = [rebuild, modal] { UnityEngine::Object::Destroy(modal->get_gameObject()); };
               (*rebuild)();
               modal->Show();
           },
           {.15f, .15f, .15f, 1});
    SaberTrailData data;
    data.color = fallback;
    data.customBlend = 0;
    for (auto &key : colors.GetArray())
        if (key.IsObject())
            data.colorGradient.push_back(
                {PresetDocument::Number(key, "time", 0),
                 {PresetDocument::Component(key, "color", 0, 1), PresetDocument::Component(key, "color", 1, 1),
                  PresetDocument::Component(key, "color", 2, 1), 1},
                 static_cast<int>(PresetDocument::Number(key, "easing", 0))});
    std::stable_sort(data.colorGradient.begin(), data.colorGradient.end(),
                     [](auto a, auto b) { return a.time < b.time; });
    for (int i = 0; i < 48; i++)
        Solid(Box(field, f.width / 2 * i / 48.f, .5f, f.width / 96.f + .03f, 3),
              TrailColor(data, i / 47.f, {0, 0, 0, 1}));
    row = f.Row();
    Text(Box(row, 0, 0, f.width / 2, 4), "Custom Blend");
    float blend = PresetDocument::Number(trail, "customBlend", 1);
    GradientButton(Box(row, f.width / 2, 0, f.width / 2, 4), blends, blend,
                   [row, blendArray, d, changed, blend] { EditFloatGradient(row, *blendArray, blend, *d, changed); });
}
} // namespace VainSabers::PCUI
