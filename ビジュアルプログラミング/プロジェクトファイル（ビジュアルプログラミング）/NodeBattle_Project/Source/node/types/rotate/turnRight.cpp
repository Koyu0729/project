#include "turnRight.h"
#include "player.h"

NodeBase* TurnRight::Execute()
{
    m_GameChara->RequestRotate(m_Type);

    return GetNext();
}
