#pragma once
#include "singleOutputNode.h"

// 敵から逃げる
class MoveEscape : public SingleOutputNode
{
  public:
	MoveEscape(int id)
		: SingleOutputNode(id, NodeCategory::Movement, NodeType::Movement_Escape)
	{
	}

	NodeBase *Execute() override;
};
