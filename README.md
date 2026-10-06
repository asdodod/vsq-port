# VainSabers Quest

Quest port of [VainSabers by Vainstains](https://github.com/Vainstains/VainSabers).
Target: **Beat Saber 1.40.8_7379**, **Scotland2**, ARM64.
Version: **0.0.5 beta**, Quest revision 9.

## Installation and presets

Install the release QMOD through your Quest mod manager. Open **Mods → VainSabers** in the gameplay setup menu.

Presets live in **`/sdcard/VainSabers`**, beside Download, Movies and Pictures in the headset's internal storage. Import `.json` or `.vainsaber` there, then press **Refresh presets**.

**Create new preset** saves an empty `NewPreset.json` and selects it under **Gameplay Saber**. Existing names get a numeric suffix. Press **Edit** to add parts. **Save** writes JSON with a backup; **Export** writes a `.vainsaber` with embedded resources and a PNG.

Tap a number to open its keypad. Hold and turn the controller horizontally to adjust it; releasing after a drag does not open the keypad.

## Build

Required: QPM, CMake 3.22+, Ninja, Android NDK r27 and PowerShell 7 for the scripts. The dependency snapshot is included in `qpm.shared.json`; the game headers are pinned to `bs-cordl 4008.0.0`.

From this directory:

```powershell
qpm restore
$env:ANDROID_NDK_HOME = 'C:/Android/ndk/r27/android-ndk-r27-windows/android-ndk-r27'
pwsh ./scripts/build.ps1
pwsh ./scripts/createqmod.ps1
```

The library is built into `build/libvainsabers.so`; the QMOD is created at the repository root. `assets/vs_assets` is the Android bundle used by the native build. CMake generates its embedded C++ byte array automatically inside `build`, so generated source is not checked in.

## AssetBundle

The `unity` folder contains the asset project for **Unity 2021.3.16f1** with Android Build Support. Run `scripts/BuildAssetBundlesQuest.bat` after changing shaders or assets; set `UNITY_PATH` if Unity is installed elsewhere. The BAT builds GLES3/Vulkan assets, checks the result and copies the bundle to `assets/vs_assets`. Rebuild the native library afterwards.

## Source layout

- `src`, `include`: native mod, rendering, presets and Quest UI.
- `tests`: compile-time regressions for motion history, number gestures and trail activation.
- `unity`: source shaders, bundle assets and local Unity regression checks.
- `cmake`, `scripts`: native build, asset embedding and packaging.

Original PC rendering and UI references come from VainSabers master commit `6f85587582b437435f5e48973ee10bd0dde4cb7f` and the author's `0.0.5-bs1.40.8-34ce50e` release. PC DLLs and decompilation output are not included here.

## Validation and limits

Revision 9 aligns new-part defaults with PC: Length 0.100, Start/End Radius 0.030 and Blur Fade 1.000.

Revision 8 generates real ribbon vertices instead of depending on GPU history arrays, follows the gameplay color manager and PC menu override colors, and retains empty editor panels without an added hint. Revision 7 fixed keypad routing and empty-preset creation.

Local checks cover ARM64 compilation, entry-point exports, the embedded bundle, number gestures and trail motion at 60–120 Hz, keypad event routing and ribbon rendering in Unity/OpenGL. They do not establish headset performance or full visual parity with PC. Revision 9 changes new-part defaults only; this latest build still needs a Quest test.

Texture atlases, noise and some additional PC trail effects are not fully ported. Some gradient/animator controls are adapted for Quest. Bloom is not part of this mod.

## Credits

- Vainstains: original VainSabers, shaders, assets, presets and PC UI.
- QuestPackageManager, Sc2ad, zoller27osu and jakibaki: beatsaber-hook and Quest tooling.
- Quest-BSML contributors: BSML UI.
- Il2CppQuestTypePatching contributors: custom-types.
- Lauriethefish, danrouse and Bobby Shmurner: Quest mod template.

Dependency source and license information is maintained in the linked upstream projects. This port does not replace the ownership or licensing of the original mod and assets.


