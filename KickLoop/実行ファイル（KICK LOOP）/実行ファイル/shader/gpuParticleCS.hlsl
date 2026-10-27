struct ParticleGpuData
{
    float4 PositionLife; // xyz = position, w = life
    float4 VelocityMaxLife; // xyz = velocity, w = maxLife
    float4 ColorSize; // xyz = color, w = size
    float4 Param; // x = active, y = swirl, z = drag, w = gravity
};

//===================================
// UpdateConstantBuffer
// register(b0)
// b は ConstantBuffer 用のスロット。
//===================================
cbuffer UpdateConstantBuffer : register(b0)
{
    float4 EmitPosition; // xyz = emit position
    float4 EmitParam; // x = count, y = start index, z = mode, w = max count
    float4 TimeParam; // x = time, y = delta frame
    float4 LifeSizeParam; // x = life min, y = life max, z = size min, w = size max
    float4 VelocityParam; // x = speed min, y = speed max, z = y min, w = y max
    float4 GravityParam; // x = gravity
};

//===================================
// Particles
// register(u0)
// u は UAV(Unordered Access View) 用のスロット。
//===================================
RWStructuredBuffer<ParticleGpuData> Particles : register(u0);

//===================================
// Random Hash
//===================================
static const uint RANDOM_HASH_MASK = 0x00ffffffU;
static const float RANDOM_HASH_DIVISOR = 16777216.0f;

static const uint RANDOM_HASH_SHIFT_A = 16U;
static const uint RANDOM_HASH_SHIFT_B = 15U;

static const uint RANDOM_HASH_MULTIPLY_A = 0x7feb352dU;
static const uint RANDOM_HASH_MULTIPLY_B = 0x846ca68bU;

// seedBaseを作るための係数。
static const uint RANDOM_INDEX_HASH = 1973U;
static const uint RANDOM_LOCAL_INDEX_HASH = 9277U;
static const uint RANDOM_EMIT_START_HASH = 26699U;
static const uint RANDOM_TIME_HASH = 911U;
static const uint RANDOM_MODE_HASH = 3571U;

// 共通乱数オフセット。
static const uint RANDOM_LIFE_OFFSET = 0x0101U;
static const uint RANDOM_SIZE_OFFSET = 0x0202U;
static const uint RANDOM_COLOR_OFFSET = 0x9000U;

static const uint RANDOM_ANGLE_OFFSET = 0x0301U;
static const uint RANDOM_SPEED_OFFSET = 0x0302U;
static const uint RANDOM_Y_OFFSET = 0x0303U;
static const uint RANDOM_X_OFFSET = 0x0304U;
static const uint RANDOM_Z_OFFSET = 0x0305U;
static const uint RANDOM_SWIRL_OFFSET = 0x0306U;
static const uint RANDOM_DRAG_OFFSET = 0x0307U;

//// FireWork
//static const uint FIREWORK_DIRECTION_OFFSET = 0x1000U;
//static const uint FIREWORK_SPEED_OFFSET = 0x1001U;
//static const uint FIREWORK_SWIRL_OFFSET = 0x1002U;
//static const uint FIREWORK_DRAG_OFFSET = 0x1003U;

//// FloatUp
//static const uint FLOAT_UP_ANGLE_OFFSET = 0x2000U;
//static const uint FLOAT_UP_SPEED_OFFSET = 0x2001U;
//static const uint FLOAT_UP_Y_OFFSET = 0x2002U;
//static const uint FLOAT_UP_SWIRL_OFFSET = 0x2003U;
//static const uint FLOAT_UP_DRAG_OFFSET = 0x2004U;

//// Spark
//static const uint SPARK_ANGLE_OFFSET = 0x3000U;
//static const uint SPARK_SPEED_OFFSET = 0x3001U;
//static const uint SPARK_X_OFFSET = 0x3002U;
//static const uint SPARK_Y_OFFSET = 0x3003U;
//static const uint SPARK_Z_OFFSET = 0x3004U;
//static const uint SPARK_SWIRL_OFFSET = 0x3005U;
//static const uint SPARK_DRAG_OFFSET = 0x3006U;

