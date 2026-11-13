#pragma once
#include "nodeBase.h"
class BranchBase : public NodeBase
{
  protected:
	// 条件の結果と、「はい」「いいえ」で進む先
	bool m_BranchResult = false;

	NodeBase *m_TrueNext = nullptr;

	NodeBase *m_FalseNext = nullptr;

  public:
	BranchBase(int id, NodeType type)
		: NodeBase(id, NodeCategory::If, type)
	{
	}

	// 条件を調べて、「はい」か「いいえ」を決める
	virtual bool Branch() = 0;

	// プレイヤーへ行動を渡して、次のノードを返す
	NodeBase *Execute() override
	{
		m_BranchResult = Branch();

		return GetNext();
	}

	bool GetConditionResult() const
	{
		return m_BranchResult;
	}

	void SetNext(NodeBase *next, Port port = Port::OutDef) override
	{
		if (port == Port::OutTrue)
		{
			m_TrueNext = next;
		}
		else if (port == Port::OutFalse)
		{
			m_FalseNext = next;
		}
	}

	NodeBase *GetNext() const override
	{
		if (m_BranchResult)
		{
			return m_TrueNext;
		}
		else
		{
			return m_FalseNext;
		}
	}

	NodeBase *GetNextForPort(Port port) const override
	{
		if (port == Port::OutTrue)
		{
			return m_TrueNext;
		}

		if (port == Port::OutFalse)
		{
			return m_FalseNext;
		}

		return nullptr;
	}
};
