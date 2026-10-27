#include "nodeManager.h"
#include <algorithm>
#include <unordered_set>
#include <cstdio>

// 置いたノードを登録する
void NodeManager::RegisterNode(NodeBase *node)
{
	if (!node || GetNodeUseID(node->GetID()))
	{
		return;
	}

	m_Nodes.push_back(node);

	m_AddNextID = (std::max)(m_AddNextID, node->GetID() + 1);
}

// 接続できるか確認して、ノード同士をつなぐ
bool NodeManager::RegisterConnect(NodeBase *from, NodeBase *to, Port port)
{
	if (!from || !to || from == to || GetNodeUseID(from->GetID()) != from || GetNodeUseID(to->GetID()) != to || to->GetCategory() == NodeCategory::Start)
	{
		m_Message = L"出力から入力へつないでください";

		return false;
	}

	// 条件ノードは「はい・いいえ」、それ以外は通常の出力だけを使う
	bool invalidPort;

	if (from->GetCategory() == NodeCategory::If)
	{
		invalidPort = port != Port::OutTrue && port != Port::OutFalse;
	}
	else
	{
		invalidPort = port != Port::OutDef;
	}

	if (invalidPort)
	{
		m_Message = L"出力から入力へつないでください";

		return false;
	}

	std::vector<NodeBase *> pending{to};

	std::unordered_set<NodeBase *> visited;

	while (!pending.empty())
	{
		auto *node = pending.back();

		pending.pop_back();

		if (node == from)
		{
			m_Message = L"線を戻す接続はできません。終端で自動的に繰り返します";

			return false;
		}

		if (!visited.insert(node).second)
		{
			continue;
		}

		for (Port out : {Port::OutDef, Port::OutTrue, Port::OutFalse})
		{
			if (auto *next = node->GetNextForPort(out))
			{
				pending.push_back(next);
			}
		}
	}

	// 指定した出力の接続だけを置き換える。複数のノードから同じ行動へ接続できる。
	m_SaveData.erase(std::remove_if(m_SaveData.begin(), m_SaveData.end(),
									[from, port](const NodeConnectionSaveData &c)
									{
										return c.fromID == from->GetID() && c.fromPort == port;
									}),
					 m_SaveData.end());

	from->SetNext(to, port);

	m_SaveData.push_back({from->GetID(), to->GetID(), port});

	m_Message = L"接続しました";

	return true;
}

// 消すノードにつながっている線も外す
void NodeManager::UnregisterConnect(NodeBase *selected)
{
	if (!selected || GetNodeUseID(selected->GetID()) != selected)
	{
		return;
	}

	for (auto *node : m_Nodes)
	{
		for (Port port : {Port::OutDef, Port::OutTrue, Port::OutFalse})
		{
			if (node == selected || node->GetNextForPort(port) == selected)
			{
				node->SetNext(nullptr, port);
			}
		}
	}

	const int id = selected->GetID();

	m_SaveData.erase(std::remove_if(m_SaveData.begin(), m_SaveData.end(), [id](const NodeConnectionSaveData &c)
									{
										return c.fromID == id || c.toID == id;
									}),
					 m_SaveData.end());

	m_Nodes.erase(std::remove(m_Nodes.begin(), m_Nodes.end(), selected), m_Nodes.end());

	delete selected;
}

// 管理しているノード本体をすべて解放する
void NodeManager::ClearLogic()
{
	for (auto *node : m_Nodes)
	{
		delete node;
	}

	m_Nodes.clear();

	m_SaveData.clear();

	m_AddNextID = 0;

	m_Message.clear();
}

// 接続情報を確認するために出力する
void NodeManager::PrintConnections() const
{
	for (const auto &c : m_SaveData)
	{
		printf("%d (%d) -> %d\n", c.fromID, (int)c.fromPort, c.toID);
	}
}

// 番号からノード本体を探す
NodeBase *NodeManager::GetNodeUseID(int id) const
{
	for (auto *node : m_Nodes)
	{
		if (node && node->GetID() == id)
		{
			return node;
		}
	}

	return nullptr;
}
