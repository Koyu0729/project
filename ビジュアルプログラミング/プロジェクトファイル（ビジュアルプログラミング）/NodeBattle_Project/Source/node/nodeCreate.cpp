// 実行用ノードと編集画面の表示を、それぞれ生成する
#include "nodeCreate.h"

#include "nodeIncludes.h"

#include "nodeEditor.h"

#include "manager.h"

NodeBase *NodeCreate::CreateLogic(NodeCategory category, NodeType type, int createId)
{
	NodeBase *logic = nullptr;

	int id = createId;

	if (category == NodeCategory::Movement && (type == NodeType::Movement_Left || type == NodeType::Movement_Right))
	{
		return new SideMove(id, type);
	}

	if (category == NodeCategory::Rotate && (type == NodeType::Rotate_ToTarget || type == NodeType::Rotate_TurnBack))
	{
		return new TargetRotation(id, type);
	}

	if (category == NodeCategory::If && (type == NodeType::If_TargetExists || type == NodeType::If_TargetFar || type == NodeType::If_TargetHP || type == NodeType::If_TargetAhead))
	{
		return new TargetCondition(id, type);
	}

	if (category == NodeCategory::Rotate && (type == NodeType::Rotate_SpinLeft || type == NodeType::Rotate_SpinRight))
	{
		return new SpinRotation(id, type);
	}

	if (category == NodeCategory::Movement && (type == NodeType::Movement_OrbitLeft || type == NodeType::Movement_OrbitRight))
	{
		return new OrbitMovement(id, type);
	}

	if (category == NodeCategory::If && (type == NodeType::If_TargetAttacking || type == NodeType::If_ObstacleAhead || type == NodeType::If_Random))
	{
		return new TacticalCondition(id, type);
	}

	if (category == NodeCategory::Movement && (type == NodeType::Movement_KeepDistance || type == NodeType::Movement_FlankLeft || type == NodeType::Movement_FlankRight))
	{
		return new TacticalMovement(id, type);
	}

	switch (category)
	{
	case NodeCategory::Start:

		if (type == NodeType::Start)
		{
			logic = new Start(id);
		}

		break;

	case NodeCategory::If:

		if (type == NodeType::If_FindObject)
		{
			logic = new IfFindObject(id);
		}
		else if (type == NodeType::If_Stats)
		{
			logic = new IfPlayerStats(id);
		}
		else if (type == NodeType::If_RangeIO)
		{
			logic = new IfRangeIO(id);
		}

		break;

	case NodeCategory::AttackMelee:

		if (type == NodeType::Attack_Melee || type == NodeType::Attack_Sword)
		{
			logic = new AttackMelee(id, type);
		}

		break;

	case NodeCategory::AttackRange:

		if (type == NodeType::Attack_Gun)
		{
			logic = new AttackGun(id);
		}

		break;

	case NodeCategory::Action:

		if (type == NodeType::Action_Wait || type == NodeType::Action_Search || type == NodeType::Action_Step)
		{
			logic = new TimedAction(id, type);
		}
		else if (type == NodeType::Action_Guard)
		{
			logic = new Guard(id);
		}

		break;

	case NodeCategory::Movement:

		if (type == NodeType::Movement_Forward)
		{
			logic = new MoveForward(id);
		}
		else if (type == NodeType::Movement_Back)
		{
			logic = new MoveBack(id);
		}
		else if (type == NodeType::Movement_Stop)
		{
			logic = new MoveStop(id);
		}
		else if (type == NodeType::Movement_ToTarget)
		{
			logic = new MoveToTarget(id);
		}
		else if (type == NodeType::Movement_Escape)
		{
			logic = new MoveEscape(id);
		}

		break;

	case NodeCategory::Rotate:

		if (type == NodeType::Rotate_FaceLeft)
		{
			logic = new FaceLeft(id);
		}
		else if (type == NodeType::Rotate_FaceRight)
		{
			logic = new FaceRight(id);
		}
		else if (type == NodeType::Rotate_TurnLeft)
		{
			logic = new TurnLeft(id);
		}
		else if (type == NodeType::Rotate_TurnRight)
		{
			logic = new TurnRight(id);
		}

		break;

	default:
		printf("まだ、そのノードの条件ないよ");

		break;
	}

	return logic;
}

// ノード本体と、画面の見た目を結びつける
NodeView *NodeCreate::CreateView(NodeBase *logic, Vector2 releasedPos)
{
	if (!logic)
	{
		return nullptr;
	}

	// NodeView作成
	NodeView *view = Manager::GetScene()->AddGameObject2D<NodeView>(4);

	view->Init(logic);

	view->SetPosition(releasedPos);

	return view;
}
