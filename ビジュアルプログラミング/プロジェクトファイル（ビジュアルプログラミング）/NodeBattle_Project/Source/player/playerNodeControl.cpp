#include "main.h"
#include "player.h"
#include "playerNodeControl.h"
#include "manager.h"
#include "stageObstacle.h"
#include "nodeRuntime.h"
#include "playerRotateList.h"
#include "playerMovementList.h"
// 操作するプレイヤーを受け取る
PlayerNodeControl::PlayerNodeControl(Player &player)
	: m_Player(player)
{
}

// ノードの実行と、行動用のクラスを用意する
void PlayerNodeControl::InitNodeLogic()
{
	m_NodeRun = new NodeRuntime();

	m_RotateList = new PlayerRotateList(&m_Player);

	m_MovementList = new PlayerMovementList(&m_Player);
}

// ノードの実行と、行動用のクラスを解放する
void PlayerNodeControl::UninitNodeLogic()
{
	m_Player.ResetAttacks();

	if (m_NodeRun)
	{
		m_NodeRun->Stop();
	}

	for (auto *node : m_RunNodes)
	{
		delete node;
	}

	m_RunNodes.clear();

	delete m_RotateList;

	m_RotateList = nullptr;

	delete m_MovementList;

	m_MovementList = nullptr;

	delete m_NodeRun;

	m_NodeRun = nullptr;
}

// 実行するノードを入れ替えて、プレイヤーと結びつける
void PlayerNodeControl::SetRunNodes(std::vector<NodeBase *> nodes)
{
	if (m_NodeRun)
	{
		m_NodeRun->Stop();
	}

	for (auto *node : m_RunNodes)
	{
		delete node;
	}

	m_RunNodes = std::move(nodes);

	for (auto *node : m_RunNodes)
	{
		if (node)
		{
			node->SetPlayer(&m_Player);
		}
	}

	if (m_NodeRun)
	{
		m_NodeRun->SetRunNodes(m_RunNodes);
	}

	m_RuntimeStarted = false;
}

// 前のフレームの行動が残らないように消す
void PlayerNodeControl::ClearNodeCommands()
{
	m_Melee = m_Range = m_Action = m_Movement = m_Rotate = NodeType::Type_Null;

	m_Categories.clear();
}

// このフレームで受け取った行動の要求を解除する
void PlayerNodeControl::CommandAction()
{
	m_Action = NodeType::Type_Null;
}

// ノードから受け取った移動を実行する
void PlayerNodeControl::CommandMove()
{
	if (!m_MovementList)
	{
		return;
	}

	switch (m_Movement)
	{
	case NodeType::Movement_Forward:
		m_MovementList->PlayerForward();

		break;

	case NodeType::Movement_Back:
		m_MovementList->PlayerBack();

		break;

	case NodeType::Movement_Left:
		m_MovementList->PlayerSide(-1);

		break;

	case NodeType::Movement_Right:
		m_MovementList->PlayerSide(1);

		break;

	case NodeType::Movement_Stop:
		m_MovementList->PlayerStop();

		break;

	case NodeType::Movement_ToTarget:
		m_MovementList->PlayerToTarget();

		break;

	case NodeType::Movement_Escape:
		m_MovementList->PlayerEscape();

		break;
	}

	m_Movement = NodeType::Type_Null;
}

// ノードから受け取った回転を実行する
void PlayerNodeControl::CommandRotate()
{
	if (!m_RotateList)
	{
		return;
	}

	switch (m_Rotate)
	{
	case NodeType::Rotate_FaceLeft:
		m_RotateList->PlayerFaceLeft();

		break;

	case NodeType::Rotate_FaceRight:
		m_RotateList->PlayerFaceRight();

		break;

	case NodeType::Rotate_TurnLeft:
		m_RotateList->PlayerTurnLeft();

		break;

	case NodeType::Rotate_TurnRight:
		m_RotateList->PlayerTurnRight();

		break;
	}

	m_Rotate = NodeType::Type_Null;
}

// 行動の分類を追加する
void PlayerNodeControl::AddCategory(NodeCategory value)
{
	m_Categories.push_back(value);
}

// ノードの実行管理を取得する
NodeRuntime *PlayerNodeControl::GetNodeRun() const
{
	return m_NodeRun;
}

// 近接攻撃の要求を取得する
NodeType PlayerNodeControl::GetMelee() const
{
	return m_Melee;
}

// 近接攻撃の要求を設定する
void PlayerNodeControl::SetMelee(NodeType value)
{
	m_Melee = value;
}

// 射撃の要求を取得する
NodeType PlayerNodeControl::GetRange() const
{
	return m_Range;
}

// 射撃の要求を設定する
void PlayerNodeControl::SetRange(NodeType value)
{
	m_Range = value;
}

// 行動の要求を取得する
NodeType PlayerNodeControl::GetAction() const
{
	return m_Action;
}

// 行動の要求を設定する
void PlayerNodeControl::SetAction(NodeType value)
{
	m_Action = value;
}

// 移動の要求を取得する
NodeType PlayerNodeControl::GetMovement() const
{
	return m_Movement;
}

// 移動の要求を設定する
void PlayerNodeControl::SetMovement(NodeType value)
{
	m_Movement = value;
}

// 回転の要求を取得する
NodeType PlayerNodeControl::GetRotate() const
{
	return m_Rotate;
}

// 回転の要求を設定する
void PlayerNodeControl::SetRotate(NodeType value)
{
	m_Rotate = value;
}

// 実行用のノード一覧を取得する
const std::vector<NodeBase *> &PlayerNodeControl::GetRunNodes() const
{
	return m_RunNodes;
}

// 行動に使う経過時間を取得する
float PlayerNodeControl::GetNodeDeltaSeconds() const
{
	return m_NodeDeltaSeconds;
}

// 行動に使う経過時間を設定する
void PlayerNodeControl::SetNodeDeltaSeconds(float value)
{
	m_NodeDeltaSeconds = value;
}

// 敵に近づくときの停止距離を取得する
float PlayerNodeControl::GetNodeMoveDistance() const
{
	return m_NodeMoveDistance;
}

// 敵に近づくときの停止距離を設定する
void PlayerNodeControl::SetNodeMoveDistance(float value)
{
	m_NodeMoveDistance = value;
}

// ノードの繰り返し設定を取得する
bool PlayerNodeControl::GetNodeLoop() const
{
	return m_NodeLoop;
}

// ノードの繰り返し設定を設定する
void PlayerNodeControl::SetNodeLoop(bool value)
{
	m_NodeLoop = value;
}

// ノード実行を開始したかどうかを取得する
bool PlayerNodeControl::GetRuntimeStarted() const
{
	return m_RuntimeStarted;
}

// ノード実行を開始したかどうかを設定する
void PlayerNodeControl::SetRuntimeStarted(bool value)
{
	m_RuntimeStarted = value;
}

// ノードを実行できるかどうかを取得する
bool PlayerNodeControl::GetCanStartNode() const
{
	return m_CanStartNode;
}

// ノードを実行できるかどうかを設定する
void PlayerNodeControl::SetCanStartNode(bool value)
{
	m_CanStartNode = value;
}
