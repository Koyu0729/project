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

SYACHI::SYACHI() : SYACHI(XMFLOAT3(0.0f, -2.0f, 0.0f), XMFLOAT3(0.0f, 0.0f, 0.0f))
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
    m_CupsuleCollision->CupsuleUpdate(m_Position);
    GetObjectManager()->GetCollisionManager()->CollisionMapRegist(m_CupsuleCollision);
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
        // 毎フレーム抽選して、当たったらジャンプする。
        if (std::uniform_int_distribution<int>(0, m_PopInterval - 1)(m_Random) == 1)
        {
            m_PopInterval = (std::max)(20, m_PopInterval - 10);
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
        if (++m_CooldownFrame >= 100)
        {
            m_Position.y = -2.0f;
            m_Angle = static_cast<float>(std::uniform_int_distribution<int>(0, 359)(m_Random));
            m_State = SYACHI_STATE_DEFAULT;
        }
        break;
    default:
        BeginCooldown();
        break;
    }
    // シャチの移動に合わせて、当たり判定の位置も動かす。
    m_CupsuleCollision->CupsuleUpdate(m_Position);
    GetObjectManager()->GetCollisionManager()->CollisionMapRegist(m_CupsuleCollision);
}

void SYACHI::Draw()
{
    ModelDraw(m_Position, m_Rotation, m_Scale, GetModel(KILLERWHALE));
}

void SYACHI::DrawMulti(int)
{
    Draw();
}

// 突進処理。現在の通常状態からはジャンプのみ
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
        // 最初のフレームだけ、移動を始める位置を覚えておく。
        m_StartPoint = m_Position;
        m_MovePoint = XMFLOAT3(
            m_Position.x + (dx * std::cos(angle) - dz * std::sin(angle)) * 100.0f, m_Position.y,
            m_Position.z + (dx * std::sin(angle) + dz * std::cos(angle)) * 100.0f);
        m_PathLength = 100.0f;
        m_Progress = 0.0f;
        m_MoveInitialized = true;
    }

    m_Progress = (std::min)(1.0f, m_Progress + 0.4f / m_PathLength);
    m_Position.x = m_StartPoint.x + (m_MovePoint.x - m_StartPoint.x) * m_Progress;
    m_Position.z = m_StartPoint.z + (m_MovePoint.z - m_StartPoint.z) * m_Progress;
    m_Rotation.y = -std::atan2(m_MovePoint.x - m_StartPoint.x, m_MovePoint.z - m_StartPoint.z) + XM_PI;
        
    OBJECTS objects = GetObjectManager()->GetObjects(TAG_GROUND);

    for (int i = 0; i < objects.GetCount(); ++i)
    {
        BLOCK *block = dynamic_cast<BLOCK *>(objects.GetGameObject(i));
        if (!block || block->m_State != BLOCK_STATE_DEFAULT)
        {
            continue;
        }
        const XMFLOAT3 p = block->GetPosition();
        const float dx = p.x - m_Position.x, dz = p.z - m_Position.z;
        if (dx * dx + dz * dz < m_Scale.z * m_Scale.z)
        {
            block->BlockDespawn(6 * 60);
        }
    }

    if (m_Progress >= 1.0f)
    {
        BeginCooldown();
    }
}

void SYACHI::SyachiJump()
{
    if (!m_MoveInitialized)
    {
        // 最初のフレームだけ、移動を始める位置を覚えておく。
        m_StartPoint = m_Position;
        m_MovePoint = XMFLOAT3(fieldMid.x, -12.0f, fieldMid.z);
        const float dx = m_MovePoint.x - m_StartPoint.x;
        const float dz = m_MovePoint.z - m_StartPoint.z;

        // 最初の距離を使う。距離が0でも割り算できるようにしておく。
        m_PathLength = (std::max)(0.4f, std::sqrt(dx * dx + dz * dz));
        m_Progress = 0.0f;
        m_MoveInitialized = true;
        m_Rotation.y = -std::atan2(dx, dz) + XM_PI;
    }
    m_Progress = (std::min)(1.0f, m_Progress + 0.4f / m_PathLength);
    m_Position.x = m_StartPoint.x + (m_MovePoint.x - m_StartPoint.x) * m_Progress;
    m_Position.z = m_StartPoint.z + (m_MovePoint.z - m_StartPoint.z) * m_Progress;

    // 山なりに飛んで、最後は水面の下まで沈む。
    m_Position.y = m_StartPoint.y + (m_MovePoint.y - m_StartPoint.y) * m_Progress +
                   40.0f * m_Progress * (1.0f - m_Progress);

    // 飛ぶ方向に合わせて、上がるときは頭を上げ、落ちるときは下げる。
    const float verticalTangent = m_MovePoint.y - m_StartPoint.y +
                                  40.0f * (1.0f - 2.0f * m_Progress);
    m_Rotation.x = -std::atan2(verticalTangent, m_PathLength);
    OBJECTS objects = GetObjectManager()->GetObjects(TAG_GROUND);

    // 床に近い高さで、横の距離も近ければ壊す。
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
        // 上空を通るだけでは床を壊さないよう、高さの差も調べる。
        const float heightDifference = std::abs(position.y - m_Position.y);

        if (dx * dx + dz * dz <= 1.0f && heightDifference <= 1.0f)
        {
            block->BlockDespawn(210);
        }
    }
    if (m_Progress >= 1.0f)
    {
        BeginCooldown();
    }
}
