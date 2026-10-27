#pragma once
#include "singleOutputNode.h"

class MoveBack : public SingleOutputNode
{
  public:
	MoveBack(int id)
		: SingleOutputNode(id, NodeCategory::Movement, NodeType::Movement_Back)
	{
	}

	NodeBase *Execute() override;
};
