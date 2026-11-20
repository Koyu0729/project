#include "main.h"
#include "nodeBase.h"
#include "player.h"
#include <sstream>
#include <iomanip>

// このノードを始めるときに、時間と行動を準備する
void NodeBase::Begin()
{
	m_Elapsed = 0;

	m_TurnRemaining = m_Settings.angleDegrees * XM_PI / 180.0f;

	m_ExecutionDetail.clear();

	if (!m_GameChara)
	{
		return;
	}

	m_GameChara->ClearNodeCommands();

	if (m_Type == NodeType::Action_Guard)
	{
		m_GameChara->SetGuarding(true);
	}

	if (m_Category == NodeCategory::AttackMelee || m_Category == NodeCategory::AttackRange)
	{
		Execute();
	}
}

// 設定時間や条件を見ながら、ノードを進める
NodeStatus NodeBase::Tick(float dt)
{
	if (!m_GameChara)
	{
		m_ExecutionDetail = L"キャラクターがいません";

		return NodeStatus::Failed;
	}

	// 残っている時間だけ行動する（最後のフレームで動きすぎないように）
	const float remaining = (std::max)(0.0f, m_Settings.duration - m_Elapsed);

	const float used = (std::min)(dt, remaining);

	m_Elapsed += used;

	m_GameChara->SetNodeDeltaSeconds(used);

	if (m_Category == NodeCategory::Movement)
	{
		auto *target = m_GameChara->GetTargetObject();

		if (m_Type == NodeType::Movement_ToTarget || m_Type == NodeType::Movement_Escape)
		{
			if (!target)
			{
				m_ExecutionDetail = L"敵なし：次へ";

				return NodeStatus::Completed;
			}

			Vector3 offset = target->GetPosition() - m_GameChara->GetPosition();

			offset.y = 0;

			float length = offset.length();

			bool done;

			if (m_Type == NodeType::Movement_ToTarget)
			{
				done = length <= m_Settings.distance;
			}
			else
			{
				done = length >= m_Settings.distance;
			}

			if (done)
			{
				return NodeStatus::Completed;
			}

			m_GameChara->SetNodeMoveDistance(m_Settings.distance);
		}

		if (used > 0)
		{
			Execute();
		}
	}
	else if (m_Category == NodeCategory::Rotate)
	{
		const bool left = m_Type == NodeType::Rotate_FaceLeft || m_Type == NodeType::Rotate_TurnLeft;

		const bool fast = m_Type == NodeType::Rotate_TurnLeft || m_Type == NodeType::Rotate_TurnRight;

		float turnSpeed;

		if (fast)
		{
			turnSpeed = 6.0f;
		}
		else
		{
			turnSpeed = 3.0f;
		}

		const float turn = (std::min)(m_TurnRemaining, dt * turnSpeed);

		Vector3 rotation = m_GameChara->GetRotation();

		float turnAmount;

		if (left)
		{
			turnAmount = -turn;
		}
		else
		{
			turnAmount = turn;
		}

		rotation.y += turnAmount;

		m_GameChara->SetRotation(rotation);

		m_TurnRemaining -= turn;

		if (m_TurnRemaining <= 0.0001f)
		{
			return NodeStatus::Completed;
		}
		else
		{
			return NodeStatus::Running;
		}
	}
	else if (m_Type == NodeType::Action_Search)
	{
		Vector3 rotation = m_GameChara->GetRotation();

		rotation.y += used * 3.0f;

		m_GameChara->SetRotation(rotation);
	}
	else if (m_Type == NodeType::Action_Step && used > 0)
	{
		m_GameChara->RequestMove(NodeType::Movement_Forward);
	}

	if (m_Elapsed + 0.00001f >= m_Settings.duration)
	{
		return NodeStatus::Completed;
	}
	else
	{
		return NodeStatus::Running;
	}
}

// このノードの行動を終了し、停止時の状態も解除する
void NodeBase::End()
{
	if (m_GameChara)
	{
		m_GameChara->ClearNodeCommands();

		if (m_Type == NodeType::Action_Guard)
		{
			m_GameChara->SetGuarding(false);
		}

		if (m_Category == NodeCategory::AttackMelee)
		{
			m_GameChara->CancelMeleeAttack();
		}

		if (m_Category == NodeCategory::AttackRange)
		{
			m_GameChara->CancelRangeAttack();
		}
	}
}

