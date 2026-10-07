#include "PresetDocument.hpp"
#include "PresetFilePolicy.hpp"
#include <cassert>
#include <iostream>

int main() {
    using VainSabers::PresetDocument;
    static_assert(VainSabers::IsVainSaberExport("/sdcard/VainSabers/import.vainsaber"));
    static_assert(VainSabers::IsVainSaberExport("/sdcard/VainSabers/import.VAINSABER"));
    static_assert(!VainSabers::IsVainSaberExport("/sdcard/VainSabers/import.vainsaber.json"));
    static_assert(!VainSabers::IsVainSaberExport(""));
    PresetDocument copied;
    assert(copied.Parse(R"({"version":2,"parts":[{"name":"handle","length":0.2,"customField":"keep me","animators":[{"type":"HueShiftAdder","Speed":2}]}]})"));
    copied.AddPart(true);
    assert(PresetDocument::Text(*copied.Part(), "name", "") == "handle Copy");
    assert(PresetDocument::Text(*copied.Part(), "customField", "") == "keep me");
    assert(PresetDocument::Number(*copied.Part(), "length", 0) == .2f);
    auto &copiedAnimation = (*PresetDocument::Find(*copied.Part(), "animators"))[0];
    copied.SetNumber(copiedAnimation, "Speed", 9);
    assert(PresetDocument::Number((*PresetDocument::Find(copied.Parts()[0], "animators"))[0], "Speed", 0) == 2);
    copied.AddPart(true);
    assert(PresetDocument::Text(*copied.Part(), "name", "") == "handle Copy Copy");
    PresetDocument d;
    assert(d.Parse(R"({"version":2,"parts":[{"geometryMode":"Simple","startRadius":0.024,"endRadius":0.012,"startColor":[0.2,0.3,0.4],"endColor":[0.5,0.6,0.7],"startGlow":2,"endOpacity":0.25,"inverted":true,"colorTexture":"old.png","colorTextureBase64":"old bytes","objFile":"old.obj","objBase64":"old mesh","colorAtlasCount":{"x":4,"y":8},"colorAtlasSpeedFlip":{"x":20,"y":1,"z":1}}]})"));
    auto &part = *d.Part();
    part.MemberReserve(part.MemberCount() + 64, d.json.GetAllocator());
    d.SetNumber(part, "geometryMode", 1, true);
    d.EnsureAdvancedRings(part);
    auto &rings = *PresetDocument::Find(part, "rings");
    assert(rings.Size() == 2);
    assert(PresetDocument::Number(rings[0], "position", -1) == 0);
    assert(PresetDocument::Number(rings[1], "position", -1) == 1);
    assert(PresetDocument::Number(rings[0], "radius", -1) == .024f);
    assert(PresetDocument::Number(rings[1], "radius", -1) == .012f);
    assert(PresetDocument::Number(rings[0], "glow", -1) == 2);
    assert(PresetDocument::Number(rings[1], "opacity", -1) == .25f);
    assert(PresetDocument::Flag(rings[1], "inverted", false));
    assert(PresetDocument::Component(rings[0], "color", 1, -1) == .3f);
    d.EnsureAdvancedRings(part);
    assert(rings.Size() == 2);
    // Editor writes must survive serialization independently of the Simple fields.
    d.SetNumber(rings[0], "radius", .04f);
    d.SetNumber(rings[0], "position", .2f);
    d.SetNumber(rings[0], "offsetX", .03f);
    d.SetNumber(rings[0], "offsetY", -.01f);
    d.SetNumber(rings[0], "uvOffset", .5f);
    d.SetNumber(rings[0], "opacity", .6f);
    d.SetNumber(rings[0], "glow", 1.2f);
    d.SetNumber(rings[0], "customWeight", .4f);
    d.SetComponent(rings[0], "color", 0, .7f);
    d.SetFlag(rings[0], "inverted", false);
    assert(PresetDocument::Component(part, "colorAtlasCount", 1, -1) == 8);
    d.SetComponent(part, "colorAtlasCount", 0, 6, 1);
    assert(PresetDocument::Component(part, "colorAtlasCount", 1, -1) == 8);
    d.SetComponent(part, "colorAtlasSpeedFlip", 0, 30, 1);
    assert(PresetDocument::Component(part, "colorAtlasSpeedFlip", 2, -1) == 1);
    d.SetAsset(part, "colorTexture", "new.png");
    assert(PresetDocument::Text(part, "colorTextureBase64", "wrong").empty());
    d.SetAsset(part, "objFile", "new.obj");
    assert(PresetDocument::Text(part, "objBase64", "wrong").empty());
    d.SetAsset(part, "colorTexture", "None");
    assert(PresetDocument::Text(part, "colorTexture", "wrong").empty());
    PresetDocument roundtrip;
    assert(roundtrip.Parse(d.Serialize()));
    assert(PresetDocument::Number(*roundtrip.Part(), "geometryMode", -1) == 1);
    assert(PresetDocument::Find(*roundtrip.Part(), "rings")->Size() == 2);
    auto &edited = (*PresetDocument::Find(*roundtrip.Part(), "rings"))[0];
    for (auto field : {"radius", "position", "offsetX", "offsetY", "uvOffset", "opacity", "glow", "customWeight"})
        assert(PresetDocument::Number(edited, field, -1) == PresetDocument::Number(rings[0], field, -2));
    assert(PresetDocument::Component(edited, "color", 0, -1) == .7f);
    assert(!PresetDocument::Flag(edited, "inverted", true));
    assert(PresetDocument::Number(*roundtrip.Part(), "startRadius", -1) == .024f);
    PresetDocument legacy;
    assert(legacy.Parse(R"({"parts":[],"BladeTrail":{"Glow":0.8,"ColorTexture":"fire.png"}})"));
    auto trails = PresetDocument::Find(legacy.json, "bladeTrails");
    assert(trails && trails->Size() == 1);
    assert(PresetDocument::Text((*trails)[0], "colorTexture", "") == "fire.png");
    std::cout << "Editor document regression passed: Advanced initialization, resource replacement, object atlas editing, save/load.\n";
}
