#pragma once
#include "singleOutputNode.h"

// 向きを変えずに、左右へ移動するノード
class SideMove : public SingleOutputNode
{
  public:
	SideMove(int id, NodeType type);

	// プレイヤーへ行動を渡して、次のノードを返す
	NodeBase *Execute() override;
};
