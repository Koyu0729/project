#include "main.h"
#include "sideMove.h"
#include "player.h"
#include <sstream>
#include <random>

SideMove::SideMove(int id, NodeType type)
	: SingleOutputNode(id, NodeCategory::Movement, type)
{
}

NodeBase *SideMove::Execute()
{
	if (m_GameChara)
	{
		m_GameChara->RequestMove(m_Type);
	}

	return GetNext();
}