//// Landing
//static const uint LANDING_ANGLE_OFFSET = 0x4000U;
//static const uint LANDING_SPEED_OFFSET = 0x4001U;
//static const uint LANDING_X_OFFSET = 0x4002U;
//static const uint LANDING_Y_OFFSET = 0x4003U;
//static const uint LANDING_Z_OFFSET = 0x4004U;
//static const uint LANDING_SWIRL_OFFSET = 0x4005U;
//static const uint LANDING_DRAG_OFFSET = 0x4006U;

//// Rain
//static const uint RAIN_ANGLE_OFFSET = 0x5000U;
//static const uint RAIN_HORIZONTAL_OFFSET = 0x5001U;
//static const uint RAIN_Y_OFFSET = 0x5002U;
//static const uint RAIN_SWIRL_OFFSET = 0x5003U;
//static const uint RAIN_DRAG_OFFSET = 0x5004U;

//===================================
// Particle Mode
//===================================
static const uint EMIT_MODE_FIREWORK = 0;
static const uint EMIT_MODE_FLOAT_UP = 1;
static const uint EMIT_MODE_SPARK = 2;
static const uint EMIT_MODE_LANDING = 3;

static const float PARTICLE_ANGLE_MIN = 0.0f;
static const float PARTICLE_ANGLE_MAX = 6.2831853f;

// FireWork
static const float FIREWORK_SWIRL_MIN = -0.018f;
static const float FIREWORK_SWIRL_MAX = 0.018f;
static const float FIREWORK_DRAG_MIN = 0.992f;
static const float FIREWORK_DRAG_MAX = 0.998f;

// FloatUp
static const float FLOAT_UP_SWIRL_MIN = -0.012f;
static const float FLOAT_UP_SWIRL_MAX = 0.012f;
static const float FLOAT_UP_DRAG_MIN = 0.990f;
static const float FLOAT_UP_DRAG_MAX = 0.998f;

// Spark
static const float SPARK_OFFSET_MIN = -0.12f;
static const float SPARK_OFFSET_MAX = 0.12f;

static const float SPARK_SWIRL_MIN = -0.006f;
static const float SPARK_SWIRL_MAX = 0.006f;
static const float SPARK_DRAG_MIN = 0.955f;
static const float SPARK_DRAG_MAX = 0.985f;

// Landing
static const float LANDING_OFFSET_MIN = -0.08f;
static const float LANDING_OFFSET_MAX = 0.08f;

static const float LANDING_SWIRL_MIN = -0.004f;
static const float LANDING_SWIRL_MAX = 0.004f;
static const float LANDING_DRAG_MIN = 0.960f;
static const float LANDING_DRAG_MAX = 0.988f;

// Rain
static const float RAIN_SWIRL_MIN = -0.0015f;
static const float RAIN_SWIRL_MAX = 0.0015f;
static const float RAIN_DRAG_MIN = 0.998f;
static const float RAIN_DRAG_MAX = 1.0f;

uint Hash(uint value)
{
    value ^= value >> RANDOM_HASH_SHIFT_A;
    value *= RANDOM_HASH_MULTIPLY_A;
    value ^= value >> RANDOM_HASH_SHIFT_B;
    value *= RANDOM_HASH_MULTIPLY_B;
    value ^= value >> RANDOM_HASH_SHIFT_A;

    return value;
}

float Random01(uint seed)
{
    return (float) (Hash(seed) & RANDOM_HASH_MASK) / RANDOM_HASH_DIVISOR;
}

float RandomRange(uint seed, float minValue, float maxValue)
{
    return minValue + (maxValue - minValue) * Random01(seed);
}

