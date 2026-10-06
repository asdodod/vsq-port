#include "DefaultPresets.hpp"
#include <algorithm>

namespace VainSabers {

static constexpr std::string_view s_default_json = R"PRESET({
  "version": 1,
  "parts": [
    {
      "name": "Blade",
      "position": [
        0,
        0,
        0
      ],
      "rotation": [
        0,
        0,
        0
      ],
      "length": 1,
      "geometryMode": "Simple",
      "hueShift": 0,
      "startRadius": 0.008,
      "startColor": [
        1,
        1,
        1
      ],
      "startCustomWeight": 1,
      "startGlow": 0.768432,
      "startOpacity": 1,
      "endRadius": 0.003,
      "endColor": [
        -0.461275,
        -0.507008,
        -0.499476
      ],
      "endCustomWeight": 0.83598,
      "endGlow": 0.470669,
      "endOpacity": 1,
      "inverted": false,
      "lit": false,
      "blur": 1,
      "blurFade": 1.765374,
      "enableEndCaps": true,
      "enableRoundedNormals": true,
      "endCapExtension": 0.814861,
      "lookDir": [
        0,
        0,
        0
      ],
      "useLookDir": false,
      "bulgeAmount": 0,
      "minimumRings": 9,
      "renderQueueOffset": 0,
      "depthOffset": 0
    },
    {
      "name": "Handle",
      "position": [
        0,
        0,
        0
      ],
      "rotation": [
        180,
        0,
        0
      ],
      "length": 0.12641,
      "geometryMode": "Simple",
      "hueShift": -0.05,
      "startRadius": 0.011,
      "startColor": [
        -0.042865,
        -0.095416,
        -0.014959
      ],
      "startCustomWeight": 0.122283,
      "startGlow": 0,
      "startOpacity": 1,
      "endRadius": 0.009,
      "endColor": [
        0.03796,
        0.049346,
        0.02355
      ],
      "endCustomWeight": 0.192428,
      "endGlow": 0,
      "endOpacity": 1,
      "inverted": false,
      "lit": false,
      "blur": 1,
      "blurFade": 0,
      "enableEndCaps": false,
      "enableRoundedNormals": true,
      "endCapExtension": 0.25,
      "lookDir": [
        0,
        0,
        0
      ],
      "useLookDir": false,
      "bulgeAmount": -0.2,
      "minimumRings": 9,
      "renderQueueOffset": 6,
      "depthOffset": 0
    },
    {
      "name": "BladeBase",
      "position": [
        0,
        0,
        0
      ],
      "rotation": [
        0,
        0,
        0
      ],
      "length": 0.027072,
      "geometryMode": "Simple",
      "hueShift": 0,
      "startRadius": 0.011,
      "startColor": [
        -0.072328,
        -0.080579,
        -0.142241
      ],
      "startCustomWeight": 0.573277,
      "startGlow": 0.729399,
      "startOpacity": 1,
      "endRadius": 0.008,
      "endColor": [
        -0.194872,
        -0.128054,
        -0.121079
      ],
      "endCustomWeight": 1,
      "endGlow": 0.766862,
      "endOpacity": 1,
      "inverted": false,
      "lit": false,
      "blur": 1,
      "blurFade": 5,
      "enableEndCaps": true,
      "enableRoundedNormals": true,
      "endCapExtension": 0.287349,
      "lookDir": [
        0,
        0,
        0
      ],
      "useLookDir": false,
      "bulgeAmount": -0.1,
      "minimumRings": 4,
      "renderQueueOffset": 0,
      "depthOffset": 0
    },
    {
      "name": "Pommel",
      "position": [
        0,
        0,
        -0.145
      ],
      "rotation": [
        0,
        0,
        0
      ],
      "length": 0.018585,
      "geometryMode": "Simple",
      "hueShift": 0,
      "startRadius": 0.005,
      "startColor": [
        1,
        1,
        1
      ],
      "startCustomWeight": 1,
      "startGlow": 0.197715,
      "startOpacity": 1,
      "endRadius": 0.01,
      "endColor": [
        1,
        1,
        1
      ],
      "endCustomWeight": 1,
      "endGlow": 1,
      "endOpacity": 1,
      "inverted": false,
      "lit": false,
      "blur": 1,
      "blurFade": 1,
      "enableEndCaps": true,
      "enableRoundedNormals": true,
      "endCapExtension": 0,
      "lookDir": [
        0,
        0,
        0
      ],
      "useLookDir": false,
      "bulgeAmount": 0.2,
      "minimumRings": 3,
      "renderQueueOffset": 0,
      "depthOffset": 0
    }
  ]
})PRESET";

