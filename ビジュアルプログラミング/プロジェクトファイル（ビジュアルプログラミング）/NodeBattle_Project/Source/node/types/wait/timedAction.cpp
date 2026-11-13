#include "main.h"
#include "timedAction.h"
#include "player.h"
#include <sstream>
#include <random>

// ノードの設定を初期化する
TimedAction::TimedAction(int id, NodeType type)
	: SingleOutputNode(id, NodeCategory::Action, type)
{
}

// 次のノードを返す。継続動作はNodeBaseで処理する
NodeBase *TimedAction::Execute()
{
	return GetNext();
}
