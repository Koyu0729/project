#pragma once
#include <vector>
#include <string>
#include "nodeBase.h"
struct NodeConnectionSaveData
{
	int fromID;

	int toID;

	Port fromPort;
};

class NodeManager
{

	// 置いてあるノードと、保存するときの接続情報
	std::vector<NodeBase *> m_Nodes;

	std::vector<NodeConnectionSaveData> m_SaveData;

	// 新しく置くノードの番号
	int m_AddNextID = 0;

	// 接続処理の結果を表示するメッセージ
	std::wstring m_Message;

  public:
	// 空の配列へノードを渡す。以降は受け取った側がノードを削除する
	NodeManager() = default;

	NodeManager(const NodeManager &) = delete;

	~NodeManager()
	{
		ClearLogic();
	}

	void TransferNodesTo(std::vector<NodeBase *> &destination)
	{
		m_Nodes.swap(destination);
		m_SaveData.clear();
		m_AddNextID = 0;
	}

	// 実行側へノードを渡す。NodeManagerにはノードを残さない
	std::vector<NodeBase *> ReleaseNodes()
	{
		std::vector<NodeBase *> nodes;
		TransferNodesTo(nodes);
		return nodes;
	}

	NodeManager &operator=(const NodeManager &) = delete;

	// 新しいノードに使う番号を作る
	int CreateNodeID()
	{
		return m_AddNextID++;
	}

	// 置いたノードを登録する
	void RegisterNode(NodeBase *node);

	// 接続できるか確認して、ノード同士をつなぐ
	bool RegisterConnect(NodeBase *from, NodeBase *to, Port fromPort);

	// 消すノードにつながっている線も外す
	void UnregisterConnect(NodeBase *node);

	// 管理しているノード本体をすべて解放する
	void ClearLogic();

	// 接続情報を確認するために出力する
	void PrintConnections() const;

	// 番号からノード本体を探す
	NodeBase *GetNodeUseID(int id) const;

	const std::vector<NodeBase *> &GetNodes() const
	{
		return m_Nodes;
	}

	const std::vector<NodeConnectionSaveData> &GetSaveConnections() const
	{
		return m_SaveData;
	}

	const std::wstring &GetMessage() const
	{
		return m_Message;
	}
};
