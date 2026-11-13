#pragma once
#include "nodeBase.h"

// 次の行動へ進む出口を1つ持つノード
class SingleOutputNode : public NodeBase
{
  protected:
	// 通常の出口に接続されているノード
	NodeBase *m_NextOutput = nullptr;

  public:
	// 行動カテゴリーと種類を設定する
	SingleOutputNode(int id, NodeCategory category, NodeType type)
		: NodeBase(id, category, type)
	{
	}

	// 通常の出口の接続先を設定する
	void SetNext(NodeBase *next, Port port = Port::OutDef) override
	{
		m_NextOutput = next;
	}

	// 次に実行するノードを取得する
	NodeBase *GetNext() const override
	{
		return m_NextOutput;
	}
};
