# Revision 9 validation

- New-part Length 0.100, Start/End Radius 0.030 and Blur Fade 1.000 were compared directly with PC BlurSaberData.AddComponent.
- The working ARM64 build passed. The packaged library matches that build. The publication native source matches the working native source; its separate full build below was performed for revision 8.
- The user reported that missing blade trails were disabled in their settings. Revision 9 only changes new-part defaults.
- Unity and ADB were not launched for this fix; headset validation of this latest build is still pending.

Current QMOD SHA256: 838DA5F7E8A8FB1B78E9E5920162B4DF77BB968AAA1AD69AB3AE1FF8C52C6C5F
Current library SHA256: 811D588C3AAD98BA0B84DAB2F2179B23B3EA95485F46735319A7F48046DC4491

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

Before release, test default/custom blade trails, song colors, menu override changes, empty-editor/add/remove behavior, keypad and dragging on Quest. Full PC parity and Quest performance are not established by these checks. Texture atlases, noise and some PC trail effects remain incomplete, as described in README.

