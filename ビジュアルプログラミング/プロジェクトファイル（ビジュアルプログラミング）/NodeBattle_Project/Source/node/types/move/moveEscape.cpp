#include "moveEscape.h"
#include "player.h"

NodeBase* MoveEscape::Execute()
{
    m_GameChara->RequestMove(m_Type);

    return GetNext();
}
