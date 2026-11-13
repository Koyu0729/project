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
    SYACHI_STATE m_State = SYACHI_STATE_DEFAULT;  // 現在の行動状態（遊泳・突進・ジャンプ・待機）

    float m_Angle = 0.0f;                         // フィールド中央を回るときの角度（度）

    int m_JumpLotteryRange = 319;                 // ジャンプ抽選の候補数（1を引くとジャンプ）

    int m_CooldownFrame = 0;                      // 待機に入ってからの経過フレーム数

    bool m_MoveInitialized = false;               // ジャンプ・突進の開始位置と行き先を設定済みか

    XMFLOAT3 m_StartPoint = {};                   // ジャンプ・突進を始めた座標

    XMFLOAT3 m_MovePoint = {};                    // ジャンプ・突進の到達座標

    float m_Progress = 0.0f;                      // 移動の進行度（0.0で開始、1.0で到達）

    float m_PathLength = 0.0f;                    // 進行度の計算に使う水平方向の距離

    std::mt19937 m_Random;                        // ジャンプ抽選や移動方向の決定に使う乱数生成器

private:
    void SyachiTackle();
    void SyachiJump();
    void BeginCooldown();
    void BeginMove(XMFLOAT3 destination, float pathLength);
    void AdvanceMove();
    void BreakNearbyBlocks();
    void UpdateCollision();

public:
    SYACHI();
    SYACHI(XMFLOAT3 position, XMFLOAT3 rotation);
    ~SYACHI();
    void Update() override;
    void Draw() override;
    void DrawMulti(int playernum) override;
};
