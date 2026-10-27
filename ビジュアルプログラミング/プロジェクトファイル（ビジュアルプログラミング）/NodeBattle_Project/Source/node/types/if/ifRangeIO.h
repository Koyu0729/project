// 相手が設定した距離以内にいるか判定する
#pragma once
#include "branchBase.h"

class IfRangeIO : public BranchBase
{
public:
    IfRangeIO(int id) : BranchBase(id, NodeType::If_RangeIO)
    {
    }

    bool Branch() override;

};

