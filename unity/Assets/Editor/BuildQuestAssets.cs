using System;
using System.IO;
using System.Linq;
using UnityEditor;
using UnityEditor.Rendering;
using UnityEngine;
using UnityEngine.Rendering;

public static class BuildQuestAssets {
    [MenuItem("Tools/VainSabers/Build Quest AssetBundle")]
    public static void Build() {
        PlayerSettings.SetUseDefaultGraphicsAPIs(BuildTarget.Android,false);
        PlayerSettings.SetGraphicsAPIs(BuildTarget.Android,new[]{GraphicsDeviceType.OpenGLES3,GraphicsDeviceType.Vulkan});
        if(EditorUserBuildSettings.activeBuildTarget!=BuildTarget.Android &&
            !EditorUserBuildSettings.SwitchActiveBuildTarget(BuildTargetGroup.Android,BuildTarget.Android))
            throw new Exception("Cannot select Android build target");

        string output=Path.GetFullPath("../build/assetbundles");Directory.CreateDirectory(output);
        var manifest=BuildPipeline.BuildAssetBundles(output,BuildAssetBundleOptions.ForceRebuildAssetBundle |
            BuildAssetBundleOptions.ChunkBasedCompression,BuildTarget.Android);
        if(!manifest || !File.Exists(Path.Combine(output,"vs_assets")))throw new Exception("AssetBundle build failed");
        foreach(string path in AssetDatabase.GetAssetPathsFromAssetBundle("vs_assets")) {
            var shader=AssetDatabase.LoadAssetAtPath<Shader>(path);if(!shader)continue;
            var errors=ShaderUtil.GetShaderMessages(shader).Where(m=>m.severity==ShaderCompilerMessageSeverity.Error).ToArray();
            if(errors.Length>0)throw new Exception(path+": "+string.Join("; ",errors.Select(m=>m.message)));
        }
        string assets=Path.GetFullPath("../assets");Directory.CreateDirectory(assets);
        File.Copy(Path.Combine(output,"vs_assets"),Path.Combine(assets,"vs_assets"),true);
        File.Copy(Path.Combine(output,"vs_assets.manifest"),Path.Combine(assets,"vs_assets.manifest"),true);
        Debug.Log("QUEST_BUNDLE_OK: "+Path.Combine(assets,"vs_assets"));
    }
}
