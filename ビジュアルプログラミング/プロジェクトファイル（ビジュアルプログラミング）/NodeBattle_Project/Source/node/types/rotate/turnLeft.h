#pragma once
#include "singleOutputNode.h"

class TurnLeft : public SingleOutputNode
{
  public:
	TurnLeft(int id)
		: SingleOutputNode(id, NodeCategory::Rotate, NodeType::Rotate_TurnLeft)
	{
	}

	NodeBase *Execute() override;
};
