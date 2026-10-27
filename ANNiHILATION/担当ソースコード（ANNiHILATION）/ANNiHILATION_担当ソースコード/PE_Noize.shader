// 【ファイルの役割】横方向の歪み、RGBずれ、粒状ノイズ、走査線を組み合わせて画面を加工する。
// 【依存・注意点】Intensityとマスクで効果量を決める。被ダメージと死亡で同じShaderを使い、制御側が強度を変える。
// このコメントは現状のコードの説明であり、実装者や動作保証を示すものではありません。

Shader "Hidden/Custom/PE_Noize"
{
    Properties
    {
        _Intensity("Intensity", Range(0, 1)) = 1

        _NoiseScale("Noise Scale", Range(1, 500)) = 120
        _NoiseSpeed("Noise Speed", Range(0, 100)) = 15

        _MaskTexture("Mask Texture", 2D) = "white" {}
        _MaskStrength("Mask Strength", Range(0, 1)) = 0
        _InvertMask("Invert Mask", Float) = 0

        _DistortionStrength("Distortion Strength", Range(0, 5)) = 1
        _WaveStrength("Wave Strength", Range(0, 5)) = 1
        _RowJitterStrength("Row Jitter Strength", Range(0, 5)) = 1
        _BandStrength("Band Strength", Range(0, 5)) = 1

        _RgbShiftStrength("RGB Shift Strength", Range(0, 5)) = 1
        _GrainStrength("Grain Strength", Range(0, 5)) = 0.5
        _ScanlineStrength("Scanline Strength", Range(0, 5)) = 0.5
    }

    SubShader
    {
        Tags
        {
            "RenderPipeline" = "UniversalPipeline"
            "RenderType" = "Opaque"
        }

        ZWrite Off
        ZTest Always
        Cull Off
        Blend Off

        Pass
        {
            Name "PE_Noize"

            HLSLPROGRAM

            #pragma target 3.0
            #pragma vertex Vert
            #pragma fragment Frag

            #include "Packages/com.unity.render-pipelines.universal/ShaderLibrary/Core.hlsl"

            // CustomPostProcessEffectRendererFeature の Blitter.BlitTexture から渡される画面
            TEXTURE2D_X(_BlitTexture);

            TEXTURE2D(_MaskTexture);
            SAMPLER(sampler_MaskTexture);

            float _Intensity;

            float _NoiseScale;
            float _NoiseSpeed;

            float _MaskStrength;
            float _InvertMask;

            float _DistortionStrength;
            float _WaveStrength;
            float _RowJitterStrength;
            float _BandStrength;

            float _RgbShiftStrength;
            float _GrainStrength;
            float _ScanlineStrength;

            struct Varyings
            {
                float4 positionHCS : SV_POSITION;
                float2 uv : TEXCOORD0;
            };

            Varyings Vert(uint vertexID : SV_VertexID)
            {
                Varyings output;

                output.positionHCS = GetFullScreenTriangleVertexPosition(vertexID);
                output.uv = GetFullScreenTriangleTexCoord(vertexID);

                return output;
            }

            float Hash12(float2 p)
            {
                float3 p3 = frac(float3(p.xyx) * 0.1031);
                p3 += dot(p3, p3.yzx + 33.33);
                return frac((p3.x + p3.y) * p3.z);
            }

            float2 ClampScreenUV(float2 uv)
            {
                return clamp(uv, float2(0.001, 0.001), float2(0.999, 0.999));
            }

            float GetMaskPower(float2 uv)
            {
                float mask = SAMPLE_TEXTURE2D(_MaskTexture, sampler_MaskTexture, uv).r;

                if (_InvertMask > 0.5)
                {
                    mask = 1.0 - mask;
                }

                // MaskStrength 0 なら全体、1 ならマスク通り
                return lerp(1.0, mask, saturate(_MaskStrength));
            }

            half4 Frag(Varyings input) : SV_Target
            {
                float2 uv = input.uv;

                float power = saturate(_Intensity);

                if (power <= 0.0001)
                {
                    return SAMPLE_TEXTURE2D_X(_BlitTexture, sampler_LinearClamp, uv);
                }

                float time = _Time.y * _NoiseSpeed;

                float maskPower = GetMaskPower(uv);
                float effectPower = power * maskPower;

                // -----------------------------
                // 横方向ノイズ・歪み
                // -----------------------------
                float row = floor(uv.y * _NoiseScale);

                float rowNoiseA = Hash12(float2(row, floor(time * 10.0)));
                float rowNoiseB = Hash12(float2(row * 2.17, floor(time * 6.0)));

                // 細かい横ラインずれ
                float rowJitter =
                    (rowNoiseA - 0.5) *
                    0.012 *
                    _RowJitterStrength *
                    _DistortionStrength *
                    effectPower;

                // なめらかな波
                float wave =
                    sin((uv.y * 35.0) + time) *
                    0.004 *
                    _WaveStrength *
                    _DistortionStrength *
                    effectPower;

                // 太い帯が横にズレる
                float bandLine = floor((uv.y + time * 0.03) * 12.0);
                float bandNoise = Hash12(float2(bandLine, floor(time * 2.0)));

                float bandMask = step(0.78, bandNoise);
                float bandShift =
                    (bandNoise - 0.5) *
                    0.045 *
                    bandMask *
                    _BandStrength *
                    _DistortionStrength *
                    effectPower;

                float xOffset = rowJitter + wave + bandShift;

                float2 baseUV = ClampScreenUV(uv + float2(xOffset, 0.0));

                // -----------------------------
                // RGB横ズレ
                // -----------------------------
                float rgbGap =
                    0.0025 *
                    _RgbShiftStrength *
                    effectPower;

                float2 uvR = ClampScreenUV(baseUV + float2(rgbGap, 0.0));
                float2 uvG = ClampScreenUV(baseUV);
                float2 uvB = ClampScreenUV(baseUV - float2(rgbGap, 0.0));

                half r = SAMPLE_TEXTURE2D_X(_BlitTexture, sampler_LinearClamp, uvR).r;
                half g = SAMPLE_TEXTURE2D_X(_BlitTexture, sampler_LinearClamp, uvG).g;
                half b = SAMPLE_TEXTURE2D_X(_BlitTexture, sampler_LinearClamp, uvB).b;

                half3 color = half3(r, g, b);

                // -----------------------------
                // 粒ノイズ
                // -----------------------------
                float grain =
                    Hash12(uv * _ScreenParams.xy + time * 60.0) - 0.5;

                color += grain * 0.12 * _GrainStrength * effectPower;

                // -----------------------------
                // 走査線
                // -----------------------------
                float scanline =
                    sin(uv.y * _ScreenParams.y * 3.14159265);

                scanline = scanline * 0.5 + 0.5;

                float scanDark =
                    lerp(1.0, scanline, 0.35 * _ScanlineStrength * effectPower);

                color *= scanDark;

                return half4(saturate(color), 1.0);
            }

            ENDHLSL
        }
    }
}