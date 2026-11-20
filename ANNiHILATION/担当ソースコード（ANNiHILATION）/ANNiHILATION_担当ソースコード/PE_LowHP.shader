Shader "Hidden/Custom/PE_LowHP"
{
    Properties
    {
        _Intensity("Intensity", Range(0, 1)) = 0

        _VignetteColor("Vignette Color", Color) = (1, 0, 0, 1)
        _ColorStrength("Color Strength", Range(0, 3)) = 1.2
        _VignettePower("Vignette Power", Range(0.1, 8)) = 2.5
        _Darkness("Darkness", Range(0, 1)) = 0.25

        _VignetteMaskTexture("Vignette Mask Texture", 2D) = "white" {}
        _VignetteMaskStrength("Vignette Mask Strength", Range(0, 1)) = 0
        _InvertVignetteMask("Invert Vignette Mask", Float) = 0
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
            Name "PE_LowHP"

            HLSLPROGRAM

            #pragma target 3.0
            #pragma vertex Vert
            #pragma fragment Frag

            #include "Packages/com.unity.render-pipelines.universal/ShaderLibrary/Core.hlsl"

            // RendererFeature の Blitter.BlitTexture から入ってくる画面
            TEXTURE2D_X(_BlitTexture);

            TEXTURE2D(_VignetteMaskTexture);
            SAMPLER(sampler_VignetteMaskTexture);

            float _Intensity;

            float4 _VignetteColor;
            float _ColorStrength;
            float _VignettePower;
            float _Darkness;

            float _VignetteMaskStrength;
            float _InvertVignetteMask;

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

            float GetVignetteMask(float2 uv)
            {
                float mask = SAMPLE_TEXTURE2D(
                    _VignetteMaskTexture,
                    sampler_VignetteMaskTexture,
                    uv
                ).r;

                if (_InvertVignetteMask > 0.5)
                {
                    mask = 1.0 - mask;
                }

                return saturate(mask);
            }

            float GetCircleVignette(float2 uv)
            {
                float2 centerUV = uv - 0.5;

                // 画面比率で横伸びしないように補正
                float2 screenSize = GetScaledScreenParams().xy;
                float aspect = screenSize.x / screenSize.y;
                centerUV.x *= aspect;

                float dist = length(centerUV);

                // 中心は0、外側ほど1
                float vignette = saturate(dist * 1.6);

                // 値が大きいほど端だけに寄る
                vignette = pow(vignette, _VignettePower);

                return saturate(vignette);
            }

            float GetFinalVignette(float2 uv)
            {
                float circleVignette = GetCircleVignette(uv);
                float maskVignette = GetVignetteMask(uv);

                // 0なら普通の丸ビネット
                // 1ならマスク形状のビネット
                float vignette = lerp(
                    circleVignette,
                    maskVignette,
                    saturate(_VignetteMaskStrength)
                );

                return saturate(vignette);
            }

            half4 Frag(Varyings input) : SV_Target
            {
                float2 uv = input.uv;

                half4 baseColor = SAMPLE_TEXTURE2D_X(
                    _BlitTexture,
                    sampler_LinearClamp,
                    uv
                );

                float intensity = saturate(_Intensity);

                if (intensity <= 0.0001)
                {
                    return baseColor;
                }

                float vignette = GetFinalVignette(uv);

                float damagePower = vignette * intensity;

                half3 color = baseColor.rgb;

                half3 vignetteColor = _VignetteColor.rgb;

                // 外側を赤くする
                color = lerp(
                    color,
                    vignetteColor,
                    saturate(damagePower * _ColorStrength)
                );

                // 外側を暗くする
                color *= 1.0 - saturate(damagePower * _Darkness);

                return half4(saturate(color), baseColor.a);
            }

            ENDHLSL
        }
    }
}