static constexpr std::string_view s_menupointer_dot_json = R"PRESET({
  "Version": 2,
  "Parts": [
    {
      "Name": "Part 1",
      "Position": [
        0.0,
        0.0,
        0.0
      ],
      "Rotation": [
        0.0,
        0.0,
        0.0
      ],
      "LinkedPartIndex": -1,
      "Length": 0.025,
      "GeometryMode": 0,
      "SpriteSizeX": 0.05,
      "SpriteSizeY": 0.05,
      "SpriteDivisionsX": 4,
      "SpriteDivisionsY": 4,
      "DoubleSided": false,
      "HueShift": 0.0,
      "StartRadius": 0.008,
      "StartColor": [
        0.0,
        0.199999988,
        1.0
      ],
      "StartCustomWeight": 0.0,
      "StartGlow": 0.7,
      "StartOpacity": 1.0,
      "EndRadius": 0.008,
      "EndColor": [
        0.0,
        0.199999988,
        1.0
      ],
      "EndCustomWeight": 0.0,
      "EndGlow": 0.7,
      "EndOpacity": 1.0,
      "Inverted": false,
      "Lit": false,
      "Blur": 1.0,
      "BlurFade": 0.1,
      "EnableEndCaps": true,
      "EnableRoundedNormals": true,
      "ManualRingVerts": false,
      "RingVertsManual": 20,
      "Side": 0,
      "MirrorOnLeft": false,
      "EndCapExtension": 1.15,
      "LookDir": [
        0.0,
        0.0,
        0.0
      ],
      "UseLookDir": false,
      "BulgeAmount": 1.0,
      "MinimumRings": 5,
      "RenderQueueOffset": 0,
      "DepthOffset": 0.0,
      "DisableGlowPass": false,
      "DisableDepthPrepass": false,
      "RimFactor": 0.0,
      "RimPower": 3.0,
      "RimPowerGradient": [
        {
          "Time": 0.0,
          "Value": 0.0,
          "Easing": 0
        },
        {
          "Time": 0.142857149,
          "Value": 0.0,
          "Easing": 0
        },
        {
          "Time": 0.2857143,
          "Value": 0.0,
          "Easing": 0
        },
        {
          "Time": 0.428571433,
          "Value": 0.0,
          "Easing": 0
        },
        {
          "Time": 0.5714286,
          "Value": 0.0,
          "Easing": 0
        },
        {
          "Time": 0.714285731,
          "Value": 0.0,
          "Easing": 0
        },
        {
          "Time": 0.857142866,
          "Value": 0.0,
          "Easing": 0
        },
        {
          "Time": 1.0,
          "Value": 0.0,
          "Easing": 0
        }
      ],
      "RimPerpendicular": 0.0,
      "SpecularStrength": 0.41,
      "SpecularPower": 48.0,
      "Metallic": 0.0,
      "Smoothness": 0.0,
      "CubemapStrength": 0.78,
      "CubemapRotation": 0.0,
      "FresnelStrength": 0.6,
      "FresnelPower": 2.89,
      "FresnelCustomBlend": 0.0,
      "RimColor": [
        0.47,
        0.51,
        0.57
      ],
      "ColorTexture": null,
      "GlowTexture": null,
      "ColorTextureBase64": null,
      "GlowTextureBase64": null,
      "TextureWrap": 1,
      "ColorAtlasCount": {
        "x": 1.0,
        "y": 1.0,
        "normalized": {
          "x": 0.707106769,
          "y": 0.707106769,
          "magnitude": 1.0,
          "sqrMagnitude": 0.99999994
        },
        "magnitude": 1.41421354,
        "sqrMagnitude": 2.0
      },
      "ColorAtlasSpeedFlip": {
        "x": 1.0,
        "y": 0.0,
        "z": 0.0,
        "magnitude": 1.0,
        "sqrMagnitude": 1.0
      },
      "GlowAtlasCount": {
        "x": 1.0,
        "y": 1.0,
        "normalized": {
          "x": 0.707106769,
          "y": 0.707106769,
          "magnitude": 1.0,
          "sqrMagnitude": 0.99999994
        },
        "magnitude": 1.41421354,
        "sqrMagnitude": 2.0
      },
      "GlowAtlasSpeedFlip": {
        "x": 1.0,
        "y": 0.0,
        "z": 0.0,
        "magnitude": 1.0,
        "sqrMagnitude": 1.0
      },
      "ObjFile": null,
      "ObjBase64": null,
      "ObjScale": 1.0,
      "Animators": null,
      "Rings": null
    }
  ],
  "UseCustomTrails": true,
  "TipTrails": [
    {
      "Position": [
        0.0,
        0.0,
        1.0
      ],
      "Color": [
        1.0,
        1.0,
        1.0
      ],
      "CustomBlend": 1.0,
      "ColorGradient": [
        {
          "Time": 0.0,
          "Color": [
            1.0,
            1.0,
            1.0
          ],
          "Easing": 0
        },
        {
          "Time": 1.0,
          "Color": [
            1.0,
            1.0,
            1.0
          ],
          "Easing": 0
        }
      ],
      "CustomBlendGradient": [
        {
          "Time": 0.0,
          "Value": 1.0,
          "Easing": 0
        },
        {
          "Time": 1.0,
          "Value": 1.0,
          "Easing": 0
        }
      ],
      "Glow": 1.0,
      "Opacity": 0.0,
      "Width": 0.008,
      "Length": 0,
      "QueueOffset": 0,
      "DepthOffset": 0.0,
      "Fade": 1.0,
      "ColorTexture": null,
      "GlowTexture": null,
      "ColorTextureBase64": null,
      "GlowTextureBase64": null,
      "TextureWrap": 1,
      "MotionActivation": 1.0,
      "NoiseEnabled": false,
      "NoiseIntensity": 0.02,
      "NoiseScale": 2.0,
      "NoiseSpeed": 1.0,
      "MotionFadePower": 0.0,
      "ColorAtlasCount": {
        "x": 1.0,
        "y": 1.0,
        "normalized": {
          "x": 0.707106769,
          "y": 0.707106769,
          "magnitude": 1.0,
          "sqrMagnitude": 0.99999994
        },
        "magnitude": 1.41421354,
        "sqrMagnitude": 2.0
      },
      "ColorAtlasSpeedFlip": {
        "x": 1.0,
        "y": 0.0,
        "z": 0.0,
        "magnitude": 1.0,
        "sqrMagnitude": 1.0
      },
      "GlowAtlasCount": {
        "x": 1.0,
        "y": 1.0,
        "normalized": {
          "x": 0.707106769,
          "y": 0.707106769,
          "magnitude": 1.0,
          "sqrMagnitude": 0.99999994
        },
        "magnitude": 1.41421354,
        "sqrMagnitude": 2.0
      },
      "GlowAtlasSpeedFlip": {
        "x": 1.0,
        "y": 0.0,
        "z": 0.0,
        "magnitude": 1.0,
        "sqrMagnitude": 1.0
      }
    }
  ],
  "BladeTrail": null,
  "BladeTrails": [
    {
      "Position": [
        0.0,
        0.0,
        1.0
      ],
      "Color": [
        1.0,
        1.0,
        1.0
      ],
      "CustomBlend": 1.0,
      "ColorGradient": [
        {
          "Time": 0.0,
          "Color": [
            1.0,
            1.0,
            1.0
          ],
          "Easing": 0
        },
        {
          "Time": 1.0,
          "Color": [
            1.0,
            1.0,
            1.0
          ],
          "Easing": 0
        }
      ],
      "CustomBlendGradient": [
        {
          "Time": 0.0,
          "Value": 1.0,
          "Easing": 0
        },
        {
          "Time": 1.0,
          "Value": 1.0,
          "Easing": 0
        }
      ],
      "Glow": 1.0,
      "Opacity": 0.0,
      "Width": 0.01,
      "Length": 0,
      "QueueOffset": 0,
      "DepthOffset": 0.0,
      "Fade": 1.0,
      "ColorTexture": null,
      "GlowTexture": null,
      "ColorTextureBase64": null,
      "GlowTextureBase64": null,
      "TextureWrap": 1,
      "MotionActivation": 1.0,
      "NoiseEnabled": false,
      "NoiseIntensity": 0.02,
      "NoiseScale": 2.0,
      "NoiseSpeed": 1.0,
      "MotionFadePower": 0.0,
      "ColorAtlasCount": {
        "x": 1.0,
        "y": 1.0,
        "normalized": {
          "x": 0.707106769,
          "y": 0.707106769,
          "magnitude": 1.0,
          "sqrMagnitude": 0.99999994
        },
        "magnitude": 1.41421354,
        "sqrMagnitude": 2.0
      },
      "ColorAtlasSpeedFlip": {
        "x": 1.0,
        "y": 0.0,
        "z": 0.0,
        "magnitude": 1.0,
        "sqrMagnitude": 1.0
      },
      "GlowAtlasCount": {
        "x": 1.0,
        "y": 1.0,
        "normalized": {
          "x": 0.707106769,
          "y": 0.707106769,
          "magnitude": 1.0,
          "sqrMagnitude": 0.99999994
        },
        "magnitude": 1.41421354,
        "sqrMagnitude": 2.0
      },
      "GlowAtlasSpeedFlip": {
        "x": 1.0,
        "y": 0.0,
        "z": 0.0,
        "magnitude": 1.0,
        "sqrMagnitude": 1.0
      }
    }
  ]
})PRESET";

