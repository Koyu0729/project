// 【ファイルの役割】ぼかし済みの画面をUI画像のアルファに合わせて表示する。
// 【依存・注意点】_UIBlurTextureを描画側が供給する必要がある。Renderer FeatureとUI描画の順序を確認する。
// このコメントは現状のコードの説明であり、実装者や動作保証を示すものではありません。

Shader "UI/UI Blur Mask"
{
    Properties
    {
        [PerRendererData] _MainTex ("Mask Texture", 2D) = "white" {}
        _Color ("Tint", Color) = (1,1,1,1)

        _BlurTint ("Blur Tint", Color) = (1,1,1,1)
        _AlphaMultiplier ("Alpha Multiplier", Range(0, 1)) = 1
        _AlphaThreshold ("Alpha Threshold", Range(0, 1)) = 0.01

        // Unity UI Mask / RectMask2D 用
        _StencilComp ("Stencil Comparison", Float) = 8
        _Stencil ("Stencil ID", Float) = 0
        _StencilOp ("Stencil Operation", Float) = 0
        _StencilWriteMask ("Stencil Write Mask", Float) = 255
        _StencilReadMask ("Stencil Read Mask", Float) = 255
        _ColorMask ("Color Mask", Float) = 15
    }

    SubShader
    {
        Tags
        {
            "Queue"="Transparent"
            "IgnoreProjector"="True"
            "RenderType"="Transparent"
            "PreviewType"="Plane"
            "CanUseSpriteAtlas"="True"
            "RenderPipeline"="UniversalPipeline"
        }

        Stencil
        {
            Ref [_Stencil]
            Comp [_StencilComp]
            Pass [_StencilOp]
            ReadMask [_StencilReadMask]
            WriteMask [_StencilWriteMask]
        }

        Cull Off
        Lighting Off
        ZWrite Off
        ZTest [unity_GUIZTestMode]
        Blend SrcAlpha OneMinusSrcAlpha
        ColorMask [_ColorMask]

        Pass
        {
            Name "UIBlurMask"

            HLSLPROGRAM
            #pragma vertex vert
            #pragma fragment frag

            #include "Packages/com.unity.render-pipelines.universal/ShaderLibrary/Core.hlsl"

            TEXTURE2D(_MainTex);
            SAMPLER(sampler_MainTex);

            TEXTURE2D_X(_UIBlurTexture);
            SAMPLER(sampler_UIBlurTexture);

            float4 _MainTex_ST;
            float4 _Color;
            float4 _BlurTint;
            float _AlphaMultiplier;
            float _AlphaThreshold;

            struct Attributes
            {
                float4 positionOS : POSITION;
                float4 color      : COLOR;
                float2 uv         : TEXCOORD0;
            };

            struct Varyings
            {
                float4 positionCS : SV_POSITION;
                float4 color      : COLOR;
                float2 uv         : TEXCOORD0;
                float4 screenPos  : TEXCOORD1;
            };

            Varyings vert(Attributes input)
            {
                Varyings output;

                output.positionCS = TransformObjectToHClip(input.positionOS.xyz);
                output.uv = TRANSFORM_TEX(input.uv, _MainTex);
                output.color = input.color * _Color;
                output.screenPos = ComputeScreenPos(output.positionCS);

                return output;
            }

            half4 frag(Varyings input) : SV_Target
            {
                half4 maskColor = SAMPLE_TEXTURE2D(_MainTex, sampler_MainTex, input.uv);
            
                float maskAlpha = maskColor.a * input.color.a * _AlphaMultiplier;
            
                if (maskAlpha <= _AlphaThreshold)
                {
                    discard;
                }
            
                float2 screenUV = input.screenPos.xy / input.screenPos.w;
                screenUV = UnityStereoTransformScreenSpaceTex(screenUV);
            
                half4 blurColor = SAMPLE_TEXTURE2D_X(
                    _UIBlurTexture,
                    sampler_LinearClamp,
                    screenUV
                );
            
                return half4(blurColor.rgb, maskAlpha);
            }

            ENDHLSL
        }
    }
}