#include "main.h"
#include "sideMove.h"
#include "player.h"
#include <sstream>
#include <random>

// ノードの設定を初期化する
SideMove::SideMove(int id, NodeType type)
	: SingleOutputNode(id, NodeCategory::Movement, type)
{
}

// プレイヤーへ行動を渡して、次のノードを返す
NodeBase *SideMove::Execute()
{
	if (m_GameChara)
	{
		m_GameChara->RequestMove(m_Type);
	}

	return GetNext();
}
