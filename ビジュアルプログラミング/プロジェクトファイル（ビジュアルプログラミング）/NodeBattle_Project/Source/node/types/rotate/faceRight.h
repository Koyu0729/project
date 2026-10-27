#pragma once
#include "singleOutputNode.h"

class FaceRight : public SingleOutputNode
{
  public:
	FaceRight(int id)
		: SingleOutputNode(id, NodeCategory::Rotate, NodeType::Rotate_FaceRight)
	{
	}

	NodeBase *Execute() override;
};
