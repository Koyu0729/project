#include "moveToTarget.h"
#include "player.h"

NodeBase* MoveToTarget::Execute()
{
    m_GameChara->RequestMove(m_Type);

    return GetNext();
}
