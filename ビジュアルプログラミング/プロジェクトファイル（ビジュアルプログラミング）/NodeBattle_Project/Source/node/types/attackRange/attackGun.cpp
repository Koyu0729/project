#include "attackGun.h"
#include "player.h"

NodeBase* AttackGun::Execute()
{
    m_GameChara->RequestAttackRange(m_Type);

    return GetNext();
}
