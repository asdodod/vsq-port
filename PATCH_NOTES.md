# ✦ Patch notes

## VainSabers 0.0.5 — Quest port version 07

- Export now writes a self-contained .vainsaber without generating a PNG preview, verifies the saved file and requests Android media scanning for USB/MTP visibility.
- The Export button now shows `Exported <preset>.vainsaber` after success, or `Export failed` on failure.
- Exported .vainsaber files are read-only in the editor; JSON presets remain editable.
- Duplicating a part preserves its source name, for example `handle` becomes `handle Copy`.
- Bake blade-trail vertex noise into the native mesh update once per frame, preserving the original seeded noise and filtering. Both eyes and glow passes reuse those vertices.
- Skip vertex uploads and rendering for fully invisible blade trails.
- Fixed switching from Simple to Advanced: rings inherit the start/end properties and drive the Advanced mesh instead of leaving a Simple fallback.
- Matched PC ring insertion, minimum ring count, field order and Up/Right offsets.
- Replaced the OBJ filename input with a model selector and aligned its geometry fields with PC.
- Added blade-trail texture selectors, atlas controls and noise settings to the editor.
- Selecting a different texture or OBJ now clears the previous embedded resource so the new file takes effect.
- Added atlas controls to part materials and applied imported part atlas settings during rendering.
- Added MirrorOnce texture wrapping and support for legacy single BladeTrail JSON presets.
- Nested texture controls preserve their parent editor when a picker or keypad closes.
- Updated linked-part editing and animator controls; imported look-direction and glow-pass settings are applied by VainSabers. Some PC behaviors still differ, and QuestBloom can override glow-pass disabling.

## Earlier Quest port builds — startup fixes

- Fixed the mod's installation phase to use late loading.
- Deferred Unity asset loading until the game is ready to use the sabers.

## Earlier Quest port builds — editor and rendering

- Matched PC defaults for new parts.
- Corrected version-dependent OBJ orientation and preserved legacy OBJ versions when saving.
- Added embedded/external blade-trail textures, atlases and animated noise.
- Fixed trail motion activation and followed song/menu saber colors.
- Fixed keypad interaction and empty-preset creation.
- Added the PC-style editor, preset import/export, blur history and Android rendering fixes.
