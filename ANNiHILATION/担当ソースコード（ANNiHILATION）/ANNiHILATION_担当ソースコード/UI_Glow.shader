// 【ファイルの役割】UI画像を周囲にサンプリングし、疑似的な発光の広がりを描画する。
// 【依存・注意点】実際のBloomとは別の表現。調整するプロパティは_BloomColor、_BloomStrength、_BloomSizeなど。
// このコメントは現状のコードの説明であり、実装者や動作保証を示すものではありません。

Shader "UI/OverlayPseudoBloom"
{
    Properties
    {
        [PerRendererData] _MainTex ("Sprite Texture", 2D) = "white" {}

        _Color ("Tint", Color) = (1,1,1,1)

        [HDR] _BloomColor ("Bloom Color", Color) = (1,1,1,1)
        _BloomStrength ("Bloom Strength", Range(0, 10)) = 100.5
        _BloomSize ("Bloom Size", Range(0, 30)) = 5.0
        _AlphaPower ("Alpha Power", Range(0.1, 5.0)) = 1.0
        _FinalAlpha ("Final Alpha", Range(0, 1)) = 0.7

        _AlphaCutoff ("Alpha Cutoff", Range(0, 1)) = 0.001

        // Unity UI Mask / RectMask2D 対応用
        _StencilComp ("Stencil Comparison", Float) = 8
        _Stencil ("Stencil ID", Float) = 0
        _StencilOp ("Stencil Operation", Float) = 0
        _StencilWriteMask ("Stencil Write Mask", Float) = 255
        _StencilReadMask ("Stencil Read Mask", Float) = 255
        _ColorMask ("Color Mask", Float) = 15

        [Toggle(UNITY_UI_ALPHACLIP)] _UseUIAlphaClip ("Use Alpha Clip", Float) = 0
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

        // Overlay UI用。普通の透明合成。
        Blend SrcAlpha OneMinusSrcAlpha

        ColorMask [_ColorMask]

        Pass
        {
            Name "UI_OverlayPseudoBloom"

            CGPROGRAM
            #pragma vertex vert
            #pragma fragment frag
            #pragma target 3.0

            #pragma multi_compile_local _ UNITY_UI_CLIP_RECT
            #pragma multi_compile_local _ UNITY_UI_ALPHACLIP

            #include "UnityCG.cginc"
            #include "UnityUI.cginc"

            struct appdata_t
            {
                float4 vertex   : POSITION;
                float4 color    : COLOR;
                float2 texcoord : TEXCOORD0;
            };

            struct v2f
            {
                float4 vertex        : SV_POSITION;
                fixed4 color         : COLOR;
                float2 texcoord      : TEXCOORD0;
                float4 worldPosition : TEXCOORD1;
            };

            sampler2D _MainTex;
            float4 _MainTex_ST;
            float4 _MainTex_TexelSize;

            fixed4 _Color;
            fixed4 _BloomColor;

            float _BloomStrength;
            float _BloomSize;
            float _AlphaPower;
            float _FinalAlpha;
            float _AlphaCutoff;

            float4 _ClipRect;

            v2f vert(appdata_t v)
            {
                v2f o;

                o.worldPosition = v.vertex;
                o.vertex = UnityObjectToClipPos(v.vertex);
                o.texcoord = TRANSFORM_TEX(v.texcoord, _MainTex);
                o.color = v.color * _Color;

                return o;
            }

            fixed4 SampleSprite(float2 uv, fixed4 vertexColor)
            {
                return tex2D(_MainTex, uv) * vertexColor;
            }

            fixed4 frag(v2f i) : SV_Target
            {
                float2 texel = _MainTex_TexelSize.xy * _BloomSize;

                fixed4 blur = 0.0;
                float weightSum = 0.0;

                // 中心
                float w0 = 0.20;
                blur += SampleSprite(i.texcoord, i.color) * w0;
                weightSum += w0;

                // 近距離
                float w1 = 0.14;
                blur += SampleSprite(i.texcoord + texel * float2( 1,  0), i.color) * w1;
                blur += SampleSprite(i.texcoord + texel * float2(-1,  0), i.color) * w1;
                blur += SampleSprite(i.texcoord + texel * float2( 0,  1), i.color) * w1;
                blur += SampleSprite(i.texcoord + texel * float2( 0, -1), i.color) * w1;
                weightSum += w1 * 4.0;

                // 近距離斜め
                float w2 = 0.10;
                blur += SampleSprite(i.texcoord + texel * float2( 1,  1), i.color) * w2;
                blur += SampleSprite(i.texcoord + texel * float2(-1,  1), i.color) * w2;
                blur += SampleSprite(i.texcoord + texel * float2( 1, -1), i.color) * w2;
                blur += SampleSprite(i.texcoord + texel * float2(-1, -1), i.color) * w2;
                weightSum += w2 * 4.0;

                // 中距離
                float2 texel2 = texel * 2.0;
                float w3 = 0.075;
                blur += SampleSprite(i.texcoord + texel2 * float2( 1,  0), i.color) * w3;
                blur += SampleSprite(i.texcoord + texel2 * float2(-1,  0), i.color) * w3;
                blur += SampleSprite(i.texcoord + texel2 * float2( 0,  1), i.color) * w3;
                blur += SampleSprite(i.texcoord + texel2 * float2( 0, -1), i.color) * w3;
                weightSum += w3 * 4.0;

                // 中距離斜め
                float w4 = 0.055;
                blur += SampleSprite(i.texcoord + texel2 * float2( 1,  1), i.color) * w4;
                blur += SampleSprite(i.texcoord + texel2 * float2(-1,  1), i.color) * w4;
                blur += SampleSprite(i.texcoord + texel2 * float2( 1, -1), i.color) * w4;
                blur += SampleSprite(i.texcoord + texel2 * float2(-1, -1), i.color) * w4;
                weightSum += w4 * 4.0;

                // 遠距離
                float2 texel3 = texel * 3.5;
                float w5 = 0.035;
                blur += SampleSprite(i.texcoord + texel3 * float2( 1,  0), i.color) * w5;
                blur += SampleSprite(i.texcoord + texel3 * float2(-1,  0), i.color) * w5;
                blur += SampleSprite(i.texcoord + texel3 * float2( 0,  1), i.color) * w5;
                blur += SampleSprite(i.texcoord + texel3 * float2( 0, -1), i.color) * w5;
                weightSum += w5 * 4.0;

                // 遠距離斜め
                float w6 = 0.025;
                blur += SampleSprite(i.texcoord + texel3 * float2( 1,  1), i.color) * w6;
                blur += SampleSprite(i.texcoord + texel3 * float2(-1,  1), i.color) * w6;
                blur += SampleSprite(i.texcoord + texel3 * float2( 1, -1), i.color) * w6;
                blur += SampleSprite(i.texcoord + texel3 * float2(-1, -1), i.color) * w6;
                weightSum += w6 * 4.0;

                blur /= max(weightSum, 0.0001);

                float alpha = pow(saturate(blur.a), _AlphaPower);

                fixed4 finalColor;
                finalColor.rgb = blur.rgb * _BloomColor.rgb * _BloomStrength;
                finalColor.a = saturate(alpha * _BloomColor.a * _FinalAlpha);

                #ifdef UNITY_UI_CLIP_RECT
                finalColor.a *= UnityGet2DClipping(i.worldPosition.xy, _ClipRect);
                #endif

                #ifdef UNITY_UI_ALPHACLIP
                clip(finalColor.a - _AlphaCutoff);
                #endif

                return finalColor;
            }

            ENDCG
        }
    }
}