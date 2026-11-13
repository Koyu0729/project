struct ParticleGpuData
{
    float4 PositionLife; // xyz = position, w = life
    float4 VelocityMaxLife; // xyz = velocity, w = maxLife
    float4 ColorSize; // xyz = color, w = size
    float4 Param; // x = active, y = swirl, z = drag, w = gravity
};

StructuredBuffer<ParticleGpuData> Particles : register(t0);

cbuffer RenderConstantBuffer : register(b5)
{
    matrix ViewProjection;
    float4 CameraRight;
    float4 CameraUp;
};

struct VS_IN
{
    float3 Position : POSITION;
    float3 Normal : NORMAL;
    float4 Diffuse : COLOR;
    float2 TexCoord : TEXCOORD0;
    uint InstanceId : SV_InstanceID;
};

struct VS_OUT
{
    float4 Position : SV_POSITION;
    float2 TexCoord : TEXCOORD0;
    float4 Color : COLOR0;
    float LifeRate : TEXCOORD1;
};

//===================================
// Particle Mode
//===================================
static const float PARTICLE_SIZE_PULSE_BASE = 0.92f;
static const float PARTICLE_SIZE_PULSE_RANGE = 0.08f;
static const float PARTICLE_SIZE_PULSE_SPEED = 10.0f;
static const float PARTICLE_SIZE_MIN_RATE = 0.35f;
static const float PARTICLE_SIZE_MAX_RATE = 1.10f;


//=============================================================
// VertexShader
// DrawInstancedで渡されたInstanceIdを使い、
// GPU上のParticleBufferから1粒分のデータを読む。
// 粒子の中心位置からCameraRight / CameraUp方向へ頂点を広げ、
// 常にカメラを向く板ポリゴンとして描画する。
//=============================================================
VS_OUT main(VS_IN input)
{
    VS_OUT output;

    ParticleGpuData particle = Particles[input.InstanceId];
    
    // 残り寿命の割合を計算する。
    // 1.0に近いほど生まれた直後、0.0に近いほど消える直前。
    float lifeRate = 0.0f;

    if (particle.VelocityMaxLife.w > 0.0f)
    {
        lifeRate = saturate(particle.PositionLife.w / particle.VelocityMaxLife.w);
    }

    // 寿命に応じてサイズを変える。
    // 消え際は小さくなる。
    float sizeAnim =
        PARTICLE_SIZE_PULSE_BASE +
        PARTICLE_SIZE_PULSE_RANGE * sin((1.0f - lifeRate) * PARTICLE_SIZE_PULSE_SPEED);

    float size =
        particle.ColorSize.w *
        lerp(PARTICLE_SIZE_MIN_RATE, PARTICLE_SIZE_MAX_RATE, lifeRate) *
        sizeAnim;

    //===================================
    // ビルボード化
    //===================================
    float3 worldPosition = particle.PositionLife.xyz;

    worldPosition += CameraRight.xyz * input.Position.x * size;
    worldPosition += CameraUp.xyz * input.Position.y * size;

    output.Position = mul(float4(worldPosition, 1.0f), ViewProjection);
    output.TexCoord = input.TexCoord;
    output.Color = float4(particle.ColorSize.xyz, 1.0f);
    output.LifeRate = lifeRate;

    return output;
}