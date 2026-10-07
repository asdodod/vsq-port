#include "PcUI.hpp"
#include "PluginConfig.hpp"
#include "PresetFilePolicy.hpp"
#include "MenuSabers.hpp"
#include <filesystem>
#include <cmath>
namespace VainSabers {
void VainSabersMenuHost::DeletePreset() {
    auto s = State();
    auto &d = s->document;
    if (!s->editing || IsVainSaberExport(d.path.c_str()))
        return;
    std::error_code ec;
    if (!d.path.empty() && std::filesystem::exists(d.path, ec)) {
        std::filesystem::copy_file(d.path, d.path + ".deleted.bak", std::filesystem::copy_options::overwrite_existing,
                                   ec);
        if (ec) {
            s->status = "Cannot back up preset: " + ec.message();
            BuildEditor();
            return;
        }
        std::filesystem::remove(d.path, ec);
        if (ec) {
            s->status = "Delete failed: " + ec.message();
            BuildEditor();
            return;
        }
    }
    ClosePanel();
    s->status = "Deleted: " + d.name;
    ShowHome();
}
void VainSabersMenuHost::BuildEditor() {
    using namespace PCUI;
    using J = rapidjson::Value;
    ClosePopup();
    auto s = State();
    auto &d = s->document;
    // Numeric callbacks retain value pointers until this panel is rebuilt.
    // Reserve before creating them, so adding absent PC fields cannot move members.
    d.json.MemberReserve(d.json.MemberCount() + 32, d.json.GetAllocator());
    for (auto &part : d.Parts().GetArray())
        part.MemberReserve(part.MemberCount() + 128, d.json.GetAllocator());
    for (auto key : {"tipTrails", "bladeTrails"})
        if (auto list = PresetDocument::Find(d.json, key); list && list->IsArray())
            for (auto &t : list->GetArray())
                if (t.IsObject())
                    t.MemberReserve(t.MemberCount() + 64, d.json.GetAllocator());
    auto p = d.Part();
    if (!_floatingPanel) {
        auto screen = BSML::Lite::CreateFloatingScreen({250, 126}, {0, 1.2f, 2}, {0, 0, 0}, 120, false, false);
        _floatingPanel = screen->get_gameObject();
        PCUI::ConfigureFloatingPanel(_floatingPanel);
        screen->get_transform()->set_localScale({.018f, .018f, .018f});
    }
    UI::Clear(_editorRoot);
    auto root = Box(_floatingPanel->get_transform(), 0, 0, 250, 126);
    _editorRoot = root->get_gameObject();
    UI::Fill(_editorRoot, 0);
    // PC layout: four equal columns, with a 40-unit static preview when unheld.
    const float w = s->holdSabers ? 61 : 50.5f;
    Form cf{Panel(root, 0, 0, w, 34, "Config : " + d.name + (d.dirty ? " *" : "")), w - 2};
    auto row = cf.Row();
    Button(Box(row, 0, 0, w - 10, 4), "Save", [this] { SavePreset(); }, {.3f, .5f, .7f, 1});
    Button(Box(row, w - 8, 0, 6, 4), "<",
           [this] {
               ClosePanel();
               State()->status.clear();
               ShowHome();
           },
           {.55f, .35f, .2f, 1});
    Button(cf.Row(), s->exportConfirmation.empty() ? "Export" : s->exportConfirmation,
           [this] { ExportPreset(); }, {.3f, .45f, .3f, 1});
    Button(cf.Row(),
           s->deleteConfirm == 0   ? "Delete"
           : s->deleteConfirm == 1 ? "Sure?"
                                   : "Really?",
           [this] {
               auto s = State();
               s->revertConfirm = 0;
               if (++s->deleteConfirm >= 3) {
                   s->deleteConfirm = 0;
                   DeletePreset();
               } else
                   BuildEditor();
           },
           {.7f, .15f, .15f, 1});
    Button(cf.Row(),
           s->revertConfirm == 0   ? "Revert"
           : s->revertConfirm == 1 ? "Sure?"
                                   : "Really?",
           [this] {
               auto s = State();
               s->deleteConfirm = 0;
               if (++s->revertConfirm >= 3) {
                   s->revertConfirm = 0;
                   auto original = s->document.original;
                   s->document.Parse(original);
                   s->status.clear();
                   Preview();
               }
               BuildEditor();
           },
           {.6f, .5f, .2f, 1});
    Input(cf.Row(), s->saveAs, [this](std::string name) { State()->saveAs = name; });
    Button(cf.Row(), s->holdSabers ? "Un-hold Sabers" : "Hold Sabers", [this] {
        State()->holdSabers = !State()->holdSabers;
        BuildEditor();
        Preview();
    });
    auto partBody = Panel(root, 0, 36, w, 90, "Part");
    Form pf{partBody, w - 2};
    std::vector<std::string> parts;
    for (auto &v : d.Parts().GetArray())
        parts.push_back(PresetDocument::Text(v, "name", "Part"));
    row = pf.Row();
    Dropdown(Box(row, 0, 0, w - 17, 4), "", p ? parts[d.part] : "", parts, [this, parts](std::string v) {
        State()->document.part = std::find(parts.begin(), parts.end(), v) - parts.begin();
        State()->ring = 0;
        BuildEditor();
    });
    Button(Box(row, w - 16, 0, 4, 4), "@",
           [this, p, partBody] {
               if (!p)
                   return;
               auto modal = BSML::Lite::CreateModal(partBody, {50, 16}, nullptr, true);
               modal->moveToCenter = true;
               Input(Box(modal->get_transform(), -24, -6, 48, 5), PresetDocument::Text(*p, "name", "Part"),
                     [this, p](std::string v) { State()->document.SetText(*p, "name", v); });
               Button(Box(modal->get_transform(), -24, 1, 48, 4), "OK", [this, modal] {
                   modal->Hide();
                   BuildEditor();
               });
               modal->Show();
           },
           {.15f, .15f, .15f, 1});
    auto removePart = Button(Box(row, w - 11, 0, 4, 4), "-",
                             [this, p] {
                                 if (!p)
                                     return;
                                 State()->document.RemovePart();
                                 State()->ring = 0;
                                 BuildEditor();
                                 Preview();
                             },
                             {.7f, .15f, .25f, 1});
    if (!p) {
        removePart->set_color({.9f, .9f, .9f, .3f});
        auto hit = removePart->get_transform()->get_parent()->GetComponent<UnityEngine::UI::Image *>();
        if (hit)
            hit->set_raycastTarget(false);
    }
    Button(Box(row, w - 6, 0, 4, 4), "+",
           [this] {
               State()->document.AddPart();
               State()->ring = 0;
               BuildEditor();
               Preview();
           },
           {.15f, .6f, .25f, 1});
    // PC creates every panel before clearing the fields for an empty preset.
    // Keep the editor's shape and titles when there is no selected part.
    float gx = w + 2, mx = (w + 2) * 2, tx = (w + 2) * 3;
    if (!s->holdSabers) {
        Back(Box(root, (w + 2) * 2, 0, 40, 126), {0, 0, 0, 1});
        mx += 42;
        tx += 42;
    }
    std::string titlePrefix = p ? PresetDocument::Text(*p, "name", "Part") + " : " : "";
    Form gf{Panel(root, gx, 0, w, 126, titlePrefix + "Geometry"), w - 2};
    Form mf{Panel(root, mx, 0, w, 126, titlePrefix + "Material"), w - 2};
    Form tf{Panel(root, tx, 0, w, 126, "Trails"), w - 2};
    if (!p) {
        FitForm(cf, 26);
        FitForm(pf, 82);
        return;
    }
    auto num = [this](Form &f, J &obj, const char *key, const char *label, float def, float lo, float hi, float step,
                      int digits, Color tint = Color{1, 1, 1, 1}) {
        auto ptr = &obj;
        std::string field = key;
        f.Number(
            label, PresetDocument::Number(obj, key, def), lo, hi, step, digits,
            [this, ptr, field, digits](float v) {
                State()->document.SetNumber(*ptr, field.c_str(), v, digits == 0);
                Preview();
            },
            tint);
    };
    auto flag = [this](Form &f, J &obj, const char *key, const char *label, bool def = false, bool rebuild = false) {
        auto ptr = &obj;
        std::string field = key;
        f.Toggle(label, PresetDocument::Flag(obj, key, def), [this, ptr, field, rebuild](bool v) {
            State()->document.SetFlag(*ptr, field.c_str(), v);
            Preview();
            if (rebuild)
                BuildEditor();
        });
    };
    auto vec = [this](Form &f, J &obj, const char *key, const char *axes, float def, float lo, float hi, float step,
                      int digits) {
        auto ptr = &obj;
        std::string field = key;
        const Color colors[] = {{243.f / 255 * 1.3f, 118.f / 255 * 1.3f, 156.f / 255 * 1.3f, 1},
                                {114.f / 255 * 1.3f, 238.f / 255 * 1.3f, 145.f / 255 * 1.3f, 1},
                                {121.f / 255 * 1.3f, 167.f / 255 * 1.3f, 247.f / 255 * 1.3f, 1}};
        float sensitivity = field == "position" ? .1f : field == "rotation" ? 45.f : 1.f;
        for (size_t i = 0; i < 3; ++i)
            f.Number(
                std::string(1, axes[i]), PresetDocument::Component(obj, key, i, def), lo, hi, step, digits,
                [this, ptr, field, i, def](float v) {
                    State()->document.SetComponent(*ptr, field.c_str(), i, v, def);
                    Preview();
                },
                colors[i], sensitivity);
    };
    auto enumField = [this](Form &f, J &obj, const char *key, const char *label, std::vector<std::string> choices,
                            int def, bool rebuild) {
        int index = def;
        auto v = PresetDocument::Find(obj, key);
        if (v && v->IsInt())
            index = v->GetInt();
        else if (v && v->IsString()) {
            auto found = std::find(choices.begin(), choices.end(), v->GetString());
            if (found != choices.end())
                index = found - choices.begin();
        }
        index = std::clamp(index, 0, static_cast<int>(choices.size()) - 1);
        auto ptr = &obj;
        std::string field = key;
        f.Dropdown(label, choices[index], choices, [this, ptr, field, choices, rebuild](std::string value) {
            auto idx = std::find(choices.begin(), choices.end(), value) - choices.begin();
            State()->document.SetNumber(*ptr, field.c_str(), idx, true);
            if (field == "geometryMode" && idx == 1) {
                State()->document.EnsureAdvancedRings(*ptr);
                State()->ring = 0;
            }
            Preview();
            if (rebuild)
                BuildEditor();
        });
        return index;
    };
    row = pf.Row();
    std::vector<std::string> links{"None"};
    links.insert(links.end(), parts.begin(), parts.end());
    int linked = PresetDocument::Number(*p, "linkedPartIndex", -1);
    Dropdown(Box(row, 0, 0, w - 32, 4), "Link", linked >= 0 && linked < parts.size() ? parts[linked] : "None", links,
             [this, p, links](std::string v) {
                 int i = std::find(links.begin(), links.end(), v) - links.begin() - 1;
                 if (i == State()->document.part)
                     i = -1;
                 State()->document.SetNumber(*p, "linkedPartIndex", i, true);
                 BuildEditor();
                 Preview();
             });
    Form sideForm{Box(row, w - 30, 0, 22, 4), 22};
    enumField(sideForm, *p, "side", "Side", {"Both", "LeftOnly", "RightOnly"}, 0, false);
    Button(Box(row, w - 7, 0, 5, 4), "Dup", [this] {
        State()->document.AddPart(true);
        BuildEditor();
        Preview();
    });
    flag(pf, *p, "mirrorOnLeft", "Mirror");
    pf.Header("Position");
    vec(pf, *p, "position", "XYZ", 0, -1, 1, .00025f, 3);
    pf.Header("Rotation");
    vec(pf, *p, "rotation", "XYZ", 0, -180, 180, 1, 0);
    auto source = p;
    std::vector<bool> seen(d.Parts().Size());
    size_t sourceIndex = d.part;
    while (true) {
        seen[sourceIndex] = true;
        int next = PresetDocument::Number(*source, "linkedPartIndex", -1);
        if (next < 0 || next >= d.Parts().Size())
            break;
        if (seen[next]) {
            source = p;
            break;
        }
        sourceIndex = next;
        source = &d.Parts()[sourceIndex];
    }
    pf.Header("Geometry");
    num(pf, *source, "length", "Length", .1f, .001f, 1, .001f, 3);
    pf.Header("Animators");
    BuildAnimators(pf, *p, d, [this] { Preview(); }, [this] { BuildEditor(); });
    int geometry =
        enumField(gf, *source, "geometryMode", "Geometry Type", {"Simple", "Advanced", "Sprite", "Obj"}, 0, true);
    if (geometry == 0) {
        for (auto prefix : {"start", "end"}) {
            std::string pre = prefix;
            gf.Header(pre == "start" ? "Start Properties" : "End Properties");
            num(gf, *source, (pre + "Radius").c_str(), "Radius", .015f, .0001f, .1f, .0001f, 3);
            num(gf, *source, (pre + "Glow").c_str(), "Glow", 1, 0, 1.5f, .005f, 3);
            num(gf, *source, (pre + "Opacity").c_str(), "Opacity", 1, 0, 1, .01f, 2);
            gf.Space(2);
            vec(gf, *source, (pre + "Color").c_str(), "RGB", 1, -1, 1, .005f, 3);
            num(gf, *source, (pre + "CustomWeight").c_str(), "Custom Weight", 1, 0, 1, .005f, 3);
        }
        gf.Header("Common Properties");
        num(gf, *source, "bulgeAmount", "Bulge Amount", 0, -1, 1, .005f, 3);
        num(gf, *source, "minimumRings", "Rings", 4, 2, 10, 1, 0);
        flag(gf, *source, "inverted", "Inverted");
        flag(gf, *source, "enableEndCaps", "Use End Caps", true);
        num(gf, *source, "endCapExtension", "End Cap Extension", .25f, 0, 3, .01f, 2);
        flag(gf, *source, "enableRoundedNormals", "Rounded Normals", true);
    } else if (geometry == 1) {
        d.EnsureAdvancedRings(*source);
        auto &rings = d.Ensure(*source, "rings");
        if (!rings.IsArray())
            rings.SetArray();
        auto rlist = &rings;
        row = gf.Row();
        size_t count = rings.Size();
        if (count)
            s->ring = std::min(s->ring, count - 1);
        Button(Box(row, 0, 0, 8, 4), "<", [this] {
            if (State()->ring)
                --State()->ring;
            BuildEditor();
        });
        Text(Box(row, 8, 0, w - 36, 4), std::to_string(count ? s->ring + 1 : 0) + "/" + std::to_string(count), 3, true);
        Button(Box(row, w - 28, 0, 8, 4), ">", [this, rlist] {
            if (State()->ring + 1 < rlist->Size())
                ++State()->ring;
            BuildEditor();
        });
        Button(Box(row, w - 19, 0, 8, 4), "-",
               [this, rlist] {
                   if (rlist->Size() > 1)
                       rlist->Erase(rlist->Begin() + State()->ring);
                   State()->document.dirty = true;
                   BuildEditor();
                   Preview();
               },
               {.7f, .15f, .25f, 1});
        Button(Box(row, w - 10, 0, 8, 4), "+",
               [this, rlist] {
                   auto &d = State()->document;
                   J ring(rapidjson::kObjectType);
                   if (rlist->Size())
                       ring.CopyFrom((*rlist)[State()->ring], d.json.GetAllocator());
                   else {
                       d.SetNumber(ring, "radius", .015f);
                       d.SetNumber(ring, "position", 0);
                   }
                   d.SetNumber(ring, "position", std::clamp(PresetDocument::Number(ring, "position", 0) + .1f, 0.f, 1.f));
                   size_t insertion = std::min(State()->ring + 1, static_cast<size_t>(rlist->Size()));
                   rlist->PushBack(J(), d.json.GetAllocator());
                   for (size_t i = rlist->Size() - 1; i > insertion; --i)
                       (*rlist)[i].Swap((*rlist)[i - 1]);
                   (*rlist)[insertion].Swap(ring);
                   State()->ring = insertion;
                   BuildEditor();
                   Preview();
               },
               {.15f, .6f, .25f, 1});
        if (count) {
            auto &r = rings[s->ring];
            gf.Space(1);
            gf.Header("Ring Properties");
            num(gf, r, "position", "Position", 0, -1, 2, .001f, 3);
            num(gf, r, "radius", "Radius", .03f, .0001f, .05f, .0001f, 3);
            flag(gf, r, "inverted", "Inverted");
            gf.Header("Offset");
            num(gf, r, "offsetY", "Up", 0, -.1f, .1f, .001f, 3);
            num(gf, r, "offsetX", "Right", 0, -.1f, .1f, .001f, 3);
            num(gf, r, "uvOffset", "UV Offset", 0, -1, 1, .01f, 2);
            num(gf, r, "glow", "Glow", 1, 0, 1.5f, .005f, 3);
            num(gf, r, "opacity", "Opacity", 1, 0, 1, .01f, 2);
            gf.Space(2);
            vec(gf, r, "color", "RGB", 1, -1, 1, .005f, 3);
            num(gf, r, "customWeight", "Custom Weight", 1, 0, 1, .005f, 3);
        }
    } else if (geometry == 2) {
        gf.Header("Sprite Size");
        num(gf, *source, "spriteSizeX", "Width (X)", .2f, .001f, .5f, .001f, 3);
        num(gf, *source, "spriteSizeY", "Height (Y)", .2f, .001f, .5f, .001f, 3);
        gf.Header("Subdivisions");
        num(gf, *source, "spriteDivisionsX", "Divisions X", 1, 1, 20, 1, 0);
        num(gf, *source, "spriteDivisionsY", "Divisions Y", 1, 1, 20, 1, 0);
        flag(gf, *source, "doubleSided", "Double Sided", false);
        gf.Header("Vertex properties");
        num(gf, *source, "startGlow", "Glow", 1, 0, 1.5f, .005f, 3);
        num(gf, *source, "startOpacity", "Opacity", 1, 0, 1, .01f, 2);
        vec(gf, *source, "startColor", "RGB", 1, -1, 1, .005f, 3);
        num(gf, *source, "startCustomWeight", "Custom Weight", 1, 0, 1, .005f, 3);
    } else {
        AssetDropdown(gf.Row(), "Obj File", *source, "objFile", d, false, [this] { Preview(); });
        gf.Header("Obj Size");
        num(gf, *source, "objScale", "Scale", 1, .0001f, 1, .001f, 3);
        gf.Header("Vertex properties");
        num(gf, *source, "startGlow", "Glow", 1, 0, 1.5f, .005f, 3);
        num(gf, *source, "startOpacity", "Opacity", 1, 0, 1, .01f, 2);
        gf.Space(2);
        vec(gf, *source, "startColor", "RGB", 1, -1, 1, .005f, 3);
        num(gf, *source, "startCustomWeight", "Custom Weight", 1, 0, 1, .005f, 3);
    }
    if (geometry < 2) {
        gf.Header("Ring Verts");
        gf.Dropdown("Mode", PresetDocument::Flag(*source, "manualRingVerts", false) ? "Manual" : "Auto",
                    {"Auto", "Manual"}, [this, source](std::string v) {
                        State()->document.SetFlag(*source, "manualRingVerts", v == "Manual");
                        BuildEditor();
                        Preview();
                    });
        if (PresetDocument::Flag(*source, "manualRingVerts", false))
            num(gf, *source, "ringVertsManual", "Verts (Sides)", 20, 4, 20, 1, 0);
    }
    mf.Header("General");
    num(mf, *source, "hueShift", "Hue Shift", 0, 0, 1, .01f, 2);
    flag(mf, *source, "lit", "Use Lit Shader", false, true);
    mf.Header("Textures");
    TextureField(mf.Row(), "Color / Opacity", *source, "color", d, [this] { Preview(); });
    TextureField(mf.Row(), "Glow", *source, "glow", d, [this] { Preview(); });
    enumField(mf, *source, "textureWrap", "Wrap Mode", {"Repeat", "Clamp", "Mirror", "MirrorOnce"}, 0, false);
    mf.Header("Angle Mapping");
    for (auto key : {"rimPowerGradient", "glowAddendGradient", "opacityMultiplierGradient"}) {
        std::string label = key == std::string("rimPowerGradient")     ? "Rim"
                            : key == std::string("glowAddendGradient") ? "Glow"
                                                                       : "Opacity";
        auto line = mf.Row();
        Text(Box(line, 0, 0, (w - 2) * .5f, 4), label);
        auto &keys = d.Ensure(*source, key);
        if (!keys.IsArray())
            keys.SetArray();
        auto arr = &keys;
        float fallback = key == std::string("opacityMultiplierGradient") ? 1 : 0;
        if (key == std::string("rimPowerGradient") && keys.Empty() &&
            PresetDocument::Number(*source, "rimFactor", 0) != 0) {
            bool dirty = d.dirty;
            float factor = PresetDocument::Number(*source, "rimFactor", 0),
                  power = PresetDocument::Number(*source, "rimPower", 3);
            for (int i = 0; i < 8; i++) {
                J k(rapidjson::kObjectType);
                d.SetNumber(k, "time", i / 7.f);
                d.SetNumber(k, "value", factor * std::pow(i / 7.f, std::max(.0001f, power)));
                keys.PushBack(k, d.json.GetAllocator());
            }
            d.dirty = dirty;
        }
        GradientButton(Box(line, (w - 2) * .5f, 0, (w - 2) * .5f, 4), keys, fallback, [this, arr, fallback, line] {
            EditFloatGradient(line, *arr, fallback, State()->document, [this] { Preview(); });
        });
    }
    mf.Toggle("Perpendicular Filter", PresetDocument::Number(*source, "rimPerpendicular", 0) > .5f,
              [this, source](bool v) {
                  State()->document.SetNumber(*source, "rimPerpendicular", v ? 1 : 0);
                  Preview();
              });
    if (PresetDocument::Flag(*source, "lit", false)) {
        mf.Header("Lit Shading");
        num(mf, *source, "specularStrength", "Specular Strength", .41f, 0, 2, .01f, 2);
        num(mf, *source, "specularPower", "Specular Power", 48, 4, 512, 1, 0);
        num(mf, *source, "metallic", "Metallic", 0, 0, 1, .01f, 2);
        num(mf, *source, "smoothness", "Smoothness", 0, 0, 1, .01f, 2);
        num(mf, *source, "cubemapStrength", "Cubemap Strength", .78f, 0, 2, .01f, 2);
        num(mf, *source, "cubemapRotation", "Cubemap Rotation", 0, 0, 360, 1, 0);
        num(mf, *source, "fresnelStrength", "Fresnel Strength", .6f, 0, 2, .01f, 2);
        num(mf, *source, "fresnelPower", "Fresnel Power", 2.89f, .25f, 8, .25f, 2);
        mf.Header("Rim Color");
        vec(mf, *source, "rimColor", "RGB", 1, 0, 1, .005f, 3);
        num(mf, *source, "fresnelCustomBlend", "Custom Blend", 0, 0, 1, .01f, 2);
    }
    mf.Header("Blur");
    num(mf, *source, "blur", "Time", 1, 0, 1, .1f, 1);
    num(mf, *source, "blurFade", "Softness", 1, 0, 5, .1f, 1);
    mf.Header("Rendering");
    num(mf, *source, "depthOffset", "Depth Offset", 0, -.02f, .02f, .001f, 3);
    num(mf, *source, "renderQueueOffset", "Queue Offset", 0, -10, 10, 1, 0);
    flag(mf, *source, "disableGlowPass", "Disable Glow Pass");
    flag(mf, *source, "disableDepthPrepass", "Disable Depth Prepass");
    flag(tf, d.json, "useCustomTrails", "Use Custom Trails", false, true);
    if (PresetDocument::Flag(d.json, "useCustomTrails", false)) {
        tf.Dropdown("", s->bladeTrails ? "Blade Trails" : "Tip Trails", {"Tip Trails", "Blade Trails"},
                    [this](std::string v) {
                        State()->bladeTrails = v == "Blade Trails";
                        State()->trail = 0;
                        BuildEditor();
                    });
        auto &list = d.Ensure(d.json, s->bladeTrails ? "bladeTrails" : "tipTrails");
        if (!list.IsArray())
            list.SetArray();
        auto array = &list;
        size_t count = list.Size();
        if (count)
            s->trail = std::min(s->trail, count - 1);
        row = tf.Row();
        Button(Box(row, 0, 0, 8, 4), "<", [this] {
            if (State()->trail)
                --State()->trail;
            BuildEditor();
        });
        Text(Box(row, 8, 0, w - 36, 4), std::to_string(count ? s->trail + 1 : 0) + "/" + std::to_string(count), 3,
             true);
        Button(Box(row, w - 28, 0, 8, 4), ">", [this, array] {
            if (State()->trail + 1 < array->Size())
                ++State()->trail;
            BuildEditor();
        });
        Button(Box(row, w - 19, 0, 8, 4), "-",
               [this, array] {
                   if (array->Size())
                       array->Erase(array->Begin() + State()->trail);
                   State()->document.dirty = true;
                   BuildEditor();
                   Preview();
               },
               {.7f, .15f, .25f, 1});
        Button(Box(row, w - 10, 0, 8, 4), "+",
               [this, array] {
                   auto &d = State()->document;
                   J t(rapidjson::kObjectType);
                   const bool blade = State()->bladeTrails;
                   d.SetNumber(t, "opacity", blade ? .3f : 1.f);
                   d.SetNumber(t, "glow", 1);
                   d.SetNumber(t, "length", blade ? GetPluginConfig().bladeTrailMS : 140, true);
                   d.SetNumber(t, "width", blade ? .01f : .008f);
                   d.SetNumber(t, "motionActivation", 0);
                   d.SetNumber(t, "textureWrap", 1, true);
                   d.SetComponent(t, "position", 2, 1);
                   array->PushBack(t, d.json.GetAllocator());
                   State()->trail = array->Size() - 1;
                   BuildEditor();
                   Preview();
               },
               {.15f, .6f, .25f, 1});
        if (count) {
            auto &t = list[s->trail];
            tf.Header("Position");
            vec(tf, t, "position", "XYZ", 0, -1, 1, .001f, 3);
            tf.Header("Color");
            TrailGradientFields(tf, t, d, [this] { Preview(); });
            tf.Header("Properties");
            num(tf, t, "glow", "Glow", 1, 0, 1.5f, .005f, 3);
            num(tf, t, "opacity", "Opacity", .3f, 0, 1, .01f, 2);
            if (!s->bladeTrails)
                num(tf, t, "width", "Width", .008f, .001f, .05f, .001f, 3);
            num(tf, t, "length", "Length (ms)", 140, 0, 500, 1, 0);
            num(tf, t, "queueOffset", "Queue Offset", 0, -10, 10, 1, 0);
            num(tf, t, "depthOffset", "Depth Offset", 0, -.02f, .02f, .001f, 3);
            num(tf, t, "fade", "Fade Strength", 1, 0, 1, .01f, 2);
            num(tf, t, "motionActivation", "Motion Activation", 1, 0, 1, .01f, 2);
            if (s->bladeTrails) {
                num(tf, t, "motionFadePower", "Motion Fade Power", 0, 0, 10, .05f, 2);
                tf.Header("Noise");
                flag(tf, t, "noiseEnabled", "Noise", false, true);
                if (PresetDocument::Flag(t, "noiseEnabled", false)) {
                    num(tf, t, "noiseIntensity", "Intensity", .02f, 0, 1, .001f, 3);
                    num(tf, t, "noiseScale", "Scale", 2, 1, 10, .1f, 1);
                    num(tf, t, "noiseSpeed", "Speed", 1, 0, 10, .05f, 2);
                }
                tf.Header("Textures");
                TextureField(tf.Row(), "Color / Opacity", t, "color", d, [this] { Preview(); });
                TextureField(tf.Row(), "Glow", t, "glow", d, [this] { Preview(); });
                enumField(tf, t, "textureWrap", "Wrap Mode", {"Repeat", "Clamp", "Mirror", "MirrorOnce"}, 1, false);
            }
        }
    }
    FitForm(cf, 26);
    FitForm(pf, 82);
    FitForm(gf, 118);
    FitForm(mf, 118);
    FitForm(tf, 118);
    if (!s->status.empty())
        Text(Box(root, 0, 127, 250, 6), s->status, 2.7f, true);
}
} // namespace VainSabers