// リングバッファ形式の発生範囲内かを判定する。
bool IsEmitIndex(uint index, uint emitStart, uint emitCount, uint maxCount)
{
    if (emitCount == 0)
    {
        return false;
    }

    uint relative = (index + maxCount - emitStart) % maxCount;

    return relative < emitCount;
}

//===================================
// パーティクルごとの色設定
//===================================
float3 SelectFireWorkColor(float t)
{
    if (t < 0.25f)
    {
        return float3(1.0f, 0.45f, 0.15f);
    }

    if (t < 0.50f)
    {
        return float3(1.0f, 0.85f, 0.25f);
    }

    if (t < 0.75f)
    {
        return float3(0.35f, 0.90f, 1.0f);
    }

    return float3(1.0f, 0.95f, 0.75f);
}

float3 SelectFloatUpColor(float t)
{
    if (t < 0.40f)
    {
        return float3(0.35f, 0.90f, 1.0f);
    }

    if (t < 0.75f)
    {
        return float3(0.75f, 0.45f, 1.0f);
    }

    return float3(0.90f, 1.0f, 1.0f);
}

float3 SelectSparkColor(float t)
{
    if (t < 0.55f)
    {
        return float3(1.0f, 0.95f, 0.65f);
    }

    if (t < 0.85f)
    {
        return float3(1.0f, 0.55f, 0.15f);
    }

    return float3(1.0f, 1.0f, 1.0f);
}

float3 SelectLandingColor(float t)
{
    if (t < 0.45f)
    {
        return float3(0.20f, 1.00f, 0.55f);
    }

    if (t < 0.80f)
    {
        return float3(0.20f, 0.85f, 1.00f);
    }

    return float3(0.85f, 1.00f, 0.95f);
}

float3 SelectRainColor(float t)
{
    if (t < 0.50f)
    {
        return float3(0.45f, 0.80f, 1.0f);
    }

    return float3(0.75f, 0.95f, 1.0f);
}

