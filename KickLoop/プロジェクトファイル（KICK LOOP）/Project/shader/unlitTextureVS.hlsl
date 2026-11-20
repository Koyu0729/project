
#include "common.hlsl"


void main(in VS_IN In, out PS_IN Out, out float3 worldNormal : TEXCOORD1, out float3 localPosition : TEXCOORD2)
{

	matrix wvp;
	wvp = mul(World, View);
	wvp = mul(wvp, Projection);

	Out.Position = mul(In.Position, wvp);
	Out.TexCoord = In.TexCoord;
	Out.Diffuse = In.Diffuse * Material.Diffuse;
	worldNormal = normalize(mul(float4(In.Normal.xyz, 0.0f), World).xyz);
	localPosition = In.Position.xyz;

}

