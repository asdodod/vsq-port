using System;
using System.IO;
using System.Linq;
using UnityEditor;
using UnityEngine;

// Desktop integration check for the two production material paths. This does
// not run the native Quest component or establish headset performance.
public static class QuestRevision8Check {
    static string Output => Path.GetFullPath("../build/revision8-validation");
    public static void Run() {
        Directory.CreateDirectory(Output);
        var cameraObject = new GameObject("Ribbon validation camera");
        var camera = cameraObject.AddComponent<Camera>();
        camera.orthographic = true; camera.orthographicSize = .9f;
        camera.transform.position = new Vector3(0, 0, -3);
        camera.clearFlags = CameraClearFlags.SolidColor; camera.backgroundColor = Color.black;
        camera.nearClipPlane = .01f; camera.farClipPlane = 10;
        var target = new RenderTexture(256, 256, 24);
        camera.targetTexture = target;
        var image = new Texture2D(256, 256, TextureFormat.RGB24, false);
        var report = "api=" + SystemInfo.graphicsDeviceType + "\n";
        try {
            foreach (var shaderName in new[]{"vs_flatglow_2side", "vs_flatglow"}) {
                var shader = AssetDatabase.LoadAssetAtPath<Shader>("Assets/" + shaderName + ".shader");
                if(!shader || !shader.isSupported) throw new Exception("Unsupported production shader: " + shaderName);
                foreach(int duration in new[]{60, 150, 200}) foreach(bool left in new[]{true, false}) {
                    var go = new GameObject("Real ribbon vertices");
                    var filter = go.AddComponent<MeshFilter>();
                    var renderer = go.AddComponent<MeshRenderer>();
                    go.transform.position = new Vector3(.23f, -.18f, .17f);
                    go.transform.rotation = Quaternion.Euler(14, -21, 17);
                    go.transform.localScale = Vector3.one * .73f;
                    int segments = Mathf.Clamp(duration / 6, 4, 512);
                    int count = (segments + 1) * 7;
                    var vertices = new Vector3[count]; var colors = new Color[count]; var uv = new Vector2[count];
                    var triangles = new int[segments * 6 * 6]; int ti = 0;
                    var positions = new Vector3[32]; var forwards = new Vector3[32]; var ups = new Vector3[32];
                    for(int i = 0; i < 32; ++i) {
                        positions[i] = new Vector3((left ? 1 : -1) * (.65f - 1.3f * i / 31), -.5f, 0);
                        var rotation = Quaternion.Euler(-90, 0, 12f * i / 31);
                        forwards[i] = rotation * Vector3.forward; ups[i] = rotation * Vector3.up;
                    }
                    var game = left ? new Color(.2f, .85f, .1f) : new Color(.75f, .15f, .95f);
                    var offset = new Vector3(.08f, .04f, 1);
                    for(int i = 0; i <= segments; ++i) {
                        float t = i / (float)segments, hist = t * 31;
                        int index = Mathf.Min((int)hist, 30); float fraction = hist - index;
                        var position = Vector3.Lerp(positions[index], positions[index + 1], fraction);
                        var forward = Vector3.Lerp(forwards[index], forwards[index + 1], fraction).normalized;
                        var up = Vector3.Lerp(ups[index], ups[index + 1], fraction).normalized;
                        var right = Vector3.Cross(up, forward).normalized; up = Vector3.Cross(forward, right).normalized;
                        var tip = position + right * offset.x + up * offset.y + forward * offset.z;
                        var bladeBase = Vector3.Lerp(position, tip, .01f);
                        float alpha = Mathf.LerpUnclamped(.9f, 0, t) * Mathf.Pow(t, .02f);
                        for(int v = 0; v < 7; ++v) {
                            int current = i * 7 + v;
                            var world = Vector3.Lerp(bladeBase, tip, v / 6f);
                            vertices[current] = go.transform.worldToLocalMatrix.MultiplyPoint3x4(world);
                            if(Vector3.Distance(go.transform.TransformPoint(vertices[current]), world) > .00001f)
                                throw new Exception("Moving parent transformed ribbon twice");
                            colors[current] = new Color(game.r, game.g, game.b, alpha * alpha * .3f * (v / 6f));
                            uv[current] = new Vector2(t, v / 6f);
                            if(i < segments && v < 6) {
                                triangles[ti++] = current; triangles[ti++] = current + 7; triangles[ti++] = current + 1;
                                triangles[ti++] = current + 1; triangles[ti++] = current + 7; triangles[ti++] = current + 8;
                            }
                        }
                    }
                    var mesh = new Mesh(); mesh.MarkDynamic(); mesh.vertices = vertices;
                    mesh.colors = colors; mesh.uv = uv; mesh.triangles = triangles;
                    mesh.bounds = new Bounds(Vector3.zero, Vector3.one * 100); filter.sharedMesh = mesh;
                    var material = new Material(shader); material.renderQueue = 3600;
                    material.SetShaderPassEnabled("ALPHA", false); material.SetInteger("_TrailHistCount", 0);
                    renderer.sharedMaterial = material;
                    var block = new MaterialPropertyBlock(); block.SetFloat("_TrailOpacityScale", 1);
                    renderer.SetPropertyBlock(block);
                    try {
                        int lit = Render(camera, target, image);
                        if(lit < 1000) throw new Exception("Real ribbon vertices remain invisible: " + shaderName);
                        var sample = image.GetPixels().Where(p => p.maxColorComponent > .02f).ToArray();
                        if(left && sample.Average(p => p.g) <= sample.Average(p => p.r))
                            throw new Exception("Game's green color was replaced by red");
                        if(!left && sample.Average(p => p.b) <= sample.Average(p => p.g))
                            throw new Exception("Game's purple color was replaced by blue");
                        string name = shaderName + "-" + duration + "-" + (left ? "left" : "right");
                        File.WriteAllBytes(Path.Combine(Output, name + ".png"), image.EncodeToPNG());
                        // Reproduce the old failure when history uniforms are unavailable.
                        mesh.vertices = new Vector3[count];
                        int collapsed = Render(camera, target, image);
                        if(collapsed != 0) throw new Exception("Collapsed mesh failure did not reproduce");
                        report += name + " litPixels=" + lit + " collapsedPixels=" + collapsed + "\n";
                        Debug.Log("QUEST_CPU_RIBBON_OK " + name + " lit=" + lit);
                    } finally {
                        UnityEngine.Object.DestroyImmediate(go); UnityEngine.Object.DestroyImmediate(mesh);
                        UnityEngine.Object.DestroyImmediate(material);
                    }
                }
            }
            File.WriteAllText(Path.Combine(Output, "ribbon.txt"), "QUEST_REVISION8_CHECK_OK\n" + report);
            Debug.Log("QUEST_REVISION8_CHECK_OK");
        } finally {
            RenderTexture.active = null; UnityEngine.Object.DestroyImmediate(image);
            UnityEngine.Object.DestroyImmediate(target); UnityEngine.Object.DestroyImmediate(cameraObject);
        }
    }
    static int Render(Camera camera, RenderTexture target, Texture2D image) {
        camera.Render(); RenderTexture.active = target;
        image.ReadPixels(new Rect(0, 0, 256, 256), 0, 0); image.Apply(); RenderTexture.active = null;
        return image.GetPixels().Count(p => p.maxColorComponent > .015f);
    }
}

