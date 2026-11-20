#pragma once
#include "singleOutputNode.h"

class Start : public SingleOutputNode
{
  public:
	Start(int id)
		: SingleOutputNode(id, NodeCategory::Start, NodeType::Start)
	{
	}

	NodeBase *Execute() override;
};
