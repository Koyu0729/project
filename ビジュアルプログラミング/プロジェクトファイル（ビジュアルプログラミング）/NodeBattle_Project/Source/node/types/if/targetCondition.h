#pragma once
#include "branchBase.h"

// 敵の有無・距離・HP・正面にいるかを調べる条件
class TargetCondition : public BranchBase
{
  public:
	TargetCondition(int id, NodeType type);

	// 条件を調べて、「はい」か「いいえ」を決める
	bool Branch() override;
};
