#include "playerRotateList.h"
#include "player.h"

static constexpr float ROTATE_STEP = 0.05f; // ラジアン/フレーム（調整用）

void PlayerRotateList::PlayerFaceLeft()
{
    if (!m_Player) return;

    Vector3 rot = m_Player->GetRotation();

    // 少しずつ左へ回す
    rot.y -= ROTATE_STEP;
    m_Player->SetRotation(rot);
}

void PlayerRotateList::PlayerFaceRight()
{
    if (!m_Player) return;

    Vector3 rot = m_Player->GetRotation();

    // 少しずつ右へ回す
    rot.y += ROTATE_STEP;
    m_Player->SetRotation(rot);
}

void PlayerRotateList::PlayerTurnLeft()
{
    if (!m_Player) return;

    Vector3 rot = m_Player->GetRotation();

    rot.y -= ROTATE_STEP * 2.0f;
    m_Player->SetRotation(rot);
}

void PlayerRotateList::PlayerTurnRight()
{
    if (!m_Player) return;

    Vector3 rot = m_Player->GetRotation();

    rot.y += ROTATE_STEP * 2.0f;
    m_Player->SetRotation(rot);
}
