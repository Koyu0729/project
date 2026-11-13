#pragma once
#include "singleOutputNode.h"

// 敵との距離を保ちながら、敵を中心に回るノード
class OrbitMovement : public SingleOutputNode
{
  public:
	OrbitMovement(int id, NodeType type);

	// プレイヤーへ行動を渡して、次のノードを返す
	NodeBase *Execute() override;

	// 設定時間や条件を見ながら、ノードを進める
	NodeStatus Tick(float dt) override;
};