static constexpr std::string_view s_vain_ring_json = R"PRESET({
  "version": 1,
  "parts": [
    {
      "name": "outerRing",
      "position": [
        0,
        0,
        -0.005
      ],
      "rotation": [
        -5E-06,
        180,
        180
      ],
      "length": 0.002345,
      "geometryMode": "Simple",
      "hueShift": 0,
      "startRadius": 0.024,
      "startColor": [
        -0.008575,
        -0.002451,
        0.005506
      ],
      "startCustomWeight": 1,
      "startGlow": 0.797432,
      "startOpacity": 1,
      "endRadius": 0.024,
      "endColor": [
        0.007375,
        -0.003658,
        0.009392
      ],
      "endCustomWeight": 1,
      "endGlow": 0.803775,
      "endOpacity": 1,
      "inverted": true,
      "lit": false,
      "blur": 1,
      "blurFade": 5,
      "enableEndCaps": false,
      "enableRoundedNormals": true,
      "endCapExtension": 0.25,
      "lookDir": [
        0,
        0,
        0
      ],
      "useLookDir": false,
      "bulgeAmount": -0.1,
      "minimumRings": 4,
      "renderQueueOffset": 0,
      "depthOffset": 0
    },
    {
      "name": "innerRing",
      "position": [
        0,
        0,
        -0.01
      ],
      "rotation": [
        0,
        0,
        0
      ],
      "length": 0.007448,
      "geometryMode": "Simple",
      "hueShift": 0,
      "startRadius": 0.026,
      "startColor": [
        1,
        1,
        1
      ],
      "startCustomWeight": 1,
      "startGlow": 0.759467,
      "startOpacity": 1,
      "endRadius": 0.026,
      "endColor": [
        1,
        1,
        1
      ],
      "endCustomWeight": 1,
      "endGlow": 0.689318,
      "endOpacity": 1,
      "inverted": false,
      "lit": false,
      "blur": 1,
      "blurFade": 1,
      "enableEndCaps": false,
      "enableRoundedNormals": true,
      "endCapExtension": 0.25,
      "lookDir": [
        0,
        0,
        0
      ],
      "useLookDir": false,
      "bulgeAmount": 0.1,
      "minimumRings": 4,
      "renderQueueOffset": 0,
      "depthOffset": 0
    },
    {
      "name": "inner(er)Ring",
      "position": [
        0,
        0,
        -0.01
      ],
      "rotation": [
        0,
        0,
        0
      ],
      "length": 0.007413,
      "geometryMode": "Simple",
      "hueShift": 0,
      "startRadius": 0.026,
      "startColor": [
        1,
        1,
        1
      ],
      "startCustomWeight": 1,
      "startGlow": 0,
      "startOpacity": 1,
      "endRadius": 0.026,
      "endColor": [
        1,
        1,
        1
      ],
      "endCustomWeight": 1,
      "endGlow": 0.002265,
      "endOpacity": 1,
      "inverted": true,
      "lit": false,
      "blur": 1,
      "blurFade": 1,
      "enableEndCaps": false,
      "enableRoundedNormals": true,
      "endCapExtension": 0.25,
      "lookDir": [
        0,
        0,
        0
      ],
      "useLookDir": false,
      "bulgeAmount": -0.1,
      "minimumRings": 3,
      "renderQueueOffset": 0,
      "depthOffset": 0
    },
    {
      "name": "Part 5",
      "position": [
        0,
        0,
        -0.02
      ],
      "rotation": [
        0,
        0,
        0
      ],
      "length": 0.033627,
      "geometryMode": "Simple",
      "hueShift": 0,
      "startRadius": 0.01,
      "startColor": [
        0.102138,
        0.106815,
        0.099575
      ],
      "startCustomWeight": 0.696183,
      "startGlow": 0.142042,
      "startOpacity": 1,
      "endRadius": 0.008,
      "endColor": [
        1,
        1,
        1
      ],
      "endCustomWeight": 0.851403,
      "endGlow": 0.895009,
      "endOpacity": 1,
      "inverted": false,
      "lit": false,
      "blur": 1,
      "blurFade": 5,
      "enableEndCaps": false,
      "enableRoundedNormals": true,
      "endCapExtension": 1,
      "lookDir": [
        0,
        0,
        0
      ],
      "useLookDir": false,
      "bulgeAmount": -0.4,
      "minimumRings": 4,
      "renderQueueOffset": 4,
      "depthOffset": 0
    },
    {
      "name": "Part 6",
      "position": [
        0,
        0,
        -0.02
      ],
      "rotation": [
        -5E-06,
        180,
        180
      ],
      "length": 0.102533,
      "geometryMode": "Simple",
      "hueShift": -0.275,
      "startRadius": 0.009,
      "startColor": [
        -1,
        -1,
        -1
      ],
      "startCustomWeight": 1,
      "startGlow": 0.610638,
      "startOpacity": 1,
      "endRadius": 0.009,
      "endColor": [
        -1,
        -1,
        -1
      ],
      "endCustomWeight": 0.783401,
      "endGlow": 0.442656,
      "endOpacity": 1,
      "inverted": true,
      "lit": false,
      "blur": 1,
      "blurFade": 3.668335,
      "enableEndCaps": false,
      "enableRoundedNormals": true,
      "endCapExtension": 0.25,
      "lookDir": [
        0,
        0,
        0
      ],
      "useLookDir": false,
      "bulgeAmount": -0.1,
      "minimumRings": 4,
      "renderQueueOffset": -4,
      "depthOffset": 0
    },
    {
      "name": "Part 7",
      "position": [
        0,
        0,
        -0.02
      ],
      "rotation": [
        0,
        0,
        0
      ],
      "length": 0.003743,
      "geometryMode": "Simple",
      "hueShift": 0,
      "startRadius": 0.011,
      "startColor": [
        1,
        1,
        1
      ],
      "startCustomWeight": 1,
      "startGlow": 0.390418,
      "startOpacity": 1,
      "endRadius": 0.011,
      "endColor": [
        1,
        1,
        1
      ],
      "endCustomWeight": 1,
      "endGlow": 0.128967,
      "endOpacity": 1,
      "inverted": false,
      "lit": false,
      "blur": 1,
      "blurFade": 1,
      "enableEndCaps": true,
      "enableRoundedNormals": true,
      "endCapExtension": 0,
      "lookDir": [
        0,
        0,
        0
      ],
      "useLookDir": false,
      "bulgeAmount": 0,
      "minimumRings": 4,
      "renderQueueOffset": 0,
      "depthOffset": 0
    },
    {
      "name": "Part 8",
      "position": [
        0,
        0,
        -0.12
      ],
      "rotation": [
        -5E-06,
        180,
        180
      ],
      "length": 0.017594,
      "geometryMode": "Simple",
      "hueShift": 0,
      "startRadius": 0.01,
      "startColor": [
        1,
        1,
        1
      ],
      "startCustomWeight": 1,
      "startGlow": 0.553106,
      "startOpacity": 1,
      "endRadius": 0.006,
      "endColor": [
        1,
        1,
        1
      ],
      "endCustomWeight": 1,
      "endGlow": 0.239193,
      "endOpacity": 1,
      "inverted": false,
      "lit": false,
      "blur": 1,
      "blurFade": 1,
      "enableEndCaps": true,
      "enableRoundedNormals": true,
      "endCapExtension": 0.25,
      "lookDir": [
        0,
        0,
        0
      ],
      "useLookDir": false,
      "bulgeAmount": 0.2,
      "minimumRings": 4,
      "renderQueueOffset": 0,
      "depthOffset": 0
    },
    {
      "name": "blade1",
      "position": [
        0,
        0,
        0.01
      ],
      "rotation": [
        0,
        0,
        0
      ],
      "length": 0.02702,
      "geometryMode": "Simple",
      "hueShift": 0,
      "startRadius": 0.007,
      "startColor": [
        1,
        1,
        1
      ],
      "startCustomWeight": 1,
      "startGlow": 1.068865,
      "startOpacity": 1,
      "endRadius": 0.008,
      "endColor": [
        1,
        1,
        1
      ],
      "endCustomWeight": 1,
      "endGlow": 0.964281,
      "endOpacity": 1,
      "inverted": false,
      "lit": false,
      "blur": 1,
      "blurFade": 5,
      "enableEndCaps": true,
      "enableRoundedNormals": true,
      "endCapExtension": 0.25,
      "lookDir": [
        0,
        0,
        0
      ],
      "useLookDir": false,
      "bulgeAmount": 0.5,
      "minimumRings": 3,
      "renderQueueOffset": 0,
      "depthOffset": 0
    },
    {
      "name": "Part 9",
      "position": [
        0,
        0,
        0.02
      ],
      "rotation": [
        0,
        0,
        0
      ],
      "length": 0.976705,
      "geometryMode": "Simple",
      "hueShift": 0,
      "startRadius": 0.009,
      "startColor": [
        1,
        1,
        1
      ],
      "startCustomWeight": 1,
      "startGlow": 1,
      "startOpacity": 1,
      "endRadius": 0.003,
      "endColor": [
        1,
        1,
        1
      ],
      "endCustomWeight": 1,
      "endGlow": 0.300583,
      "endOpacity": 1,
      "inverted": false,
      "lit": false,
      "blur": 1,
      "blurFade": 5,
      "enableEndCaps": true,
      "enableRoundedNormals": true,
      "endCapExtension": 0.25,
      "lookDir": [
        0,
        0,
        0
      ],
      "useLookDir": false,
      "bulgeAmount": -0.4,
      "minimumRings": 4,
      "renderQueueOffset": 0,
      "depthOffset": 0
    },
    {
      "name": "Part 10",
      "position": [
        0,
        0,
        -0.12
      ],
      "rotation": [
        0,
        0,
        0
      ],
      "length": 0.1,
      "geometryMode": "Simple",
      "hueShift": -0.275,
      "startRadius": 0.008,
      "startColor": [
        -0.003706,
        -0.00219,
        0.023983
      ],
      "startCustomWeight": 0.35195,
      "startGlow": 0,
      "startOpacity": 1,
      "endRadius": 0.008,
      "endColor": [
        0.076078,
        0.074017,
        0.054427
      ],
      "endCustomWeight": 0.263143,
      "endGlow": 0,
      "endOpacity": 1,
      "inverted": false,
      "lit": false,
      "blur": 1,
      "blurFade": 5,
      "enableEndCaps": true,
      "enableRoundedNormals": true,
      "endCapExtension": 1,
      "lookDir": [
        0,
        0,
        0
      ],
      "useLookDir": false,
      "bulgeAmount": -0.5,
      "minimumRings": 5,
      "renderQueueOffset": 0,
      "depthOffset": 0
    }
  ]
})PRESET";

