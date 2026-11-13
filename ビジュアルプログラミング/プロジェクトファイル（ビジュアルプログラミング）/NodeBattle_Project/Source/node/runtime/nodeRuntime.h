#pragma once

#include "nodeBase.h"

enum class RuntimeState
{
	Stopped,
	Running,
	Paused,
	Completed,
	Error
};

struct NodeRouteStep
{
	int fromID;

	Port port;

	int toID;
};

class NodeRuntime
{

	// 実行対象のノード（所有権は呼び出し側が持つ）
	std::vector<NodeBase *> m_RunNodes;

	// スタート・今のノード・次に進むノード
	NodeBase *m_StartNode = nullptr;

	NodeBase *m_Current = nullptr;

	NodeBase *m_Next = nullptr;

	// 行動を始めたかと、次へ進むタイミング
	bool m_Begun = false;

	bool m_Advance = false;

	// 最後からスタートへ戻るかと、今の周回数
	bool m_Loop = true;

	unsigned long long m_Pass = 0;

	// 1ノードずつ進めるときのフラグ
	bool m_Step = false;

	// 実行中・一時停止・完了などの状態
	RuntimeState m_State = RuntimeState::Stopped;

	// 実行結果と、最後に通った条件の説明
	std::wstring m_Message;

	std::wstring m_LastCondition;

	// 最後に通った線と、今の周のルート
	int m_LastFromID = -1;

	Port m_LastPort = Port::PortNull;

	std::vector<NodeRouteStep> m_Route;

	// 今のノードを終えて、次に進む準備をする
	void FinishNode(NodeBase *next);

	// 配列の位置と訪問状態を使って、接続を確認する
	static int FindNodeIndex(const std::vector<NodeBase *> &nodes, NodeBase *node);
	static bool HasCycle(const std::vector<NodeBase *> &nodes, int index, std::vector<int> &visitState);

  public:

	// スタート・設定値・接続を確認して、実行できるか調べる
	static bool Validate(const std::vector<NodeBase *> &nodes, std::wstring &message);

	// スタートから実行を始める
	bool Start(bool loop = true);

	// 実行を止めて、残っている行動とルートを消す
	void Stop();

	// 今の場所で一時停止する
	void Pause();

	// 一時停止していたところから再開する
	void Resume();

	// 次の1ノードだけ進める
	void Step();

	// 毎フレームの処理
	void Update(float deltaSeconds);

	// 今の周回で通ったルートを取得する
	const std::vector<NodeRouteStep> &GetRoute() const
	{
		return m_Route;
	}

	// 実行側が使っているノードを確認する
	const std::vector<NodeBase *> &GetRunNodes() const
	{
		return m_RunNodes;
	}

	// 今の実行を止めて、使うノードを入れ替える
	void SetRunNodes(const std::vector<NodeBase *> &nodes)
	{
		Stop();

		m_RunNodes = nodes;

		m_StartNode = nullptr;
	}

	void SetLoop(bool loop)
	{
		m_Loop = loop;
	}

	bool IsLooping() const
	{
		return m_Loop;
	}

	unsigned long long GetPass() const
	{
		return m_Pass;
	}

	RuntimeState GetState() const
	{
		return m_State;
	}

	NodeBase *GetCurrentNode() const
	{
		return m_Current;
	}

	int GetCurrentID() const
	{
		if (m_Current)
		{
			return m_Current->GetID();
		}
		else
		{
			return -1;
		}
	}

	const std::wstring &GetMessage() const
	{
		return m_Message;
	}

	const std::wstring &GetLastCondition() const
	{
		return m_LastCondition;
	}

	int GetLastFromID() const
	{
		return m_LastFromID;
	}

	Port GetLastPort() const
	{
		return m_LastPort;
	}
};
