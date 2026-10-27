#include "moveBack.h"
#include "player.h"

NodeBase* MoveBack::Execute()
{
    m_GameChara->RequestMove(m_Type);

    return GetNext();
}
