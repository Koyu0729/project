// 【ファイルの役割】横方向と縦方向の2パスで画面の背景をぼかす。
// 【依存・注意点】Renderer Featureから渡される_BlitTextureを読み取る。横パスの出力が縦パスの入力となる。
// このコメントは現状のコードの説明であり、実装者や動作保証を示すものではありません。

Shader "Hidden/UI/Background Blur"
{
    Properties
    {
        _BlitTexture ("Blit Texture", 2D) = "white" {}

        _HorizontalBlur ("Horizontal Blur", Float) = 2.0
        _VerticalBlur ("Vertical Blur", Float) = 2.0
    }

    SubShader
    {
        Tags
        {
            "RenderPipeline" = "UniversalPipeline"
        }

        ZWrite Off
        ZTest Always
        Cull Off

        // Pass 0 : Horizontal Blur
        Pass
        {
            Name "Horizontal Blur"

            HLSLPROGRAM
            #pragma vertex Vert
            #pragma fragment FragHorizontal

            #include "Packages/com.unity.render-pipelines.universal/ShaderLibrary/Core.hlsl"
            #include "Packages/com.unity.render-pipelines.core/Runtime/Utilities/Blit.hlsl"

            float _HorizontalBlur;

            half4 FragHorizontal(Varyings input) : SV_Target
            {
                float2 uv = input.texcoord;
                float2 texelSize = _BlitTexture_TexelSize.xy;

                float2 offset = float2(texelSize.x * _HorizontalBlur, 0.0);

                half4 color = half4(0, 0, 0, 0);

                color += SAMPLE_TEXTURE2D_X(_BlitTexture, sampler_LinearClamp, uv - offset * 4.0) * 0.0162162162;
                color += SAMPLE_TEXTURE2D_X(_BlitTexture, sampler_LinearClamp, uv - offset * 3.0) * 0.0540540541;
                color += SAMPLE_TEXTURE2D_X(_BlitTexture, sampler_LinearClamp, uv - offset * 2.0) * 0.1216216216;
                color += SAMPLE_TEXTURE2D_X(_BlitTexture, sampler_LinearClamp, uv - offset * 1.0) * 0.1945945946;
                color += SAMPLE_TEXTURE2D_X(_BlitTexture, sampler_LinearClamp, uv)                * 0.2270270270;
                color += SAMPLE_TEXTURE2D_X(_BlitTexture, sampler_LinearClamp, uv + offset * 1.0) * 0.1945945946;
                color += SAMPLE_TEXTURE2D_X(_BlitTexture, sampler_LinearClamp, uv + offset * 2.0) * 0.1216216216;
                color += SAMPLE_TEXTURE2D_X(_BlitTexture, sampler_LinearClamp, uv + offset * 3.0) * 0.0540540541;
                color += SAMPLE_TEXTURE2D_X(_BlitTexture, sampler_LinearClamp, uv + offset * 4.0) * 0.0162162162;

                color.a = 1.0;

                return color;
            }

            ENDHLSL
        }

        // Pass 1 : Vertical Blur
        Pass
        {
            Name "Vertical Blur"

            HLSLPROGRAM
            #pragma vertex Vert
            #pragma fragment FragVertical

            #include "Packages/com.unity.render-pipelines.universal/ShaderLibrary/Core.hlsl"
            #include "Packages/com.unity.render-pipelines.core/Runtime/Utilities/Blit.hlsl"

            float _VerticalBlur;

            half4 FragVertical(Varyings input) : SV_Target
            {
                float2 uv = input.texcoord;
                float2 texelSize = _BlitTexture_TexelSize.xy;

                float2 offset = float2(0.0, texelSize.y * _VerticalBlur);

                half4 color = half4(0, 0, 0, 0);

                color += SAMPLE_TEXTURE2D_X(_BlitTexture, sampler_LinearClamp, uv - offset * 4.0) * 0.0162162162;
                color += SAMPLE_TEXTURE2D_X(_BlitTexture, sampler_LinearClamp, uv - offset * 3.0) * 0.0540540541;
                color += SAMPLE_TEXTURE2D_X(_BlitTexture, sampler_LinearClamp, uv - offset * 2.0) * 0.1216216216;
                color += SAMPLE_TEXTURE2D_X(_BlitTexture, sampler_LinearClamp, uv - offset * 1.0) * 0.1945945946;
                color += SAMPLE_TEXTURE2D_X(_BlitTexture, sampler_LinearClamp, uv)                * 0.2270270270;
                color += SAMPLE_TEXTURE2D_X(_BlitTexture, sampler_LinearClamp, uv + offset * 1.0) * 0.1945945946;
                color += SAMPLE_TEXTURE2D_X(_BlitTexture, sampler_LinearClamp, uv + offset * 2.0) * 0.1216216216;
                color += SAMPLE_TEXTURE2D_X(_BlitTexture, sampler_LinearClamp, uv + offset * 3.0) * 0.0540540541;
                color += SAMPLE_TEXTURE2D_X(_BlitTexture, sampler_LinearClamp, uv + offset * 4.0) * 0.0162162162;

                color.a = 1.0;

                return color;
            }

            ENDHLSL
        }
    }
}