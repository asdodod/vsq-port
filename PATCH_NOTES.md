# Patch notes

## 0.0.7 — Quest revision 12

- Fixed switching from Simple to Advanced: rings inherit the start/end properties and drive the Advanced mesh instead of leaving a Simple fallback.
- Matched PC ring insertion, minimum ring count, field order and Up/Right offsets.
- Replaced the OBJ filename input with a model selector and aligned its geometry fields with PC.
- Added blade-trail texture selectors, atlas controls and noise settings to the editor.
- Selecting a different texture or OBJ now clears the previous embedded resource so the new file takes effect.
- Added atlas controls to part materials and applied imported part atlas settings during rendering.
- Added MirrorOnce texture wrapping and support for legacy single BladeTrail JSON presets.
- Nested texture controls preserve their parent editor when a picker or keypad closes.
- Corrected linked-part editing and animator ranges; imported look-direction and glow-pass settings are respected.

## 0.0.6 — Quest revision 11

- Fixed the mod's installation phase to use late loading.
- Deferred Unity asset loading until the game is ready to use the sabers.

## 0.0.5 beta

- Matched PC defaults for new parts.
- Corrected version-dependent OBJ orientation and preserved legacy OBJ versions when saving.
- Added embedded/external blade-trail textures, atlases and animated noise.
- Fixed trail motion activation and followed song/menu saber colors.
- Fixed keypad interaction and empty-preset creation.
- Added the PC-style editor, preset import/export, blur history and Android rendering fixes.
