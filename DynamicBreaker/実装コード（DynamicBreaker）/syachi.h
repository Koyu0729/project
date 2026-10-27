#pragma once
#include "object.h"
#include <random>

enum SYACHI_STATE
{
    SYACHI_STATE_DEFAULT,
    SYACHI_STATE_TACKLE,
    SYACHI_STATE_JUMP,
    SYACHI_STATE_COOLDOWN,
    SYACHI_STATE_MAX
};

class SYACHI : public OBJECT
{
  private:
    // シャチごとに状態を持たせて、ほかのシャチと混ざらないようにする。
    SYACHI_STATE m_State = SYACHI_STATE_DEFAULT;
    float m_Angle = 0.0f;
    int m_PopInterval = 319;
    int m_CooldownFrame = 0;
    bool m_MoveInitialized = false;
    XMFLOAT3 m_StartPoint = {};
    XMFLOAT3 m_MovePoint = {};
    float m_Progress = 0.0f;
    float m_PathLength = 0.0f;
    std::mt19937 m_Random;
    void SyachiTackle();
    void SyachiJump();
    void BeginCooldown();

  public:
    SYACHI();
    SYACHI(XMFLOAT3 position, XMFLOAT3 rotation);
    ~SYACHI();
    void Update() override;
    void Draw() override;
    void DrawMulti(int playernum) override;
};
