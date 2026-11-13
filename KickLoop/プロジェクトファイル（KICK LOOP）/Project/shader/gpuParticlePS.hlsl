Texture2D ParticleTexture : register(t0);
SamplerState Sampler : register(s0);

struct PS_IN
{
    float4 Position : SV_POSITION;
    float2 TexCoord : TEXCOORD0;
    float4 Color : COLOR0;
    float LifeRate : TEXCOORD1;
};

//===================================
// Particle Alpha
//===================================
static const float PARTICLE_ALPHA_DISCARD_THRESHOLD = 0.01f;
static const float PARTICLE_FADE_PULSE_BASE = 0.88f;
static const float PARTICLE_FADE_PULSE_RANGE = 0.12f;
static const float PARTICLE_FADE_PULSE_SPEED = 12.0f;

//===================================
// PixelShader
// VertexShaderから渡されたUVでテクスチャを読み、
// パーティクルごとの色と寿命を掛け合わせて最終色を作る。
// 加算ブレンド前提なので、RGBにもalphaを掛けて黒い縁が出にくいようにする。
//===================================
float4 main(PS_IN input) : SV_Target
{
    float lifeAlpha = input.LifeRate;
    
    if (lifeAlpha <= PARTICLE_ALPHA_DISCARD_THRESHOLD)
    {
        discard;
    }

    float4 tex = ParticleTexture.Sample(Sampler, input.TexCoord);
    
    // 透明部分は描画しない。
    if (tex.a <= PARTICLE_ALPHA_DISCARD_THRESHOLD)
    {
        discard;
    }
    
    // 寿命に応じてフェードアウトする。
    // 少し明滅を加えて、消え際の変化を出す。
    lifeAlpha = input.LifeRate;

    float fadePulse =
    PARTICLE_FADE_PULSE_BASE +
    PARTICLE_FADE_PULSE_RANGE *
    sin((1.0f - input.LifeRate) * PARTICLE_FADE_PULSE_SPEED);

    float alpha = tex.a * lifeAlpha * fadePulse;
    
    // テクスチャ色 × パーティクル色。
    // 加算ブレンド前提なので、RGBにもalphaを掛ける。
    float3 color = tex.rgb * input.Color.rgb;

    color *= alpha;

    return float4(color, alpha);
}