#include "main.h"
#include "guard.h"
#include "player.h"
#include <sstream>
#include <random>

// 防御ノードの種類を設定する
Guard::Guard(int id)
	: SingleOutputNode(id, NodeCategory::Action, NodeType::Action_Guard)
{
}

// 防御を終えたあとに続くノードを返す
NodeBase *Guard::Execute()
{
	return GetNext();
}
