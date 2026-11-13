#pragma once
#include "singleOutputNode.h"

// 敵の方向を向く、または反対向きまで回すノード
class TargetRotation : public SingleOutputNode
{
  public:
	TargetRotation(int id, NodeType type);

	// プレイヤーへ行動を渡して、次のノードを返す
	NodeBase *Execute() override;

	// 設定時間や条件を見ながら、ノードを進める
	NodeStatus Tick(float dt) override;
};
