#include "nodeRuntime.h"
#include "branchBase.h"
#include <algorithm>
// スタート・設定値・接続を確認して、実行できるか調べる
bool NodeRuntime::Validate(const std::vector<NodeBase *> &nodes, std::wstring &message)
{
	NodeBase *start = nullptr;
	int starts = 0;

	// ノード本体とIDの重複を、前に確認したノードと比較する
	for (int i = 0; i < static_cast<int>(nodes.size()); ++i)
	{
		NodeBase *node = nodes[i];

		if (!node || node->GetID() < 0)
		{
			message = L"ノードのIDまたはデータが不正です";
			return false;
		}

		for (int j = 0; j < i; ++j)
		{
			if (nodes[j] == node || nodes[j]->GetID() == node->GetID())
			{
				message = L"ノードのIDまたはデータが不正です";
				return false;
			}
		}

		if (!node->HasValidSettings())
		{
			message = L"設定値を確認してください：" + node->GetName();
			return false;
		}

		if (node->GetCategory() == NodeCategory::Start)
		{
			start = node;
			++starts;
		}
	}

	if (starts != 1)
	{
		message = L"スタートを1個置いてください";
		return false;
	}

	if (!start->GetNextForPort(Port::OutDef))
	{
		message = L"スタートから行動または条件へ線をつないでください";
		return false;
	}

	// 接続先が同じノード配列に含まれるかを先に調べる
	for (NodeBase *node : nodes)
	{
		for (Port port : {Port::OutDef, Port::OutTrue, Port::OutFalse})
		{
			NodeBase *next = node->GetNextForPort(port);

			if (!next)
			{
				continue;
			}

			if (FindNodeIndex(nodes, next) < 0 || next->GetCategory() == NodeCategory::Start)
			{
				message = L"無効な接続があります";
				return false;
			}
		}
	}

	// スタートにつながっていないノードも含め、循環を確認する
	std::vector<int> visitState(nodes.size(), 0);

	for (int i = 0; i < static_cast<int>(nodes.size()); ++i)
	{
		if (HasCycle(nodes, i, visitState))
		{
			message = L"線が循環しています。戻る線を外してください";
			return false;
		}
	}

	// スタートからたどれるノードを、配列の末尾から順に確認する
	std::vector<int> reached(nodes.size(), 0);
	std::vector<int> pending;
	pending.push_back(FindNodeIndex(nodes, start));
	int reachedCount = 0;

	while (!pending.empty())
	{
		int index = pending.back();
		pending.pop_back();

		if (reached[index] != 0)
		{
			continue;
		}

		reached[index] = 1;
		++reachedCount;
		NodeBase *node = nodes[index];

		if (node->GetCategory() == NodeCategory::If &&
			(!node->GetNextForPort(Port::OutTrue) || !node->GetNextForPort(Port::OutFalse)))
		{
			message = L"「はい」と「いいえ」の両方をつないでください：" + node->GetDisplayName();
			return false;
		}

		for (Port port : {Port::OutDef, Port::OutTrue, Port::OutFalse})
		{
			int nextIndex = FindNodeIndex(nodes, node->GetNextForPort(port));

			if (nextIndex >= 0 && reached[nextIndex] == 0)
			{
				pending.push_back(nextIndex);
			}
		}
	}

	if (reachedCount == static_cast<int>(nodes.size()))
	{
		message = L"実行できます";
	}
	else
	{
		message = L"実行できます（使われていないノードがあります）";
	}

	return true;
}

// スタートから実行を始める
bool NodeRuntime::Start(bool loop)
{

	Stop();

	if (!Validate(m_RunNodes, m_Message))
	{
		m_State = RuntimeState::Error;

		return false;
	}

	for (auto *node : m_RunNodes)
	{
		if (node->GetCategory() == NodeCategory::Start)
		{
			m_StartNode = node;
		}
	}

	m_Loop = loop;

	m_Pass = 1;

	m_Current = m_StartNode;

	m_State = RuntimeState::Running;

	m_LastCondition.clear();

	m_LastFromID = -1;

	m_LastPort = Port::PortNull;

	return true;
}

// 実行を止めて、残っている行動とルートを消す
void NodeRuntime::Stop()
{

	if (m_Current && m_Begun)
	{
		m_Current->End();
	}

	m_Current = nullptr;

	m_Next = nullptr;

	m_Begun = false;

	m_Advance = false;

	m_Step = false;

	m_Route.clear();

	m_Pass = 0;

	m_State = RuntimeState::Stopped;

	m_Message = L"停止中";

	m_LastFromID = -1;

	m_LastPort = Port::PortNull;
}

// 今の場所で一時停止する
void NodeRuntime::Pause()
{

	if (m_State == RuntimeState::Running)
	{
		m_State = RuntimeState::Paused;

		m_Step = false;

		m_Message = L"一時停止中";
	}
}

