#pragma once
#include "singleOutputNode.h"

// 敵の方向を向く、または反対向きまで回すノード
class TargetRotation : public SingleOutputNode
{
  public:
	TargetRotation(int id, NodeType type);

	NodeBase *Execute() override;

	// 敵を向くか、設定した角度まで回ると完了する
	NodeStatus Tick(float dt) override;
};
