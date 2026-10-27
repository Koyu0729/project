#pragma once
#include "branchBase.h"

class GameObject3D;

class IfFindObject : public BranchBase
{
private:

    GameObject3D* m_OldTarget = nullptr;

    int m_RemitFrame = 0;
public:
    IfFindObject(int id) : BranchBase(id, NodeType::If_FindObject)
    {
    }

    bool Branch() override;

};
