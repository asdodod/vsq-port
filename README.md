# VainSabers Quest

**THIS MOD WAS CREATED WITH AI.** AI was used to create and modify this Quest port.

Quest port of [VainSabers by Vainstains](https://github.com/Vainstains/VainSabers): customizable sabers, motion blur, blade and tip trails, and an in-game preset editor.

Target: **Beat Saber 1.40.8_7379**, ARM64, **Scotland2**. Source version: **0.0.7**.

## Installation

Install the QMOD with your Quest mod manager, such as ModsBeforeFriday, and restart Beat Saber. Open **Mods → VainSabers** in the gameplay setup menu. Enable VainSabers and choose a Gameplay Saber preset.

The source version may be newer than the latest published release. Download published builds from [Releases](https://github.com/asdodod/vsq-port/releases).

## Installing presets

Presets belong in **`/sdcard/VainSabers`**: the `VainSabers` folder in the headset's internal-storage root, beside Download, Movies and Pictures.

Copy `.json` or `.vainsaber` files there and press **Refresh presets**. External `.obj` models and `.png`, `.jpg` or `.jpeg` textures go in the same folder. Exported `.vainsaber` files can contain their resources.

Select a preset under **Gameplay Saber**. **Menu Display** controls menu sabers; a separate menu preset can be selected when using that mode.

## Preset editor

**Create new preset** creates an empty preset and selects it under Gameplay Saber. Press **Edit**, then **+** to add a part.

- **Part:** position, rotation, linking, side, mirroring and animators.
- **Geometry:** Simple tubes, Advanced rings, sprites or OBJ models.
- **Material:** textures, angle gradients, lit shading, blur and rendering options.
- **Trails:** custom tip/blade trails, gradients and motion controls. Blade trails also support textures, animated atlases and noise.

Tap a number for direct keypad entry. Hold it and turn the controller horizontally to change its value. For textures, select a file and use **…** to edit atlas columns, rows, speed and direction.

**Save** writes JSON with a backup. **Export** writes a `.vainsaber` with embedded resources and a PNG into `/sdcard/VainSabers`, ready to share. **Hold Sabers** switches between controller and static previews.

## Bloom and PC compatibility

Bloom is available separately through [QuestBloom](https://github.com/asdodod/QuestBloom). It is not bundled with VainSabers.

Rendering and editor behavior are based on the author's PC 0.0.5 release. Quest uses native C++ and Quest UI instead of the PC runtime. Legacy plain-text PC presets are not supported; use JSON or `.vainsaber`. Visual parity and performance depend on the headset and need testing in-game.

See [Patch notes](PATCH_NOTES.md) for changes.

## Building

Requirements: QPM, CMake 3.22+, Ninja, Android NDK r27 and PowerShell 7.

```powershell
qpm restore
$env:ANDROID_NDK_HOME = 'C:/Android/ndk/r27/android-ndk-r27-windows/android-ndk-r27'
pwsh ./scripts/build.ps1
pwsh ./scripts/createqmod.ps1
```

The library is built into `build/libvainsabers.so`; the QMOD is created at the repository root. Dependencies are pinned in `qpm.shared.json`.

### Changing the AssetBundle

The `unity` folder targets **Unity 2021.3.16f1** with Android Build Support. After changing shaders or assets, run **`scripts/BuildAssetBundlesQuest.bat`**; set `UNITY_PATH` if necessary. The BAT copies the Android bundle into `assets/vs_assets`. Rebuild the native library afterwards.

## Credits

- **Vainstains:** original VainSabers, shaders, assets, presets and PC UI.
- QuestPackageManager, beatsaber-hook, custom-types, bs-cordl, Scotland2 and Quest-BSML contributors: Quest tooling and UI.
- Lauriethefish, danrouse and Bobby Shmurner: Quest mod template.

The original mod and assets retain their authorship and licensing.
