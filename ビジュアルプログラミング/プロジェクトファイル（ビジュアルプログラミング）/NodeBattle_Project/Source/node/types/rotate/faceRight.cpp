#include "faceRight.h"
#include "player.h"

NodeBase* FaceRight::Execute()
{
    m_GameChara->RequestRotate(m_Type);

    return GetNext();
}