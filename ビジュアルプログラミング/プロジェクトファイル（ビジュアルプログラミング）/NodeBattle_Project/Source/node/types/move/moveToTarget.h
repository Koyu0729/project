#pragma once
#include "singleOutputNode.h"

class MoveToTarget : public SingleOutputNode
{
  public:
	MoveToTarget(int id)
		: SingleOutputNode(id, NodeCategory::Movement, NodeType::Movement_ToTarget)
	{
	}

	NodeBase *Execute() override;
};
