#include "common.hlsl"

// 通常の壁テクスチャ。
Texture2D g_Texture : register(t0);
// 色を変えたい部分を指定するマスクテクスチャ。
Texture2D g_EmissiveMaskTexture : register(t1);

SamplerState g_SamplerState : register(s0);

// StageMaskBlock.cpp側から渡される、マスク表示用のパラメータ。
cbuffer EmissiveBuffer : register(b1)
{
    float4 EmissiveColor;

	// マスク部分を指定色へ寄せる強さ。
    float EmissivePower;

    float3 EmissivePadding;
};

// マスクテクスチャの値に応じて一部をEmissiveColorへ寄せる。
void main(in PS_IN In, out float4 outDiffuse : SV_Target)
{
    float4 baseColor;

	// 通常テクスチャを使う場合は、テクスチャ色と頂点カラーを掛け合わせる。
    if (Material.TextureEnable)
    {
        baseColor =
			g_Texture.Sample(
				g_SamplerState,
				In.TexCoord
			);

        baseColor *= In.Diffuse;
    }
    else
    {
        baseColor = In.Diffuse;
    }
	
	// マスク値から広い範囲と芯部分の強さを作り、通常色を指定色へ段階的に寄せる。
	// マスク画像は白黒なので、赤チャンネルを明るさとして使う。
    float mask =
		g_EmissiveMaskTexture.Sample(
			g_SamplerState,
			In.TexCoord
		).r;

    mask = saturate(mask);
	
    float colorMask =
		smoothstep(
			0.18f,
			0.75f,
			mask
		);
	
    float coreMask =
		smoothstep(
			0.35f,
			0.80f,
			mask
		);
	
    float3 targetColor =
		EmissiveColor.rgb * EmissivePower;
	
    float3 color =
		lerp(
			baseColor.rgb,
			targetColor,
			colorMask
		);
	
    color =
		lerp(
			color,
			targetColor,
			coreMask
		);
	
    outDiffuse.rgb = saturate(color);

    outDiffuse.a = baseColor.a;
}