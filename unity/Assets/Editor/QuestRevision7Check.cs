using System;
using System.IO;
using System.Linq;
using UnityEditor;
using UnityEditor.Rendering;
using UnityEngine;
using UnityEngine.EventSystems;
using UnityEngine.UI;

// Exercise Unity's event routing and the actual production ribbon shader.
public class VainCheckNumberField : MonoBehaviour, IPointerDownHandler, IPointerUpHandler {
    public int presses, releases;
    public Action release;
    public void OnPointerDown(PointerEventData e) { presses++; }
    public void OnPointerUp(PointerEventData e) { releases++; release?.Invoke(); }
}
public class VainCheckKey : MonoBehaviour, IPointerClickHandler {
    public int clicks;
    public void OnPointerClick(PointerEventData e) { clicks++; }
}
public static class QuestRevision7Check {
    static string Output => Path.GetFullPath("../build/revision7-validation");
    static GameObject Rect(string name, Transform parent, Vector2 size, Vector2 position) {
        var go=new GameObject(name,typeof(RectTransform));var rt=go.GetComponent<RectTransform>();rt.SetParent(parent,false);
        rt.anchorMin=rt.anchorMax=rt.pivot=new Vector2(0,1);rt.sizeDelta=size;rt.anchoredPosition=position;return go;
    }
    static void Click(GameObject target, PointerEventData e) {
        e.eligibleForClick=true;
        var press=ExecuteEvents.ExecuteHierarchy(target,e,ExecuteEvents.pointerDownHandler);
        if(!press)press=ExecuteEvents.GetEventHandler<IPointerClickHandler>(target);
        ExecuteEvents.Execute(press,e,ExecuteEvents.pointerUpHandler);
        if(target && press && press==ExecuteEvents.GetEventHandler<IPointerClickHandler>(target))
            ExecuteEvents.Execute(press,e,ExecuteEvents.pointerClickHandler);
    }
    public static void Run() {
        Directory.CreateDirectory(Output);CheckRibbon();CheckKeys();Debug.Log("QUEST_REVISION7_CHECK_OK");
    }
    public static void ExportAssetList() {
        Directory.CreateDirectory(Output);
        var assets=AssetDatabase.GetAssetPathsFromAssetBundle("vs_assets");
        var dependencies=AssetDatabase.GetDependencies(assets,true).Where(p=>p.StartsWith("Assets/") && File.Exists(p)).OrderBy(p=>p).ToArray();
        File.WriteAllLines(Path.Combine(Output,"publication-assets.txt"),dependencies);
        Debug.Log("QUEST_PUBLICATION_ASSETS_OK files="+dependencies.Length);
    }
    static void CheckKeys() {
        var root=new GameObject("Keyboard event check",typeof(RectTransform),typeof(Canvas),typeof(GraphicRaycaster));
        var canvas=root.GetComponent<Canvas>();canvas.renderMode=RenderMode.WorldSpace;canvas.sortingOrder=10;
        root.GetComponent<RectTransform>().sizeDelta=new Vector2(110,94);root.transform.localScale=Vector3.one*.01f;
        var cameraGo=new GameObject("Keyboard camera");var camera=cameraGo.AddComponent<Camera>();camera.orthographic=true;camera.orthographicSize=1;camera.transform.position=new Vector3(0,0,-2);canvas.worldCamera=camera;
        var events=new GameObject("Check EventSystem",typeof(EventSystem)).GetComponent<EventSystem>();
        var field=Rect("Number",root.transform,new Vector2(50,4),new Vector2(25,-30));var gesture=field.AddComponent<VainCheckNumberField>();
        var oldPopup=Rect("Old popup",field.transform,new Vector2(20,30),new Vector2(15,11));
        var oldKey=Rect("Old key",oldPopup.transform,new Vector2(5,4),new Vector2(2,-7));var oldClick=oldKey.AddComponent<VainCheckKey>();
        gesture.release=()=>UnityEngine.Object.DestroyImmediate(oldPopup);
        Click(oldKey,new PointerEventData(events));
        if(gesture.presses!=1 || oldKey)throw new Exception("Old nested keypad failure did not reproduce");
        gesture.release=null;gesture.presses=gesture.releases=0;
        var popup=Rect("Detached popup",field.transform,new Vector2(20,30),new Vector2(15,11));
        var before=new Vector3[4];popup.GetComponent<RectTransform>().GetWorldCorners(before);
        popup.transform.SetParent(canvas.transform,true);
        var after=new Vector3[4];popup.GetComponent<RectTransform>().GetWorldCorners(after);
        for(int i=0;i<4;i++)if(Vector3.Distance(before[i],after[i])>.00001f)throw new Exception("Popup moved after detaching");
        var popupCanvas=popup.AddComponent<Canvas>();popupCanvas.overrideSorting=true;popupCanvas.sortingOrder=20;popupCanvas.worldCamera=camera;popup.AddComponent<GraphicRaycaster>();
        var blocker=Rect("Blocker",popup.transform,new Vector2(420,430),new Vector2(-200,200));blocker.AddComponent<Image>().color=Color.clear;var blockClick=blocker.AddComponent<VainCheckKey>();blocker.transform.SetAsFirstSibling();
        var keys=new VainCheckKey[14];
        for(int i=0;i<14;i++){var key=Rect("Key "+i,popup.transform,new Vector2(5,3.9167f),new Vector2(2+i%3*5.5f,-2-(1+i/3)*4.4167f));key.AddComponent<Image>();keys[i]=key.AddComponent<VainCheckKey>();}
        Canvas.ForceUpdateCanvases();camera.Render();
        foreach(var key in keys){var corners=new Vector3[4];key.GetComponent<RectTransform>().GetWorldCorners(corners);
            Vector3 point=(corners[0]+corners[2])*.5f;var data=new PointerEventData(events);
            // VRGraphicRaycaster uses GraphicRegistry + depth and local rect
            // containment. Batch mode has no active GameView for mouse raycasts.
            var graphics=GraphicRegistry.GetGraphicsForCanvas(popupCanvas);Graphic hit=null;
            for(int i=0;i<graphics.Count;i++){var g=graphics[i];if(g.depth!=-1 && g.raycastTarget &&
                g.rectTransform.rect.Contains(g.rectTransform.InverseTransformPoint(point)) && (!hit || g.depth>hit.depth))hit=g;}
            if(!hit || hit.gameObject!=key.gameObject)throw new Exception("Key hidden by blocker or popup: "+key.name);
            Click(hit.gameObject,data);if(key.clicks!=1)throw new Exception("Key click not delivered");}
        Click(blocker,new PointerEventData(events));
        if(gesture.presses!=0 || gesture.releases!=0 || blockClick.clicks!=1)throw new Exception("Popup still triggers number gesture");
        File.WriteAllText(Path.Combine(Output,"keypad.txt"),"QUEST_KEYPAD_EVENTS_OK legacyFailureReproduced=true keys=14 blocker=true fieldGestures=0 positionPreserved=true\n");
        Debug.Log("QUEST_KEYPAD_EVENTS_OK keys=14");
        UnityEngine.Object.DestroyImmediate(root);UnityEngine.Object.DestroyImmediate(cameraGo);UnityEngine.Object.DestroyImmediate(events.gameObject);
    }
    static void CheckRibbon() {
        var shader=AssetDatabase.LoadAssetAtPath<Shader>("Assets/vs_flatglow_2side.shader");
        var errors=ShaderUtil.GetShaderMessages(shader).Where(m=>m.severity==ShaderCompilerMessageSeverity.Error).ToArray();
        if(!shader || !shader.isSupported || errors.Length>0)throw new Exception("Ribbon shader unavailable: "+string.Join(";",errors.Select(e=>e.message)));
        var go=new GameObject("Production ribbon");var renderer=go.AddComponent<MeshRenderer>();var filter=go.AddComponent<MeshFilter>();
        int segments=25,vertical=6,count=(segments+1)*(vertical+1);var mesh=new Mesh();var vertices=new Vector3[count];var uv=new Vector2[count];var colors=new Color[count];var triangles=new int[segments*vertical*6];int ti=0;
        for(int i=0;i<=segments;i++)for(int j=0;j<=vertical;j++){int index=i*(vertical+1)+j;float t=i/(float)segments,v=j/(float)vertical;
            uv[index]=new Vector2(t,v);float a=Mathf.Lerp(.9f,0,t)*Mathf.Pow(Mathf.Max(t,.001f),.02f);colors[index]=new Color(0,1,0,a*a*.3f*v);
            if(i<segments && j<vertical){int next=index+vertical+1;triangles[ti++]=index;triangles[ti++]=next;triangles[ti++]=index+1;triangles[ti++]=index+1;triangles[ti++]=next;triangles[ti++]=next+1;}}
        mesh.vertices=vertices;mesh.uv=uv;mesh.colors=colors;mesh.triangles=triangles;mesh.bounds=new Bounds(Vector3.zero,Vector3.one*100);filter.sharedMesh=mesh;
        var material=new Material(shader);material.renderQueue=3600;material.SetShaderPassEnabled("ALPHA",false);material.SetInteger("_TrailHistCount",32);material.SetFloat("_GlowBoost",1);material.SetColor("_CustomColor",Color.green);renderer.sharedMaterial=material;
        var pos=new Vector4[32];var fwd=new Vector4[32];var up=new Vector4[32];for(int i=0;i<32;i++){pos[i]=new Vector4(.6f-1.2f*i/31,-.5f,0,1);fwd[i]=new Vector4(0,1,0,0);up[i]=new Vector4(0,0,1,0);}
        var props=new MaterialPropertyBlock();props.SetVectorArray("_TrailHistPos",pos);props.SetVectorArray("_TrailHistFwd",fwd);props.SetVectorArray("_TrailHistUp",up);props.SetVector("_TrailLocalOffset",new Vector4(0,0,1,0));props.SetFloat("_TrailBaseFraction",.01f);props.SetFloat("_TrailOpacityScale",1);props.SetFloat("_TrailDuration",.15f);renderer.SetPropertyBlock(props);
        var camGo=new GameObject("Ribbon camera");var cam=camGo.AddComponent<Camera>();cam.orthographic=true;cam.orthographicSize=.8f;cam.transform.position=new Vector3(0,0,-2);cam.clearFlags=CameraClearFlags.SolidColor;cam.backgroundColor=Color.black;cam.nearClipPlane=.01f;cam.farClipPlane=10;
        var target=new RenderTexture(256,256,24);cam.targetTexture=target;var image=new Texture2D(256,256,TextureFormat.RGB24,false);
        try {cam.Render();RenderTexture.active=target;image.ReadPixels(new Rect(0,0,256,256),0,0);image.Apply();RenderTexture.active=null;int lit=image.GetPixels().Count(p=>p.g>.015f);
            File.WriteAllBytes(Path.Combine(Output,"ribbon.png"),image.EncodeToPNG());Debug.Log("QUEST_RIBBON_RENDER api="+SystemInfo.graphicsDeviceType+" litPixels="+lit);
            File.WriteAllText(Path.Combine(Output,"ribbon.txt"),"api="+SystemInfo.graphicsDeviceType+" litPixels="+lit+"\n");
            if(lit<100)throw new Exception("Production blade trail invisible");Debug.Log("QUEST_RIBBON_RENDER_OK");
        } finally {cam.targetTexture=null;target.Release();foreach(var o in new UnityEngine.Object[]{go,mesh,material,camGo,target,image})UnityEngine.Object.DestroyImmediate(o);}
    }
}