// 実行中の説明に使う、設定値つきの名前を作る
std::wstring NodeBase::GetDisplayName() const
{
	std::wostringstream text;

	text << std::fixed << std::setprecision(1);

	if (m_Type == NodeType::If_Stats)
	{
		text << L"HPが" << m_Settings.hpPercent << L"%以下？";
	}
	else if (m_Type == NodeType::If_TargetHP)
	{
		text << L"敵HPが" << m_Settings.hpPercent << L"%以下？";
	}
	else if (m_Type == NodeType::If_TargetFar)
	{
		text << L"敵までの距離 " << m_Settings.distance << L" 以上？";
	}
	else if (m_Type == NodeType::If_TargetAhead)
	{
		text << L"敵が正面 " << m_Settings.angleDegrees << L"度内？";
	}
	else if (m_Type == NodeType::If_Random)
	{
		text << L"ランダム（はい " << m_Settings.hpPercent << L"%）";
	}
	else if (m_Type == NodeType::If_ObstacleAhead)
	{
		text << L"前方 " << m_Settings.distance << L" に障害物？";
	}
	else if (m_Type == NodeType::Rotate_ToTarget)
	{
		text << m_Name;
	}
	else if (m_Type == NodeType::If_RangeIO)
	{
		text << L"敵までの距離 " << m_Settings.distance << L" 以下？";
	}
	else if (m_Type == NodeType::Rotate_SpinLeft || m_Type == NodeType::Rotate_SpinRight)
	{
		text << m_Name << L" " << m_Settings.duration << L"秒";
	}
	else if (m_Category == NodeCategory::Rotate)
	{
		text << m_Name << L" " << m_Settings.angleDegrees << L"度";
	}
	else if (m_Category != NodeCategory::Start && m_Category != NodeCategory::If)
	{
		text << m_Name << L" " << m_Settings.duration << L"秒";
	}
	else
	{
		text << m_Name;
	}

	return text.str();
}

// ノードの中に表示する設定値の説明を作る
std::wstring NodeBase::GetParameterSummary() const
{
	std::wostringstream text;

	//小数点以下は1桁で表示する
	text << std::fixed << std::setprecision(1);

	if (m_Type == NodeType::If_Stats)
	{
		text << L"HP " << m_Settings.hpPercent << L"% 以下";
	}
	else if (m_Type == NodeType::If_TargetHP)
	{
		text << L"敵HP " << m_Settings.hpPercent << L"% 以下";
	}
	else if (m_Type == NodeType::If_TargetFar)
	{
		text << L"距離 " << m_Settings.distance << L" 以上";
	}
	else if (m_Type == NodeType::If_TargetAhead)
	{
		text << L"正面の扇形 " << m_Settings.angleDegrees << L"度";
	}
	else if (m_Type == NodeType::If_TargetExists)
	{
		text << L"距離に関係なく敵の有無";
	}
	else if (m_Type == NodeType::If_Random)
	{
		text << L"ランダム（はい " << m_Settings.hpPercent << L"%）";
	}
	else if (m_Type == NodeType::If_ObstacleAhead)
	{
		text << L"前方 " << m_Settings.distance << L" に障害物？";
	}
	else if (m_Type == NodeType::Rotate_ToTarget)
	{
		text << L"最短方向へ回転 / 敵なしは次へ";
	}
	else if (m_Type == NodeType::If_RangeIO)
	{
		text << L"距離 " << m_Settings.distance << L" 以下";
	}
	else if (m_Type == NodeType::If_FindObject)
	{
		text << L"索敵距離 12 / 全方向";
	}
	else if (m_Type == NodeType::Rotate_SpinLeft || m_Type == NodeType::Rotate_SpinRight)
	{
		text << m_Settings.duration << L"秒 / " << m_Settings.angleDegrees << L"度/秒";
	}
	else if (m_Type == NodeType::Attack_Melee)
	{
		text << L"中距離 / 構えてから突く";
	}
	else if (m_Type == NodeType::Attack_Gun)
	{
		text << L"距離1.4以上 / 構え0.3秒 / 軸固定";
	}
	else if (m_Type == NodeType::Attack_Sword)
	{
		text << L"近距離 / すぐ斬る";
	}
	else if (m_Type == NodeType::If_Random)
	{
		text << L"はい " << m_Settings.hpPercent << L"% / ↑↓で確率";
	}
	else if (m_Type == NodeType::If_ObstacleAhead)
	{
		text << L"前方距離 " << m_Settings.distance << L" / ←→で距離";
	}
	else if (m_Type == NodeType::If_TargetAttacking)
	{
		text << L"構え・攻撃中を検知";
	}
	else if (m_Type == NodeType::Movement_KeepDistance)
	{
		text << m_Settings.duration << L"秒 / 距離 " << m_Settings.distance << L" / ←→で距離";
	}
	else if (m_Type == NodeType::Movement_FlankLeft || m_Type == NodeType::Movement_FlankRight)
	{
		text << L"到着で終了 / 上限 " << m_Settings.duration << L"秒 / 距離 " << m_Settings.distance;
	}
	else if (m_Type == NodeType::Action_Guard)
	{
		text << m_Settings.duration << L"秒 / 正面120度 / 残量あり";
	}
	else if (m_Category == NodeCategory::Rotate)
	{
		text << m_Settings.angleDegrees << L"度";
	}
	else if (m_Type == NodeType::Movement_OrbitLeft || m_Type == NodeType::Movement_OrbitRight)
	{
		text << m_Settings.duration << L"秒 / 距離を保ち敵を向く";
	}
	else if (m_Category == NodeCategory::Start)
	{
		text << L"ここから開始";
	}
	else
	{
		text << m_Settings.duration << L"秒";

		if (m_Type == NodeType::Movement_ToTarget)
		{
			text << L"まで\n距離 " << m_Settings.distance << L" で停止";
		}

		if (m_Type == NodeType::Movement_Escape)
		{
			text << L"まで\n距離 " << m_Settings.distance << L" まで離れる";
		}
	}

	return text.str();
}
