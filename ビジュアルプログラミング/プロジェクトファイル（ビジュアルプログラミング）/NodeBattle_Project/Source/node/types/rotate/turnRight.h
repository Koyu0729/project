#pragma once
#include "singleOutputNode.h"

class TurnRight : public SingleOutputNode
{
  public:
	TurnRight(int id)
		: SingleOutputNode(id, NodeCategory::Rotate, NodeType::Rotate_TurnRight)
	{
	}

	NodeBase *Execute() override;
};
