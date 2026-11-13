#pragma once
#include "branchBase.h"

class IfPlayerStats : public BranchBase
{
public:
    IfPlayerStats(int id) : BranchBase(id, NodeType::If_Stats)
    {
    }

    bool Branch() override;

};

