#pragma once
#include "singleOutputNode.h"

// 敵との距離を保ちながら、敵を中心に回るノード
class OrbitMovement : public SingleOutputNode
{
  public:
	OrbitMovement(int id, NodeType type);

	NodeBase *Execute() override;

	// 敵の方を向いたまま、設定した時間だけ敵の周りを回る
	NodeStatus Tick(float dt) override;
};
