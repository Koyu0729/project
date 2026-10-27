#include "moveForward.h"
#include "player.h"

NodeBase* MoveForward::Execute()
{
    m_GameChara->RequestMove(m_Type);

    return GetNext();
}
