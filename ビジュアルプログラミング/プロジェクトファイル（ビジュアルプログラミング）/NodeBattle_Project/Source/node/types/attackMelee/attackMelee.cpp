#include "attackMelee.h"
#include "player.h"

NodeBase *AttackMelee::Execute()
{
	if (m_GameChara)
	{
		m_GameChara->RequestAttackMelee(m_Type);
	}

	return GetNext();
}
