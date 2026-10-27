#pragma once
#include "singleOutputNode.h"

class MoveForward : public SingleOutputNode
{
  public:
	MoveForward(int id)
		: SingleOutputNode(id, NodeCategory::Movement, NodeType::Movement_Forward)
	{
	}

	NodeBase *Execute() override;
};