static constexpr std::string_view s_vain_seg_bi_json = R"PRESET({
  "version": 1,
  "parts": [
    {
      "name": "PommelOutline",
      "position": [
        0,
        0,
        -0.14
      ],
      "rotation": [
        0,
        180,
        0
      ],
      "length": 0.018551,
      "geometryMode": "Simple",
      "hueShift": 0,
      "startRadius": 0.011,
      "startColor": [
        -1,
        -1,
        -1
      ],
      "startCustomWeight": 0.959445,
      "startGlow": 0.367546,
      "startOpacity": 1,
      "endRadius": 0.008,
      "endColor": [
        -1,
        -1,
        -1
      ],
      "endCustomWeight": 0.908827,
      "endGlow": 0.328249,
      "endOpacity": 1,
      "inverted": false,
      "lit": false,
      "blur": 1,
      "blurFade": 0.61606,
      "enableEndCaps": true,
      "enableRoundedNormals": true,
      "endCapExtension": 1,
      "lookDir": [
        0,
        0,
        0
      ],
      "useLookDir": false,
      "bulgeAmount": 0,
      "minimumRings": 2,
      "renderQueueOffset": 0,
      "depthOffset": 0
    },
    {
      "name": "BladeOutline",
      "position": [
        0,
        0,
        0.01
      ],
      "rotation": [
        0,
        0,
        0
      ],
      "length": 0.989247,
      "geometryMode": "Simple",
      "hueShift": 0,
      "startRadius": 0.01,
      "startColor": [
        -1,
        -1,
        -1
      ],
      "startCustomWeight": 0.938699,
      "startGlow": 0.269448,
      "startOpacity": 1,
      "endRadius": 0.004,
      "endColor": [
        -1,
        -1,
        -1
      ],
      "endCustomWeight": 0.896411,
      "endGlow": 0.964438,
      "endOpacity": 1,
      "inverted": false,
      "lit": false,
      "blur": 1,
      "blurFade": 4.586564,
      "enableEndCaps": true,
      "enableRoundedNormals": true,
      "endCapExtension": 3,
      "lookDir": [
        0,
        0,
        0
      ],
      "useLookDir": false,
      "bulgeAmount": -0.3,
      "minimumRings": 2,
      "renderQueueOffset": 0,
      "depthOffset": 0
    },
    {
      "name": "Lower",
      "position": [
        0,
        0,
        -0.0067
      ],
      "rotation": [
        0,
        180,
        0
      ],
      "length": 0.114128,
      "geometryMode": "Simple",
      "hueShift": 0,
      "startRadius": 0.01,
      "startColor": [
        0.0755,
        0.0755,
        1
      ],
      "startCustomWeight": 0,
      "startGlow": 0.434426,
      "startOpacity": 1,
      "endRadius": 0.008,
      "endColor": [
        0.825979,
        0.068608,
        0.323393
      ],
      "endCustomWeight": 0,
      "endGlow": 0.221611,
      "endOpacity": 1,
      "inverted": true,
      "lit": false,
      "blur": 1,
      "blurFade": 3.557846,
      "enableEndCaps": true,
      "enableRoundedNormals": true,
      "endCapExtension": 1,
      "lookDir": [
        0,
        0,
        0
      ],
      "useLookDir": false,
      "bulgeAmount": 0,
      "minimumRings": 8,
      "renderQueueOffset": 0,
      "depthOffset": 0
    },
    {
      "name": "Part 4",
      "position": [
        0,
        0,
        -0.115
      ],
      "rotation": [
        0,
        0,
        0
      ],
      "length": 0.103319,
      "geometryMode": "Simple",
      "hueShift": 0,
      "startRadius": 0.005,
      "startColor": [
        0.28998,
        -0.04978,
        0.145245
      ],
      "startCustomWeight": 0,
      "startGlow": 0,
      "startOpacity": 1,
      "endRadius": 0.006,
      "endColor": [
        -0.030443,
        -0.012828,
        0.33073
      ],
      "endCustomWeight": 0,
      "endGlow": 0,
      "endOpacity": 1,
      "inverted": false,
      "lit": false,
      "blur": 1,
      "blurFade": 3.052174,
      "enableEndCaps": true,
      "enableRoundedNormals": true,
      "endCapExtension": 1,
      "lookDir": [
        0,
        0,
        0
      ],
      "useLookDir": false,
      "bulgeAmount": 0,
      "minimumRings": 9,
      "renderQueueOffset": 0,
      "depthOffset": 0
    }
  ]
})PRESET";