//===================================
// ComputeShaderの実行単位
// 1グループあたり256スレッドで実行する。
// SV_DispatchThreadID.xをパーティクル番号として使い、
// 1スレッドが1パーティクルを更新・生成する。
//===================================
[numthreads(256, 1, 1)]
void main(uint3 dispatchId : SV_DispatchThreadID)
{
    uint index = dispatchId.x;

    uint emitCount = (uint) EmitParam.x;
    uint emitStart = (uint) EmitParam.y;
    uint emitMode = (uint) EmitParam.z;
    uint maxCount = (uint) EmitParam.w;

    if (index >= maxCount)
    {
        return;
    }

    float time = TimeParam.x;
    float deltaFrame = TimeParam.y;

    ParticleGpuData particle = Particles[index];

    //===================================
    // 生きているパーティクルを更新する。
    //===================================
    if (particle.Param.x > 0.5f && deltaFrame > 0.0f)
    {
        float swirl = particle.Param.y;
        float drag = particle.Param.z;
        float gravity = particle.Param.w;

        float s = sin(swirl * deltaFrame);
        float c = cos(swirl * deltaFrame);

        float vx = particle.VelocityMaxLife.x;
        float vz = particle.VelocityMaxLife.z;

        particle.VelocityMaxLife.x = vx * c - vz * s;
        particle.VelocityMaxLife.z = vx * s + vz * c;

        particle.VelocityMaxLife.x *= drag;
        particle.VelocityMaxLife.z *= drag;
        particle.VelocityMaxLife.y += gravity * deltaFrame;

        particle.PositionLife.xyz += particle.VelocityMaxLife.xyz * deltaFrame;
        particle.PositionLife.w -= deltaFrame;

        if (particle.PositionLife.w <= 0.0f)
        {
            particle.PositionLife.w = 0.0f;
            particle.Param.x = 0.0f;
        }
    }

    //===================================
    // 新しく発生させる粒子の状態を設定する。
    // 発生範囲はCPU側から渡されたemitStart / emitCountで決める。
    //===================================
    if (IsEmitIndex(index, emitStart, emitCount, maxCount))
    {
        uint localEmitIndex =
            (index + maxCount - emitStart) % maxCount;

        uint seedBase =
            index * RANDOM_INDEX_HASH ^
            localEmitIndex * RANDOM_LOCAL_INDEX_HASH ^
            emitStart * RANDOM_EMIT_START_HASH ^
            (uint) time * RANDOM_TIME_HASH ^
            emitMode * RANDOM_MODE_HASH;

        float life =
            RandomRange(
                seedBase ^ RANDOM_LIFE_OFFSET,
                LifeSizeParam.x,
                LifeSizeParam.y
            );

        float size =
            RandomRange(
                seedBase ^ RANDOM_SIZE_OFFSET,
                LifeSizeParam.z,
                LifeSizeParam.w
            );

        float velocityX = 0.0f;
        float velocityY = 0.0f;
        float velocityZ = 0.0f;

        float swirl = 0.0f;
        float drag = 1.0f;

        //===============================
        // パーティクルごとの計算パターン
        // emitModeによって花火・浮遊・火花・着地・雨の動きを切り替える。
        //===============================
        if (emitMode == EMIT_MODE_FIREWORK)
        {
            float angle = RandomRange(seedBase ^ RANDOM_ANGLE_OFFSET, PARTICLE_ANGLE_MIN, PARTICLE_ANGLE_MAX);
            float speed = RandomRange(seedBase ^ RANDOM_SPEED_OFFSET, VelocityParam.x, VelocityParam.y);

            velocityX = cos(angle) * speed;
            velocityY = RandomRange(seedBase ^ RANDOM_Y_OFFSET, VelocityParam.z, VelocityParam.w);
            velocityZ = sin(angle) * speed;

            swirl = RandomRange(seedBase ^ RANDOM_SWIRL_OFFSET, FIREWORK_SWIRL_MIN, FIREWORK_SWIRL_MAX);
            drag = RandomRange(seedBase ^ RANDOM_DRAG_OFFSET, FIREWORK_DRAG_MIN, FIREWORK_DRAG_MAX);
        }
        else if (emitMode == EMIT_MODE_FLOAT_UP)
        {
            float angle = RandomRange(seedBase ^ RANDOM_ANGLE_OFFSET, PARTICLE_ANGLE_MIN, PARTICLE_ANGLE_MAX);
            float speed = RandomRange(seedBase ^ RANDOM_SPEED_OFFSET, VelocityParam.x, VelocityParam.y);

            velocityX = cos(angle) * speed;
            velocityY = RandomRange(seedBase ^ RANDOM_Y_OFFSET, VelocityParam.z, VelocityParam.w);
            velocityZ = sin(angle) * speed;

            swirl = RandomRange(seedBase ^ RANDOM_SWIRL_OFFSET, FLOAT_UP_SWIRL_MIN, FLOAT_UP_SWIRL_MAX);
            drag = RandomRange(seedBase ^ RANDOM_DRAG_OFFSET, FLOAT_UP_DRAG_MIN, FLOAT_UP_DRAG_MAX);
        }
        else if (emitMode == EMIT_MODE_SPARK)
        {
            float angle = RandomRange(seedBase ^ RANDOM_ANGLE_OFFSET, PARTICLE_ANGLE_MIN, PARTICLE_ANGLE_MAX);
            float speed = RandomRange(seedBase ^ RANDOM_SPEED_OFFSET, VelocityParam.x, VelocityParam.y);

            velocityX = cos(angle) * speed + RandomRange(seedBase ^ RANDOM_X_OFFSET, SPARK_OFFSET_MIN, SPARK_OFFSET_MAX);
            velocityY = RandomRange(seedBase ^ RANDOM_Y_OFFSET, VelocityParam.z, VelocityParam.w);
            velocityZ = sin(angle) * speed + RandomRange(seedBase ^ RANDOM_Z_OFFSET, SPARK_OFFSET_MIN, SPARK_OFFSET_MAX);

            swirl = RandomRange(seedBase ^ RANDOM_SWIRL_OFFSET, SPARK_SWIRL_MIN, SPARK_SWIRL_MAX);
            drag = RandomRange(seedBase ^ RANDOM_DRAG_OFFSET, SPARK_DRAG_MIN, SPARK_DRAG_MAX);
        }
        else if (emitMode == EMIT_MODE_LANDING)
        {
            float angle = RandomRange(seedBase ^ RANDOM_ANGLE_OFFSET, PARTICLE_ANGLE_MIN, PARTICLE_ANGLE_MAX);
            float speed = RandomRange(seedBase ^ RANDOM_SPEED_OFFSET, VelocityParam.x, VelocityParam.y);

            velocityX = cos(angle) * speed + RandomRange(seedBase ^ RANDOM_X_OFFSET, LANDING_OFFSET_MIN, LANDING_OFFSET_MAX);
            velocityY = RandomRange(seedBase ^ RANDOM_Y_OFFSET, VelocityParam.z, VelocityParam.w);
            velocityZ = sin(angle) * speed + RandomRange(seedBase ^ RANDOM_Z_OFFSET, LANDING_OFFSET_MIN, LANDING_OFFSET_MAX);

            swirl = RandomRange(seedBase ^ RANDOM_SWIRL_OFFSET, LANDING_SWIRL_MIN, LANDING_SWIRL_MAX);
            drag = RandomRange(seedBase ^ RANDOM_DRAG_OFFSET, LANDING_DRAG_MIN, LANDING_DRAG_MAX);
        }
        else
        {
            float angle = RandomRange(seedBase ^ RANDOM_ANGLE_OFFSET, PARTICLE_ANGLE_MIN, PARTICLE_ANGLE_MAX);
            float horizontal = RandomRange(seedBase ^ RANDOM_SPEED_OFFSET, VelocityParam.x, VelocityParam.y);

            velocityX = cos(angle) * horizontal;
            velocityY = RandomRange(seedBase ^ RANDOM_Y_OFFSET, VelocityParam.z, VelocityParam.w);
            velocityZ = sin(angle) * horizontal;

            swirl = RandomRange(seedBase ^ RANDOM_SWIRL_OFFSET, RAIN_SWIRL_MIN, RAIN_SWIRL_MAX);
            drag = RandomRange(seedBase ^ RANDOM_DRAG_OFFSET, RAIN_DRAG_MIN, RAIN_DRAG_MAX);
        }

        //==================================
        // パーティクルごとの色パターンを決める。
        //==================================
        float colorSeed = Random01(seedBase ^ RANDOM_COLOR_OFFSET);
        float3 color = float3(1.0f, 1.0f, 1.0f);

        if (emitMode == EMIT_MODE_FIREWORK)
        {
            color = SelectFireWorkColor(colorSeed);
        }
        else if (emitMode == EMIT_MODE_FLOAT_UP)
        {
            color = SelectFloatUpColor(colorSeed);
        }
        else if (emitMode == EMIT_MODE_SPARK)
        {
            color = SelectSparkColor(colorSeed);
        }
        else if (emitMode == EMIT_MODE_LANDING)
        {
            color = SelectLandingColor(colorSeed);
        }
        else
        {
            color = SelectRainColor(colorSeed);
        }

        //==================================
        // 新しい粒子状態をParticleBufferへ書き戻す。
        // Particlesはregister(u0)のUAVなので、ここでGPU上のBufferへ保存される。
        //==================================
        particle.PositionLife = float4(EmitPosition.xyz, life);
        particle.VelocityMaxLife = float4(velocityX, velocityY, velocityZ, life);
        particle.ColorSize = float4(color, size);
        particle.Param = float4(1.0f, swirl, drag, GravityParam.x);
    }

    Particles[index] = particle;
}