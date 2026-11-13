#include "faceLeft.h"
#include "player.h"

NodeBase* FaceLeft::Execute()
{
    m_GameChara->RequestRotate(m_Type);

    return GetNext();
}