static constexpr std::string_view s_vain_seg_inv_json = R"PRESET({
  "version": 1,
  "parts": [
    {
      "name": "PommelOutline",
      "position": [
        0,
        0,
        -0.122
      ],
      "rotation": [
        0,
        180,
        0
      ],
      "length": 0.023267,
      "geometryMode": "Simple",
      "hueShift": 0,
      "startRadius": 0.013,
      "startColor": [
        -0.260836,
        -0.262381,
        -0.252242
      ],
      "startCustomWeight": 0.874232,
      "startGlow": 0.657785,
      "startOpacity": 1,
      "endRadius": 0.008,
      "endColor": [
        -0.21673,
        -0.239781,
        -0.210086
      ],
      "endCustomWeight": 0.823074,
      "endGlow": 0.590131,
      "endOpacity": 1,
      "inverted": true,
      "lit": false,
      "blur": 1,
      "blurFade": 5,
      "enableEndCaps": true,
      "enableRoundedNormals": true,
      "endCapExtension": 1,
      "lookDir": [
        0,
        0,
        0
      ],
      "useLookDir": false,
      "bulgeAmount": 0.1,
      "minimumRings": 5,
      "renderQueueOffset": 0,
      "depthOffset": 0
    },
    {
      "name": "PommelCore",
      "position": [
        0,
        0,
        -0.125
      ],
      "rotation": [
        0,
        180,
        0
      ],
      "length": 0.017843,
      "geometryMode": "Simple",
      "hueShift": 0,
      "startRadius": 0.009,
      "startColor": [
        -0.3,
        -0.3,
        -0.3
      ],
      "startCustomWeight": 0.55,
      "startGlow": 0.119112,
      "startOpacity": 1,
      "endRadius": 0.004,
      "endColor": [
        -0.3,
        -0.3,
        -0.303236
      ],
      "endCustomWeight": 0.545978,
      "endGlow": 0.045271,
      "endOpacity": 1,
      "inverted": false,
      "lit": false,
      "blur": 1,
      "blurFade": 0.566617,
      "enableEndCaps": true,
      "enableRoundedNormals": true,
      "endCapExtension": 0.657068,
      "lookDir": [
        0,
        0,
        0
      ],
      "useLookDir": false,
      "bulgeAmount": 0.1,
      "minimumRings": 4,
      "renderQueueOffset": 0,
      "depthOffset": 0
    },
    {
      "name": "BladeOutline",
      "position": [
        0,
        0,
        0
      ],
      "rotation": [
        0,
        0,
        0
      ],
      "length": 1,
      "geometryMode": "Simple",
      "hueShift": 0,
      "startRadius": 0.013,
      "startColor": [
        -0.195557,
        -0.201548,
        -0.187838
      ],
      "startCustomWeight": 0.843543,
      "startGlow": 0.58911,
      "startOpacity": 1,
      "endRadius": 0.005,
      "endColor": [
        0,
        0,
        0
      ],
      "endCustomWeight": 1,
      "endGlow": 0.8,
      "endOpacity": 1,
      "inverted": true,
      "lit": false,
      "blur": 1,
      "blurFade": 1.05461,
      "enableEndCaps": true,
      "enableRoundedNormals": true,
      "endCapExtension": 1,
      "lookDir": [
        0,
        0,
        0
      ],
      "useLookDir": false,
      "bulgeAmount": -0.4,
      "minimumRings": 4,
      "renderQueueOffset": 0,
      "depthOffset": 0
    },
    {
      "name": "BladeCore",
      "position": [
        0,
        0,
        0.005
      ],
      "rotation": [
        0,
        0,
        0
      ],
      "length": 0.983829,
      "geometryMode": "Simple",
      "hueShift": 0,
      "startRadius": 0.007,
      "startColor": [
        -0.3,
        -0.3,
        -0.3
      ],
      "startCustomWeight": 0.596582,
      "startGlow": 0.13428,
      "startOpacity": 1,
      "endRadius": 0.0001,
      "endColor": [
        -0.288369,
        -0.263677,
        -0.29253
      ],
      "endCustomWeight": 1,
      "endGlow": 0.736611,
      "endOpacity": 1,
      "inverted": false,
      "lit": false,
      "blur": 1,
      "blurFade": 1,
      "enableEndCaps": true,
      "enableRoundedNormals": true,
      "endCapExtension": 1,
      "lookDir": [
        0,
        0,
        0
      ],
      "useLookDir": false,
      "bulgeAmount": -0.6,
      "minimumRings": 4,
      "renderQueueOffset": 0,
      "depthOffset": 0
    },
    {
      "name": "Lower",
      "position": [
        0,
        0,
        -0.012
      ],
      "rotation": [
        0,
        180,
        0
      ],
      "length": 0.1,
      "geometryMode": "Simple",
      "hueShift": 0,
      "startRadius": 0.012,
      "startColor": [
        0.06505,
        0.071422,
        0.083694
      ],
      "startCustomWeight": 0.605491,
      "startGlow": 0.039972,
      "startOpacity": 1,
      "endRadius": 0.008,
      "endColor": [
        -0.713258,
        -0.698842,
        -0.683803
      ],
      "endCustomWeight": 0.78485,
      "endGlow": 0.344593,
      "endOpacity": 1,
      "inverted": true,
      "lit": false,
      "blur": 1,
      "blurFade": 5,
      "enableEndCaps": true,
      "enableRoundedNormals": true,
      "endCapExtension": 1,
      "lookDir": [
        0,
        0,
        0
      ],
      "useLookDir": false,
      "bulgeAmount": 0.2,
      "minimumRings": 7,
      "renderQueueOffset": 0,
      "depthOffset": 0
    },
    {
      "name": "LowerCore",
      "position": [
        0,
        0,
        -0.016
      ],
      "rotation": [
        0,
        180,
        0
      ],
      "length": 0.0915,
      "geometryMode": "Simple",
      "hueShift": 0,
      "startRadius": 0.006,
      "startColor": [
        -0.289126,
        -0.337931,
        -0.306242
      ],
      "startCustomWeight": 0.781862,
      "startGlow": 0.732548,
      "startOpacity": 1,
      "endRadius": 0.0045,
      "endColor": [
        0,
        0,
        0
      ],
      "endCustomWeight": 0.8,
      "endGlow": 0.536495,
      "endOpacity": 1,
      "inverted": false,
      "lit": false,
      "blur": 1,
      "blurFade": 5,
      "enableEndCaps": true,
      "enableRoundedNormals": true,
      "endCapExtension": 1,
      "lookDir": [
        0,
        0,
        0
      ],
      "useLookDir": false,
      "bulgeAmount": 0.3,
      "minimumRings": 3,
      "renderQueueOffset": 0,
      "depthOffset": 0
    }
  ]
})PRESET";