// 一時停止していたところから再開する
void NodeRuntime::Resume()
{
	if (m_State == RuntimeState::Paused)
	{
		m_State = RuntimeState::Running;

		m_Message = L"実行中";
	}
}

// 次の1ノードだけ進める
void NodeRuntime::Step()
{
	if (m_State == RuntimeState::Paused)
	{
		m_State = RuntimeState::Running;

		m_Step = true;
	}
}

// 毎フレームの処理
void NodeRuntime::Update(float dt)
{

	// 1回の更新時間は正の値で、最大1時間までを受け付ける
	if (m_State != RuntimeState::Running || !(dt > 0 && dt <= 3600.0f))
	{
		return;
	}

	dt = (std::min)(dt, 0.1f);

	if (m_Advance)
	{

		if (m_Current && m_Begun)
		{
			m_Current->End();
		}

		m_Begun = false;

		m_Advance = false;

		m_Current = m_Next;

		if (!m_Current)
		{

			if (!m_Loop)
			{
				m_State = RuntimeState::Completed;

				m_Message = L"テスト完了";

				return;
			}

			++m_Pass;

			m_Route.clear();

			m_LastCondition.clear();

			m_Current = m_StartNode;
		}
	}

	// 不正な接続で処理が止まらないよう、1回の更新で即時に進むノードを64個までに制限する。

	for (int transitions = 0; transitions < 64; ++transitions)
	{

		if (!m_Current)
		{
			return;
		}

		if (m_Current->GetCategory() == NodeCategory::Start || m_Current->GetCategory() == NodeCategory::If)
		{

			auto *next = m_Current->Execute();

			m_LastFromID = m_Current->GetID();

			m_LastPort = Port::OutDef;

			if (auto *branch = dynamic_cast<BranchBase *>(m_Current))
			{

				if (branch->GetConditionResult())
				{
					m_LastPort = Port::OutTrue;
				}
				else
				{
					m_LastPort = Port::OutFalse;
				}

				m_LastCondition = branch->GetExecutionDetail();
			}

			int nextID = -1;

			if (next)
			{
				nextID = next->GetID();
			}

			m_Route.push_back({m_LastFromID, m_LastPort, nextID});

			if (m_Step || !next)
			{
				FinishNode(next);

				return;
			}

			m_Current = next;

			continue;
		}

		if (!m_Begun)
		{
			m_Current->Begin();

			m_Begun = true;
		}

		NodeStatus status = m_Current->Tick(dt);

		if (status == NodeStatus::Failed)
		{

			m_Message = m_Current->GetExecutionDetail();

			m_Current->End();

			m_Begun = false;

			m_State = RuntimeState::Error;

			return;
		}

		if (status == NodeStatus::Completed)
		{

			m_LastFromID = m_Current->GetID();

			m_LastPort = Port::OutDef;

			auto *next = m_Current->GetNext();

			int nextID = -1;

			if (next)
			{
				nextID = next->GetID();
			}

			m_Route.push_back({m_LastFromID, m_LastPort, nextID});

			FinishNode(next);
		}
		else
		{
			m_Message = L"実行中：" + m_Current->GetDisplayName();
		}

		return; // プレイヤーがこの行動を処理してから、次のノードへ進む。
	}

	m_Message = L"1回の更新で進むノードが多すぎます";

	m_State = RuntimeState::Error;
}

// 今のノードを終えて、次に進む準備をする
void NodeRuntime::FinishNode(NodeBase *next)
{

	m_Next = next;

	m_Advance = true;

	if (m_Step)
	{
		m_State = RuntimeState::Paused;

		m_Step = false;

		m_Message = L"1ノード完了：一時停止中";
	}
}

// ノードのポインターに対応する配列の位置を探す
int NodeRuntime::FindNodeIndex(const std::vector<NodeBase *> &nodes, NodeBase *node)
{
	for (int i = 0; i < static_cast<int>(nodes.size()); ++i)
	{
		if (nodes[i] == node)
		{
			return i;
		}
	}

	return -1;
}

// 確認中のノードに戻ったら循環。確認済みへの合流は許可する
bool NodeRuntime::HasCycle(const std::vector<NodeBase *> &nodes, int index, std::vector<int> &visitState)
{
	if (visitState[index] == 1)
	{
		return true;
	}

	if (visitState[index] == 2)
	{
		return false;
	}

	visitState[index] = 1;

	for (Port port : {Port::OutDef, Port::OutTrue, Port::OutFalse})
	{
		NodeBase *next = nodes[index]->GetNextForPort(port);
		int nextIndex = FindNodeIndex(nodes, next);

		if (nextIndex >= 0 && HasCycle(nodes, nextIndex, visitState))
		{
			return true;
		}
	}

	visitState[index] = 2;
	return false;
}
