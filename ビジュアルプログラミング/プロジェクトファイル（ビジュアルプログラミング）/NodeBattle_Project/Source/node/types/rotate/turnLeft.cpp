#include "turnLeft.h"
#include "player.h"

NodeBase* TurnLeft::Execute()
{
    m_GameChara->RequestRotate(m_Type);

    return GetNext();
}