static constexpr std::string_view s_vain_vanilla_json = R"PRESET({
  "version": 1,
  "parts": [
    {
      "name": "Part 1",
      "position": [
        0,
        0,
        0
      ],
      "rotation": [
        0,
        0,
        0
      ],
      "length": 0.010091,
      "geometryMode": "Simple",
      "hueShift": 0,
      "startRadius": 0.015,
      "startColor": [
        1,
        1,
        1
      ],
      "startCustomWeight": 0.907306,
      "startGlow": 5,
      "startOpacity": 1,
      "endRadius": 0.015,
      "endColor": [
        1,
        1,
        1
      ],
      "endCustomWeight": 1,
      "endGlow": 1.5,
      "endOpacity": 1,
      "inverted": false,
      "lit": false,
      "blur": 1,
      "blurFade": 1,
      "enableEndCaps": false,
      "enableRoundedNormals": true,
      "endCapExtension": 0,
      "lookDir": [
        0,
        0,
        0
      ],
      "useLookDir": false,
      "bulgeAmount": 0,
      "minimumRings": 2,
      "renderQueueOffset": 0,
      "depthOffset": 0
    },
    {
      "name": "Part 2",
      "position": [
        0,
        0,
        -0.115
      ],
      "rotation": [
        -5E-06,
        180,
        180
      ],
      "length": 0.01,
      "geometryMode": "Simple",
      "hueShift": 0,
      "startRadius": 0.015,
      "startColor": [
        1,
        1,
        1
      ],
      "startCustomWeight": 1,
      "startGlow": 5,
      "startOpacity": 1,
      "endRadius": 0.015,
      "endColor": [
        1,
        1,
        1
      ],
      "endCustomWeight": 1,
      "endGlow": 1.5,
      "endOpacity": 1,
      "inverted": false,
      "lit": false,
      "blur": 1,
      "blurFade": 1,
      "enableEndCaps": false,
      "enableRoundedNormals": true,
      "endCapExtension": 0,
      "lookDir": [
        0,
        0,
        0
      ],
      "useLookDir": false,
      "bulgeAmount": 0,
      "minimumRings": 2,
      "renderQueueOffset": 0,
      "depthOffset": 0
    },
    {
      "name": "Part 3",
      "position": [
        0,
        0,
        -0.12
      ],
      "rotation": [
        0,
        0,
        0
      ],
      "length": 0.120331,
      "geometryMode": "Simple",
      "hueShift": 0,
      "startRadius": 0.015,
      "startColor": [
        1,
        1,
        1
      ],
      "startCustomWeight": 1,
      "startGlow": 0.534899,
      "startOpacity": 1,
      "endRadius": 0.015,
      "endColor": [
        1,
        1,
        1
      ],
      "endCustomWeight": 1,
      "endGlow": 0.497763,
      "endOpacity": 1,
      "inverted": true,
      "lit": false,
      "blur": 1,
      "blurFade": 0,
      "enableEndCaps": true,
      "enableRoundedNormals": true,
      "endCapExtension": 0.513539,
      "lookDir": [
        0,
        0,
        0
      ],
      "useLookDir": false,
      "bulgeAmount": 0,
      "minimumRings": 5,
      "renderQueueOffset": -3,
      "depthOffset": 0
    },
    {
      "name": "Part 4",
      "position": [
        0,
        0,
        0.01
      ],
      "rotation": [
        0,
        0,
        0
      ],
      "length": 0.965666,
      "geometryMode": "Simple",
      "hueShift": 0,
      "startRadius": 0.015,
      "startColor": [
        1,
        1,
        1
      ],
      "startCustomWeight": 1,
      "startGlow": 1.473248,
      "startOpacity": 1,
      "endRadius": 0.015,
      "endColor": [
        1,
        1,
        1
      ],
      "endCustomWeight": 1,
      "endGlow": 1.5,
      "endOpacity": 1,
      "inverted": true,
      "lit": false,
      "blur": 1,
      "blurFade": 5,
      "enableEndCaps": true,
      "enableRoundedNormals": true,
      "endCapExtension": 3,
      "lookDir": [
        0,
        0,
        0
      ],
      "useLookDir": false,
      "bulgeAmount": 0,
      "minimumRings": 2,
      "renderQueueOffset": 0,
      "depthOffset": 0
    },
    {
      "name": "Part 5",
      "position": [
        0,
        0,
        0.01
      ],
      "rotation": [
        0,
        0,
        0
      ],
      "length": 0.972876,
      "geometryMode": "Simple",
      "hueShift": 0,
      "startRadius": 0.007,
      "startColor": [
        0.32162,
        0.254124,
        0.275086
      ],
      "startCustomWeight": 0.883591,
      "startGlow": 0.102233,
      "startOpacity": 1,
      "endRadius": 0.006,
      "endColor": [
        -0.000486,
        -0.007724,
        -0.003759
      ],
      "endCustomWeight": 0.309137,
      "endGlow": 0.470726,
      "endOpacity": 1,
      "inverted": false,
      "lit": false,
      "blur": 1,
      "blurFade": 5,
      "enableEndCaps": true,
      "enableRoundedNormals": true,
      "endCapExtension": 3,
      "lookDir": [
        0,
        0,
        0
      ],
      "useLookDir": false,
      "bulgeAmount": 0,
      "minimumRings": 2,
      "renderQueueOffset": 0,
      "depthOffset": 0
    },
    {
      "name": "Part 6",
      "position": [
        0,
        0,
        -0.115
      ],
      "rotation": [
        0,
        0,
        0
      ],
      "length": 0.114013,
      "geometryMode": "Simple",
      "hueShift": 0,
      "startRadius": 0.011,
      "startColor": [
        0.416054,
        0.406474,
        0.406355
      ],
      "startCustomWeight": 0.804908,
      "startGlow": 0,
      "startOpacity": 1,
      "endRadius": 0.012,
      "endColor": [
        0.444943,
        0.444187,
        0.446571
      ],
      "endCustomWeight": 0.894136,
      "endGlow": 0,
      "endOpacity": 1,
      "inverted": true,
      "lit": false,
      "blur": 1,
      "blurFade": 4.240159,
      "enableEndCaps": true,
      "enableRoundedNormals": true,
      "endCapExtension": 1,
      "lookDir": [
        0,
        0,
        0
      ],
      "useLookDir": false,
      "bulgeAmount": 0,
      "minimumRings": 4,
      "renderQueueOffset": -10,
      "depthOffset": 0
    },
    {
      "name": "Part 7",
      "position": [
        0,
        0,
        -0.115
      ],
      "rotation": [
        0,
        0,
        355
      ],
      "length": 0.113608,
      "geometryMode": "Simple",
      "hueShift": 0,
      "startRadius": 0.007,
      "startColor": [
        0.031888,
        0.047381,
        0.059978
      ],
      "startCustomWeight": 0.648231,
      "startGlow": 0,
      "startOpacity": 1,
      "endRadius": 0.007,
      "endColor": [
        0.056322,
        0.015701,
        0.049387
      ],
      "endCustomWeight": 0.681655,
      "endGlow": 0,
      "endOpacity": 1,
      "inverted": false,
      "lit": false,
      "blur": 1,
      "blurFade": 5,
      "enableEndCaps": true,
      "enableRoundedNormals": true,
      "endCapExtension": 1,
      "lookDir": [
        0,
        0,
        0
      ],
      "useLookDir": false,
      "bulgeAmount": 0,
      "minimumRings": 3,
      "renderQueueOffset": 0,
      "depthOffset": 0
    },
    {
      "name": "Part 8",
      "position": [
        0,
        0,
        -0.125
      ],
      "rotation": [
        0,
        0,
        0
      ],
      "length": 0.000606,
      "geometryMode": "Simple",
      "hueShift": 0,
      "startRadius": 0.015,
      "startColor": [
        0.00603,
        -0.01227,
        0.026614
      ],
      "startCustomWeight": 1,
      "startGlow": 5,
      "startOpacity": 1,
      "endRadius": 0.015,
      "endColor": [
        0.029915,
        0.005392,
        0.039326
      ],
      "endCustomWeight": 1,
      "endGlow": 1.5,
      "endOpacity": 1,
      "inverted": false,
      "lit": false,
      "blur": 1,
      "blurFade": 1,
      "enableEndCaps": true,
      "enableRoundedNormals": true,
      "endCapExtension": 0,
      "lookDir": [
        0,
        0,
        0
      ],
      "useLookDir": false,
      "bulgeAmount": 0,
      "minimumRings": 2,
      "renderQueueOffset": 0,
      "depthOffset": 0
    }
  ]
})PRESET";

static const std::vector<EmbeddedPreset> s_embeddedPresets = {
    {"default", s_default_json},           {"menupointer-dot", s_menupointer_dot_json},
    {"vain-ring", s_vain_ring_json},       {"vain-seg-bi", s_vain_seg_bi_json},
    {"vain-seg-inv", s_vain_seg_inv_json}, {"vain-vanilla", s_vain_vanilla_json},
};

const std::vector<EmbeddedPreset> &GetEmbeddedPresets() {
    return s_embeddedPresets;
}

std::string_view GetEmbeddedPresetJson(std::string_view name) {
    for (const auto &p : s_embeddedPresets) {
        if (p.name == name)
            return p.json;
    }
    return s_default_json;
}

} // namespace VainSabers
