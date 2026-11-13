#include "attackMelee.h"
#include "player.h"

// プレイヤーへ行動を渡して、次のノードを返す
NodeBase *AttackMelee::Execute()
{
	if (m_GameChara)
	{
		m_GameChara->RequestAttackMelee(m_Type);
	}

	return GetNext();
}
