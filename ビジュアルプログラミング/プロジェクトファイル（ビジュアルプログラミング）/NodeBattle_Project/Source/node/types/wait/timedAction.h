#pragma once
#include "singleOutputNode.h"

// 設定時間の待機・見回し・前進を扱う行動ノード
class TimedAction : public SingleOutputNode
{
  public:
	TimedAction(int id, NodeType type = NodeType::Action_Wait);

	// 次のノードを返す。継続動作はNodeBaseで処理する
	NodeBase *Execute() override;
};
