# Revision 10 validation

- Working ARM64 build passed with the new trail resource implementation and versioned OBJ import. The published native sources match the working sources; the separate full publication build recorded below was performed for revision 8.
- ELF64 / AArch64, Scotland2 setup/late_load exports and no direct unresolved Unity symbols. Official QMOD schema passed; the packaged library matches the build by SHA256.
- Production noise/coordinate helpers ran as WebAssembly without Unity or ADB: all 98,304 noise values matched seeded .NET Random(12345).
- The supplied Wiimote OBJ contained 779 vertices, 432 normals and 693 faces; version 2 Z reflection and version 1 preservation were checked against its coordinates. Triangle reversal matches the PC release's import rule.
- The supplied flame preset contained six embedded color/glow PNGs and four blade trails with noise. PNG signatures/dimensions and noise fields were checked. This confirms the resources exist; it does not constitute GPU image decoding/rendering validation.
- Texture binding, atlas/noise uniforms and OBJ rules were compared with PC SaberRibbonTrail/OBJLoader. Atlas/vector objects and array forms are accepted. The existing shader supports these uniforms on its CPU geometry path; no bundle changes were required.
- Newly owned trail 2D textures are destroyed on reconfiguration/OnDestroy. Seeded 3D noise is shared and created once.
- No Unity or headset connection was started. GPU texture loading, flame appearance and model orientation still need a Quest test.

Current QMOD SHA256: ECF1A33E90FD57040D3DD1829A8DF8EF93EAAD9BEC36C934EF488184DB312EED
Current library SHA256: ACA8FCD723E9A785BF38D6B7B19786CE630E088B88A13B32FE60E6E6E2C9908C

# Revision 9 validation

- New-part Length 0.100, Start/End Radius 0.030 and Blur Fade 1.000 were compared directly with PC BlurSaberData.AddComponent.
- The working ARM64 build passed. The packaged library matches that build. The publication native source matches the working native source; its separate full build below was performed for revision 8.
- The user reported that missing blade trails were disabled in their settings. Revision 9 only changes new-part defaults.
- Unity and ADB were not launched for this fix; headset validation of this latest build is still pending.

Revision 9 QMOD SHA256: 838DA5F7E8A8FB1B78E9E5920162B4DF77BB968AAA1AD69AB3AE1FF8C52C6C5F
Revision 9 library SHA256: 811D588C3AAD98BA0B84DAB2F2179B23B3EA95485F46735319A7F48046DC4491

# Revision 8 validation

Local checks completed on 2026-10-06:

- ARM64 build succeeded for both the working project and this independent source tree using the pinned, previously restored dependencies. Native source files match.
- ELF64/AArch64 with setup/late_load exports and no direct unresolved Unity engine symbols.
- QMOD library hash matches the working build. Manifest targets Beat Saber 1.40.8_7379 / Scotland2.
- Unity/OpenGLCore reproduced the old collapsed mesh failure when shader history is unavailable: zero visible pixels.
- Real ribbon geometry rendered with both production materials (vs_flatglow_2side and its supported vs_flatglow fallback), at 60/150/200 ms, for green and purple game colors: 21392–21572 visible pixels.
- A translated, rotated and scaled parent preserved each vertex world position. This verifies the desktop geometry/material path, not execution of the native Quest component.
- Empty editor behavior was compared with RebuildPanels and UpdatePartDropdown in the PC 0.0.5 release: panel titles/backgrounds remain, fields are empty, Remove is disabled and no extra hint is shown.

Earlier revision 7 checks covered motion history, number gestures, fixed-20-ms trail activation and detached keypad routing. See the included tests and QuestRevision7Check.cs; QuestRevision8Check.cs covers the revised ribbon material path.

Revision 8 QMOD SHA256: 8E60F4D71C02262DE636CCA88B7B92CFD703778BF4BBA4D01D9D29ECEA573368
Revision 8 library SHA256: 38F99F1135D843DE79A8EEE84A23F2CCD2AA006297C06953D90D8DDDBBF950F0

No headset connection was used. Unity started its SDK ADB server automatically; that specific process was stopped after validation. ADB is not running.

Before release, test default/custom blade trails, song colors, menu override changes, empty-editor/add/remove behavior, keypad and dragging on Quest. Full PC parity and Quest performance are not established by these checks. Revision 10 adds preset texture atlas/noise loading; corresponding editor controls remain incomplete, as described in README.

