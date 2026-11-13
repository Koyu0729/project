#include "directx.h"
#include "game.h"
#include "block.h"
#include "model.h"
#include "model_resource.h"
#include "collision_cupsule_hoge.h"
#include "syachi.h"
#include <algorithm>
#include <cmath>

static const XMFLOAT3 fieldMid = {25.0f, 0.0f, 25.0f};
static constexpr float swimY = -2.0f;
static constexpr float movePerFrame = 0.4f;
static constexpr float jumpCurve = 40.0f;
static constexpr int cooldownFrames = 100;
static constexpr float tackleDistance = 100.0f;
static constexpr float jumpBreakRadius = 1.0f;
static constexpr float jumpBreakHeight = 1.0f;
static constexpr int jumpBlockRespawnFrames = 210;
static constexpr int tackleBlockRespawnFrames = 6 * 60;

SYACHI::SYACHI() : SYACHI(XMFLOAT3(0.0f, swimY, 0.0f), XMFLOAT3(0.0f, 0.0f, 0.0f))
{
}

SYACHI::SYACHI(XMFLOAT3 position, XMFLOAT3 rotation) : m_Random(std::random_device{}())
{
    m_Position = position;
    m_Rotation = rotation;
    m_Scale = XMFLOAT3(1.0f, 1.0f, 1.0f);
    m_Tag = TAG_SYACHI;
    m_CupsuleCollision = new HOGE_CUPSULE_COLLISION(m_Position);
    m_CupsuleCollision->m_Tag.SetTag(TAG_SYACHI);
    UpdateCollision();
}

SYACHI::~SYACHI()
{
    delete m_CupsuleCollision;
}

// 次のジャンプで位置を計算し直せるようにして、待機に切り替える。
void SYACHI::BeginCooldown()
{
    m_MoveInitialized = false;
    m_Progress = 0.0f;
    m_CooldownFrame = 0;
    m_State = SYACHI_STATE_COOLDOWN;
}

void SYACHI::Update()
{
    switch (m_State)
    {
    case SYACHI_STATE_DEFAULT:
    {
        m_Rotation.x = 0.0f;
        const float radian = m_Angle * XM_PI / 180.0f;
        m_Position.x = fieldMid.x + 50.0f * std::cos(radian);
        m_Position.z = fieldMid.z + 50.0f * std::sin(radian);
        m_Rotation.y = radian - XM_PI;
        m_Angle = std::fmod(m_Angle + 0.1f, 360.0f);
        // ジャンプするたびに抽選の範囲を狭め、次のジャンプが出やすくなる。
        if (std::uniform_int_distribution<int>(0, m_JumpLotteryRange - 1)(m_Random) == 1)
        {
            m_JumpLotteryRange = (std::max)(20, m_JumpLotteryRange - 10);
            m_MoveInitialized = false;
            m_State = SYACHI_STATE_JUMP;
        }
        break;
    }
    case SYACHI_STATE_TACKLE:
        SyachiTackle();
        break;
    case SYACHI_STATE_JUMP:
        SyachiJump();
        break;
    case SYACHI_STATE_COOLDOWN:
        // 水面下で少し待ってから、周りを泳ぐ動きに戻る。
        if (++m_CooldownFrame >= cooldownFrames)
        {
            m_Position.y = swimY;
            m_Angle = static_cast<float>(std::uniform_int_distribution<int>(0, 359)(m_Random));
            m_State = SYACHI_STATE_DEFAULT;
        }
        break;
    default:
        BeginCooldown();
        break;
    }
    UpdateCollision();
}

void SYACHI::Draw()
{
    ModelDraw(m_Position, m_Rotation, m_Scale, GetModel(KILLERWHALE));
}

void SYACHI::DrawMulti(int)
{
    Draw();
}

void SYACHI::UpdateCollision()
{
    m_CupsuleCollision->CupsuleUpdate(m_Position);
    GetObjectManager()->GetCollisionManager()->CollisionMapRegist(m_CupsuleCollision);
}

