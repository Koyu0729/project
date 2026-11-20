#pragma once
#include "singleOutputNode.h"

class FaceLeft : public SingleOutputNode
{
  public:
	FaceLeft(int id)
		: SingleOutputNode(id, NodeCategory::Rotate, NodeType::Rotate_FaceLeft)
	{
	}

	NodeBase *Execute() override;
};
