# 0.0.5 beta — Quest revision 9

- Match PC BlurSaberData.AddComponent defaults for a newly added part: Length 0.100, Start/End Radius 0.030 and Blur Fade 1.000. Existing presets and Duplicate are unchanged.
- The user reported that the missing blade trails were disabled in their settings, rather than confirming a rendering failure.

# 0.0.5 beta — Quest revision 8

- Generate reusable ribbon mesh vertices from 32 movement-history poses; select a supported material instead of leaving the mesh collapsed when GPU history does not work.
- Match PC frame-time activation decay and apply blade trail Motion Fade Power. Updating color no longer recreates the ribbon mesh.
- Follow the gameplay ColorManager and the PC menu override scheme, including preview and pointer colors. Keep preset Custom Weight behavior.
- Keep Geometry, Material and Trails panels for an empty preset. Remove the added hint, leave the part dropdown empty and disable Remove.
- Validate both production ribbon materials with real geometry and a transformed parent in Unity/OpenGLCore at 60/150/200 ms. Headset rendering and performance still require testing.
# 0.0.5 beta — Quest revision 7

- Fixed numeric keypad presses reaching the number gesture handler instead of the keys. Local popups preserve their position and close when their owning panel is rebuilt.
- Create new preset now saves an empty version 2 preset, selects it in Gameplay Saber and stays on the main settings page.
- Fixed blade trail motion activation: speed now uses a fixed 20 ms history interval, shared with tip trails, instead of underestimating movement from adjacent ribbon mesh samples.
- Formatted native sources consistently, removed the unused copy-on-create branch and generated embedded assets in the build directory.
- Added an independent source tree with build instructions, pinned game headers and source assets.

Earlier revisions added the PC-style menu/editor, local numeric input, preset import/export, interpolated blur history and the GLES depth offset correction.


