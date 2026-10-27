#include "moveStop.h"
#include "player.h"

NodeBase* MoveStop::Execute()
{
    m_GameChara->RequestMove(m_Type);

    return GetNext();
}
