#pragma once
#include "singleOutputNode.h"

// その場でプレイヤー本体を回転させるノード
class SpinRotation : public SingleOutputNode
{
  public:
	SpinRotation(int id, NodeType type);

	NodeBase *Execute() override;

	// 設定した時間だけ、その場で左または右に回り続ける
	NodeStatus Tick(float dt) override;
};
