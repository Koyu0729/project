#pragma once
#include <unordered_map>
#include <vector>

// 以前の保存データを読み込めるよう、ノードの種類を表す数値を維持する。
enum class NodeCategory
{
	Category_Null = -1,
	Start,
	If,
	AttackMelee,
	AttackRange,
	Action,
	Movement,
	Rotate,
	Category_MAX
};

enum class NodeType
{
	Type_Null = -1,

	Start,

	If_FindObject,
	If_Stats,
	If_RangeIO,

	Attack_Melee,
	Attack_Gun,

	Action_Search,
	Action_Step,

	Movement_Forward,
	Movement_Back,
	Movement_Jump,
	Movement_Stop,
	Movement_ToTarget,
	Movement_Escape,

	Rotate_FaceLeft,
	Rotate_FaceRight,
	Rotate_TurnLeft,
	Rotate_TurnRight,

	Action_Wait,

	Movement_Left,
	Movement_Right,

	Rotate_ToTarget,
	Rotate_TurnBack,

	If_TargetExists,
	If_TargetFar,
	If_TargetHP,
	If_TargetAhead,

	Rotate_SpinLeft,
	Rotate_SpinRight,

	Movement_OrbitLeft,
	Movement_OrbitRight,

	Attack_Sword,

	Action_Guard,

	If_TargetAttacking,
	If_ObstacleAhead,
	If_Random,

	Movement_KeepDistance,
	Movement_FlankLeft,
	Movement_FlankRight,

	Type_MAX
};

enum class Port
{
	PortNull = -1,
	In,
	OutDef,
	OutTrue,
	OutFalse
};

struct NodeTypeName
{
	NodeType type;

	const wchar_t *name;
};

static const std::unordered_map<NodeCategory, const wchar_t *> CategoryNameTable = {
	{NodeCategory::Start, L"スタート"}, {NodeCategory::If, L"条件"}, {NodeCategory::AttackMelee, L"近接攻撃"}, {NodeCategory::AttackRange, L"射撃"}, {NodeCategory::Action, L"行動"}, {NodeCategory::Movement, L"移動"}, {NodeCategory::Rotate, L"向きを変える"}};

static const std::unordered_map<NodeCategory, std::vector<NodeTypeName>> NodeTable =
{
	{
		NodeCategory::Start,
		{
			{NodeType::Start, L"スタート"}
		}
	},

	{
		NodeCategory::If,
		{
			{NodeType::If_FindObject, L"敵が見つかった？"},
			{NodeType::If_Stats, L"HPが設定値以下？"},
			{NodeType::If_RangeIO, L"敵が近くにいる？"},
			{NodeType::If_TargetExists, L"敵がいる？"},
			{NodeType::If_TargetFar, L"敵が遠くにいる？"},
			{NodeType::If_TargetHP, L"敵HPが設定値以下？"},
			{NodeType::If_TargetAhead, L"敵が正面にいる？"},
			{NodeType::If_TargetAttacking, L"敵が攻撃中？"},
			{NodeType::If_ObstacleAhead, L"前に障害物がある？"},
			{NodeType::If_Random, L"ランダムに分岐する"}
		}
	},

	{
		NodeCategory::AttackMelee,
		{
			{NodeType::Attack_Melee, L"槍で突く"},
			{NodeType::Attack_Sword, L"剣で斬る"}
		}
	},

	{
		NodeCategory::AttackRange,
		{
			{NodeType::Attack_Gun, L"射撃する"}
		}
	},

	{
		NodeCategory::Action,
		{
			{NodeType::Action_Wait, L"待つ"},
			{NodeType::Action_Search, L"周囲を見回す"},
			{NodeType::Action_Guard, L"盾を構える"}
		}
	},

	{
		NodeCategory::Movement,
		{
			{NodeType::Movement_Forward, L"前進する"},
			{NodeType::Movement_Back, L"後退する"},
			{NodeType::Movement_Left, L"左へ横移動する"},
			{NodeType::Movement_Right, L"右へ横移動する"},
			{NodeType::Movement_Stop, L"止まる"},
			{NodeType::Movement_ToTarget, L"敵へ近づく"},
			{NodeType::Movement_Escape, L"敵から離れる"},
			{NodeType::Movement_OrbitLeft, L"敵の周りを回る（左）"},
			{NodeType::Movement_OrbitRight, L"敵の周りを回る（右）"},
			{NodeType::Movement_KeepDistance, L"指定した距離を保つ"},
			{NodeType::Movement_FlankLeft, L"敵の横へ回り込む（左）"},
			{NodeType::Movement_FlankRight, L"敵の横へ回り込む（右）"}
		}
	},

	{
		NodeCategory::Rotate,
		{
			{NodeType::Rotate_FaceLeft, L"左へ回す（ゆっくり）"},
			{NodeType::Rotate_FaceRight, L"右へ回す（ゆっくり）"},
			{NodeType::Rotate_TurnLeft, L"左へ回す（速く）"},
			{NodeType::Rotate_TurnRight, L"右へ回す（速く）"},
			{NodeType::Rotate_ToTarget, L"敵の方向を向く"},
			{NodeType::Rotate_TurnBack, L"反対向きまで回す"},
			{NodeType::Rotate_SpinLeft, L"自分が回る（左）"},
			{NodeType::Rotate_SpinRight, L"自分が回る（右）"}
		}
	}
};
