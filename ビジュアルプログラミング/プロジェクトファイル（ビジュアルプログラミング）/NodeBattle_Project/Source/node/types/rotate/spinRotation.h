#pragma once
#include "singleOutputNode.h"

// その場でプレイヤー本体を回転させるノード
class SpinRotation : public SingleOutputNode
{
  public:
	SpinRotation(int id, NodeType type);

	// プレイヤーへ行動を渡して、次のノードを返す
	NodeBase *Execute() override;

	// 設定時間や条件を見ながら、ノードを進める
	NodeStatus Tick(float dt) override;
};
