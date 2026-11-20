#pragma once
#include "singleOutputNode.h"

class MoveStop : public SingleOutputNode
{
  public:
	MoveStop(int id)
		: SingleOutputNode(id, NodeCategory::Movement, NodeType::Movement_Stop)
	{
	}

	NodeBase *Execute() override;
};
