# VainSabers Quest 0.0.6 — startup fix

The reported 0.0.5 installation crashed immediately after mod initialization with a null program counter (`pc=0`). The same native library worked when moved from Scotland2's `early_mods` directory to `mods`. The distributed manifest still listed it in `modFiles`, so the earlier intended switch to late installation was not present in the package.

## Changes

- Both the generated manifest and its QPM template now use an empty `modFiles` and put `libvainsabers.so` only in `lateModFiles`. The loader installs it into `mods`.
- `late_load` registers types, hooks and the UI without loading the AssetBundle. Existing saber and menu-pointer callbacks load assets when needed; failed loading retains the vanilla saber.
- Packaging rejects an early or duplicate library assignment. `scripts/test-load-phase.ps1` verifies the template, generated manifest and optionally the actual QMOD, including negative early/duplicate cases.
- The package and native mod identity are version 0.0.6. CMake watches `qpm.json` so future version changes regenerate compiler definitions.

## Installing the fix

Close Beat Saber. Remove the old VainSabers installation in your mod manager, then install the new `VainSabers.qmod`. Keep your presets in `/sdcard/VainSabers`; this update does not remove them. There must be no remaining `libvainsabers.so` in `Modloader/early_mods`, and only one copy in `Modloader/mods`.

## Limits of the evidence

The report does not identify the exact function behind the null call. Missing Analytics method messages are insufficient to prove a bundle incompatibility. This change corrects the demonstrated installation-phase difference and avoids creating Unity asset objects during loader initialization; it does not establish that all mod combinations are compatible.

The user who encountered the crash must retest startup on the original mod set, then menu pointers and a song using the default and affected custom presets. No headset connection or ADB operation was used to produce this fix.
