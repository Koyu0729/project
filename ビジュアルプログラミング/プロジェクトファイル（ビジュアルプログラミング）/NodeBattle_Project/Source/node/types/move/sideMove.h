#pragma once
#include "singleOutputNode.h"

// 向きを変えずに、左右へ移動するノード
class SideMove : public SingleOutputNode
{
  public:
	SideMove(int id, NodeType type);

	// ノードに指定された方向への横移動をプレイヤーに要求する
	NodeBase *Execute() override;
};
