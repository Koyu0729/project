#pragma once

#include "nodeList.h"
#include <vector>

class Player;
class NodeRuntime;
class PlayerMovementList;
class PlayerRotateList;
class NodeBase;

// プレイヤーのノード実行と行動要求
class PlayerNodeControl
{
	// 操作対象のプレイヤー（ここでは削除しない）
	Player &m_Player;

	// 担当する実行状態・リソース
	NodeRuntime *m_NodeRun = nullptr;

	PlayerMovementList *m_MovementList = nullptr;

	PlayerRotateList *m_RotateList = nullptr;

	NodeType m_Melee = NodeType::Type_Null;

	NodeType m_Range = NodeType::Type_Null;

	NodeType m_Action = NodeType::Type_Null;

	NodeType m_Movement = NodeType::Type_Null;

	NodeType m_Rotate = NodeType::Type_Null;

	std::vector<NodeBase *> m_RunNodes = {};

	std::vector<NodeCategory> m_Categories = {};

	float m_NodeDeltaSeconds = 1.0f / 60.0f;

	float m_NodeMoveDistance = 1.4f;

	bool m_NodeLoop = true;

	bool m_RuntimeStarted = false;

	bool m_CanStartNode = false;

  public:
	explicit PlayerNodeControl(Player &player);

	// ノードの実行と、行動用のクラスを用意する
	void InitNodeLogic();

	// ノードの実行と、行動用のクラスを解放する
	void UninitNodeLogic();

	// 実行するノードを入れ替えて、プレイヤーと結びつける
	void SetRunNodes(std::vector<NodeBase *> nodes);

	// 前のフレームの行動が残らないように消す
	void ClearNodeCommands();

	// このフレームで受け取った行動の要求を解除する
	void CommandAction();

	// ノードから受け取った移動を実行する
	void CommandMove();

	// ノードから受け取った回転を実行する
	void CommandRotate();

	// 行動の分類を追加する
	void AddCategory(NodeCategory value);

	// ノードの実行管理を取得する
	NodeRuntime *GetNodeRun() const;

	// 近接攻撃の要求を取得する
	NodeType GetMelee() const;

	// 近接攻撃の要求を設定する
	void SetMelee(NodeType value);

	// 射撃の要求を取得する
	NodeType GetRange() const;

	// 射撃の要求を設定する
	void SetRange(NodeType value);

	// 行動の要求を取得する
	NodeType GetAction() const;

	// 行動の要求を設定する
	void SetAction(NodeType value);

	// 移動の要求を取得する
	NodeType GetMovement() const;

	// 移動の要求を設定する
	void SetMovement(NodeType value);

	// 回転の要求を取得する
	NodeType GetRotate() const;

	// 回転の要求を設定する
	void SetRotate(NodeType value);

	// 実行用のノード一覧を取得する
	const std::vector<NodeBase *> &GetRunNodes() const;

	// 行動に使う経過時間を取得する
	float GetNodeDeltaSeconds() const;

	// 行動に使う経過時間を設定する
	void SetNodeDeltaSeconds(float value);

	// 敵に近づくときの停止距離を取得する
	float GetNodeMoveDistance() const;

	// 敵に近づくときの停止距離を設定する
	void SetNodeMoveDistance(float value);

	// ノードの繰り返し設定を取得する
	bool GetNodeLoop() const;

	// ノードの繰り返し設定を設定する
	void SetNodeLoop(bool value);

	// ノード実行を開始したかどうかを取得する
	bool GetRuntimeStarted() const;

	// ノード実行を開始したかどうかを設定する
	void SetRuntimeStarted(bool value);

	// ノードを実行できるかどうかを取得する
	bool GetCanStartNode() const;

	// ノードを実行できるかどうかを設定する
	void SetCanStartNode(bool value);
};
