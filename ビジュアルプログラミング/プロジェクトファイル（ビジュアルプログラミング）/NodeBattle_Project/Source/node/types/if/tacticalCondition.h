#pragma once
#include "branchBase.h"

// 敵の攻撃、前方の壁、ランダムな選択を「はい」「いいえ」に分ける
class TacticalCondition : public BranchBase
{
  public:
	// 条件の種類を保存し、標準の距離や確率を使う
	TacticalCondition(int id, NodeType type);

	// 分岐に到達するたびに一度だけ条件を決める
	bool Branch() override;
};
