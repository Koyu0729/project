//*****************************************************************************
// 定数バッファ
//*****************************************************************************
cbuffer ConstantBuffer : register( b0 )
{
	matrix WorldViewProjection;
    matrix World;
    bool LightEnable;
    float3 Dummy;
}


//=============================================================================
// 頂点シェーダ
//=============================================================================
void VS(in float4 inPosition	: POSITION0,
		in float4 inDiffuse		: COLOR0,
        in float2 inTexCoord	: TEXCOORD0,
		in float4 inNormal		: NORMAL0,
		out float4 outPosition	: SV_POSITION,
		out float4 outDiffuse	: COLOR0,
		out float2 outTexCoord	: TEXCOORD0,
		out float4 outNormal		: NORMAL0)

{
        outPosition = mul(inPosition, WorldViewProjection);
        outDiffuse = inDiffuse;
        outTexCoord = inTexCoord;
	
        inNormal.w = 0.0;
        outNormal = mul(inNormal, World);
}

Texture2D g_Texture : register(t0);
SamplerState g_SamplerState : register(s0);

//=============================================================================
// ピクセルシェーダ
//=============================================================================
void PS(in float4 inPosition	: POSITION0,
		 in float4 inDiffuse	: COLOR0,
         in float2 inTexCoord	: TEXCOORD0,
		 in float4 inNormal		: NORMAL0,
		 out float4 outDiffuse	: SV_Target )
{
    outDiffuse = g_Texture.Sample(g_SamplerState, inTexCoord) * inDiffuse;
	
	if (LightEnable)
    {
	// ランバート拡散証明モデル
        inNormal.xyz = normalize(inNormal.xyz); // 正規化（長さ1にする）
        float3 lightDirection = normalize(float3(0.5, -1.0, 0.25));
        float3 light = -dot(inNormal.xyz, lightDirection);
        float3 lightColor = float3(1.0, 1.0, 0.8); // ライト光
        float3 ambientColor = float3(0.2, 0.2, 0.4); // 環境光
	
	// 明るさが-1のところを0に直す
        light = saturate(light) * lightColor + ambientColor;
        outDiffuse.rgb *= light;
    }
    
    // アルファ値が０未満なら描画しない
    clip(outDiffuse.a - 0.1);
}