void SYACHI::BeginMove(XMFLOAT3 destination, float pathLength)
{
    m_StartPoint = m_Position;
    m_MovePoint = destination;
    m_PathLength = pathLength;
    m_Progress = 0.0f;
    m_MoveInitialized = true;
    m_Rotation.y = -std::atan2(m_MovePoint.x - m_StartPoint.x,
                              m_MovePoint.z - m_StartPoint.z) + XM_PI;
}

void SYACHI::AdvanceMove()
{
    m_Progress = (std::min)(1.0f, m_Progress + movePerFrame / m_PathLength);
    m_Position.x = m_StartPoint.x + (m_MovePoint.x - m_StartPoint.x) * m_Progress;
    m_Position.z = m_StartPoint.z + (m_MovePoint.z - m_StartPoint.z) * m_Progress;
}

void SYACHI::BreakNearbyBlocks()
{
    const bool jumping = m_State == SYACHI_STATE_JUMP;
    const float radius = jumping ? jumpBreakRadius : m_Scale.z;
    const float radiusSquared = radius * radius;
    const int respawnFrames = jumping ? jumpBlockRespawnFrames : tackleBlockRespawnFrames;
    OBJECTS objects = GetObjectManager()->GetObjects(TAG_GROUND);

    for (int i = 0; i < objects.GetCount(); ++i)
    {
        BLOCK *block = dynamic_cast<BLOCK *>(objects.GetGameObject(i));
        if (!block || block->m_State != BLOCK_STATE_DEFAULT)
        {
            continue;
        }

        const XMFLOAT3 position = block->GetPosition();
        const float dx = position.x - m_Position.x;
        const float dz = position.z - m_Position.z;
        const float distanceSquared = dx * dx + dz * dz;
        // ジャンプ中は床の上を通過するため、高さも判定する。
        const bool hit = jumping
            ? distanceSquared <= radiusSquared && std::abs(position.y - m_Position.y) <= jumpBreakHeight
            : distanceSquared < radiusSquared;
        if (hit)
        {
            block->BlockDespawn(respawnFrames);
        }
    }
}

// 通常の遊泳からはジャンプへ遷移する。突進は専用の状態で使う。
void SYACHI::SyachiTackle()
{
    if (!m_MoveInitialized)
    {
        float dx = fieldMid.x - m_Position.x;
        float dz = fieldMid.z - m_Position.z;
        const float length = std::sqrt(dx * dx + dz * dz);
        if (length <= 0.0001f)
        {
            BeginCooldown();
            return;
        }
        dx /= length;
        dz /= length;
        const float angle = std::uniform_int_distribution<int>(-20, 19)(m_Random) * XM_PI / 180.0f;
        const float cosAngle = std::cos(angle);
        const float sinAngle = std::sin(angle);
        const XMFLOAT3 destination(
            m_Position.x + (dx * cosAngle - dz * sinAngle) * tackleDistance,
            m_Position.y,
            m_Position.z + (dx * sinAngle + dz * cosAngle) * tackleDistance);
        BeginMove(destination, tackleDistance);
    }

    AdvanceMove();
    BreakNearbyBlocks();
    if (m_Progress >= 1.0f)
    {
        BeginCooldown();
    }
}

void SYACHI::SyachiJump()
{
    if (!m_MoveInitialized)
    {
        const XMFLOAT3 destination(fieldMid.x, -12.0f, fieldMid.z);
        const float dx = destination.x - m_Position.x;
        const float dz = destination.z - m_Position.z;
        // 真下へ飛ぶ場合も、進行度の計算でゼロ除算しない。
        const float pathLength = (std::max)(movePerFrame, std::sqrt(dx * dx + dz * dz));
        BeginMove(destination, pathLength);
    }

    AdvanceMove();

    m_Position.y = m_StartPoint.y + (m_MovePoint.y - m_StartPoint.y) * m_Progress + jumpCurve * m_Progress * (1.0f - m_Progress);

    // 放物線の接線に合わせて、ジャンプ中の頭の向きを変える。
    const float verticalTangent = m_MovePoint.y - m_StartPoint.y + jumpCurve * (1.0f - 2.0f * m_Progress);

    m_Rotation.x = -std::atan2(verticalTangent, m_PathLength);

    BreakNearbyBlocks();
    if (m_Progress >= 1.0f)
    {
        BeginCooldown();
    }
}
