#include "main.h"
#include "player.h"
#include "renderer.h"
#include "modelRenderer.h"
#include "manager.h"
#include "stageObstacle.h"
#include "field.h"
#include "arenaBounds.h"
#include "nodeRuntime.h"
#include <algorithm>
#include <cmath>

// 初期化処理を行う
void Player::Init()
{
	m_Draw.Init();

	SetScale(Vector3(0.3f, 0.3f, 0.3f));

	if (m_Team == Team::TeamA)
	{
		m_Position = Vector3(0.0f, 1.5f, 0.0f);
	}
	else
	{
		m_Position = Vector3(0.0f, 1.5f, 1.0f);
	}

#pragma region プレイヤー実行処理の初期化
	InitNodeLogic();
#pragma endregion

}

// 終了処理を行う
void Player::Uninit()
{
	UninitNodeLogic();

	m_Draw.Uninit();
}

// 毎フレームの処理
void Player::Update()
{
	// 停止中も異常な座標を戻し、フィールドの外へ出たままにしない
	Field *arena;

	if (Manager::GetScene())
	{
		arena = Manager::GetScene()->GetGameObject<Field>();
	}
	else
	{
		arena = nullptr;
	}

	if (arena && arena->GetVisible())
	{
		m_Position = ArenaBounds::Constrain(m_Position);
	}

	ClearNodeCommands();

	// フレームの長さを使って、移動と攻撃を秒で進める
	const float dt = GetFrameDeltaSeconds();

	m_NodeControl.SetNodeDeltaSeconds(dt);

	if (m_Stats.hp <= 0)
	{
		if (m_NodeControl.GetNodeRun())
		{
			m_NodeControl.GetNodeRun()->Stop();
		}

		return;
	}

	if (!m_NodeControl.GetCanStartNode())
	{
		return;
	}

	if (!m_NodeControl.GetRuntimeStarted() && !m_NodeControl.GetRunNodes().empty())
	{
		m_NodeControl.GetNodeRun()->SetRunNodes(m_NodeControl.GetRunNodes());

		m_NodeControl.GetNodeRun()->Start(m_NodeControl.GetNodeLoop());

		m_NodeControl.SetRuntimeStarted(true);
	}

	if (m_NodeControl.GetNodeRun()->GetState() != RuntimeState::Running)
	{
		return;
	}

	m_Attacks.Update(dt);

	UpdateGuard(dt);

	m_OldPos = m_Position;

	m_NodeControl.GetNodeRun()->Update(dt);

	if (m_NodeControl.GetMelee() != NodeType::Type_Null)
	{
		CommandAttackMelee();
	}

	if (m_NodeControl.GetRange() != NodeType::Type_Null)
	{
		CommandAttackRange();
	}

	if (m_NodeControl.GetAction() != NodeType::Type_Null)
	{
		CommandAction();
	}

	if (m_NodeControl.GetMovement() != NodeType::Type_Null)
	{
		CommandMove();
	}

	if (m_NodeControl.GetRotate() != NodeType::Type_Null)
	{
		CommandRotate();
	}

	// ノードで移動した後、壁に当たった分だけ位置を戻す
	if (Manager::GetScene())
	{
		for (auto *wall : Manager::GetScene()->GetGameObjects<StageObstacle>())
		{
			if (!wall->GetVisible())
			{
				continue;
			}

			m_Position = wall->ResolveMovement(m_OldPos, m_Position);
		}
	}

	// 高速移動・ジャンプ・角への移動でも、四方の壁の内側に収める
	if (arena && arena->GetVisible())
	{
		m_Position = ArenaBounds::Constrain(m_Position);
	}

	auto *current = m_NodeControl.GetNodeRun()->GetCurrentNode();

	bool aiming = current && ((current->GetCategory() == NodeCategory::Rotate && current->GetType() != NodeType::Rotate_SpinLeft && current->GetType() != NodeType::Rotate_SpinRight) || current->GetType() == NodeType::Action_Search);

	bool tacticalAiming = current && (current->GetType() == NodeType::Movement_KeepDistance || current->GetType() == NodeType::Movement_FlankLeft || current->GetType() == NodeType::Movement_FlankRight || current->GetType() == NodeType::Action_Guard);

	m_Attacks.UpdateSword(!aiming && !tacticalAiming && !m_Attacks.IsMeleePending() && (!current || current->GetCategory() != NodeCategory::AttackMelee && current->GetCategory() != NodeCategory::AttackRange));

	ClearNodeCommands();
}

// 描画の担当クラスに処理を渡す
void Player::Draw()
{
	m_Draw.Draw();
}

// ノード制御の担当クラスに処理を渡す
void Player::InitNodeLogic()
{
	m_NodeControl.InitNodeLogic();
}

// ノード制御の担当クラスに処理を渡す
void Player::UninitNodeLogic()
{
	m_NodeControl.UninitNodeLogic();
}

// ノード制御の担当クラスに処理を渡す
void Player::SetRunNodes(std::vector<NodeBase *> nodes)
{
	m_NodeControl.SetRunNodes(std::move(nodes));
}

// ノード制御の担当クラスに処理を渡す
void Player::ClearNodeCommands()
{
	m_NodeControl.ClearNodeCommands();
}

// ノード制御の担当クラスに処理を渡す
void Player::CommandRotate()
{
	m_NodeControl.CommandRotate();
}

// ノード制御の担当クラスに処理を渡す
void Player::CommandMove()
{
	m_NodeControl.CommandMove();
}

// ノード制御の担当クラスに処理を渡す
void Player::CommandAction()
{
	m_NodeControl.CommandAction();
}

// 要求された武器の攻撃を管理クラスへ渡す
void Player::CommandAttackMelee()
{
	m_Attacks.StartMelee(m_NodeControl.GetMelee());
	m_NodeControl.SetMelee(NodeType::Type_Null);
}

// 要求された射撃を管理クラスへ渡す
void Player::CommandAttackRange()
{
	m_Attacks.StartRange(m_NodeControl.GetRange());
}

// HPからダメージ分を引く
void Player::TakeDamage(int num, const Vector3 *attackOrigin)
{
	// 攻撃が来た方向を見て、正面の盾だけで防ぐ（横と背後は通す）
	if (m_Stats.Shield && attackOrigin)
	{
		Vector3 incoming = *attackOrigin - m_Position;

		incoming.y = 0;

		float distance = incoming.length();

		if (distance > 0.0001f && Vector3::dot(incoming * (1.0f / distance), GetForward()) >= CombatBalance::ShieldFrontDot)
		{
			return;
		}
	}

	m_Stats.hp -= num;

	if (m_Stats.hp < 0)
	{
		m_Stats.hp = 0;
	}
}

void Player::ResetForTest()
{
	if (m_NodeControl.GetNodeRun())
	{
		m_NodeControl.GetNodeRun()->Stop();
	}

	ClearNodeCommands();

	m_NodeControl.SetRuntimeStarted(false);

	m_Stats.hp = m_Stats.maxHp;

	m_GuardEnergy = CombatBalance::ShieldCapacity;

	SetGuarding(false);

	CancelRangeAttack();

	m_If = {};

	m_Attacks.Reset();

	CancelMeleeAttack();

	m_Position = Vector3(0, 1.5f, 0);

	m_Rotation = Vector3(0, 0, 0);
}

// 実行終了時に武器の状態を戻す
void Player::ResetAttacks()
{
	m_Attacks.Reset();
}

// 盾は連続ノードでも残量を共有し、構え直して持続時間をリセットしない
void Player::UpdateGuard(float dt)
{
	if (m_GuardRequested)
	{
		m_GuardEnergy = (std::max)(0.0f, m_GuardEnergy - dt);

		m_Stats.Shield = m_GuardEnergy > 0.00001f;
	}
	else
	{
		m_GuardEnergy = (std::min)(CombatBalance::ShieldCapacity, m_GuardEnergy + dt * CombatBalance::ShieldRecoveryPerSecond);
	}
}

bool Player::HasVisibleTarget() const
{
	auto *target = GetTargetObject();

	if (!target)
	{
		return false;
	}

	Vector3 offset = target->GetPosition() - m_Position;

	offset.y = 0;
	// 全方向を対象に、距離12以内の敵を検出する。
	return offset.length() <= 12.0f;
}

bool Player::HasObstacleAhead(float distance) const
{
	if (!Manager::GetScene())
	{
		return false;
	}

	Vector3 forward(std::sin(m_Rotation.y), 0, std::cos(m_Rotation.y));

	Vector3 destination = m_Position + forward * distance;

	for (auto *wall : Manager::GetScene()->GetGameObjects<StageObstacle>())
	{
		if (wall->GetVisible() && wall->BlocksPath(m_Position, destination, 0.3f))
		{
			return true;
		}
	}

	return false;
}

// 実際に動いている攻撃だけを条件へ伝える（停止後の射撃状態は残さない）
bool Player::IsAttacking() const
{
	if (!m_NodeControl.GetCanStartNode() || !m_NodeControl.GetNodeRun() || (m_NodeControl.GetNodeRun()->GetState() != RuntimeState::Running && m_NodeControl.GetNodeRun()->GetState() != RuntimeState::Paused))
	{
		return false;
	}

	return m_Attacks.IsAttacking();
}

// 武器表示に使う長さを取得する
float Player::GetMeleeExtension() const
{
	return m_Attacks.GetMeleeExtension();
}

// 武器表示に使う向きを取得する
float Player::GetMeleeSwingHeading() const
{
	return m_Attacks.GetMeleeSwingHeading();
}

// 武器の接触判定を管理クラスへ委譲する
bool Player::SwordTouches(Player *target, const Vector3 &origin, float heading, float length) const
{
	return m_Attacks.SwordTouches(target, origin, heading, length);
}

GameObject3D *Player::GetTargetObject() const
{
	if (!m_TargetObject || !Manager::GetScene())
	{
		return nullptr;
	}

	// 相手がシーン内に存在するかをアドレスで確認し、破棄済みの相手を参照しない。

	for (auto *object : Manager::GetScene()->GetGameObjects<GameObject3D>())
	{
		if (object != m_TargetObject)
		{
			continue;
		}

		if (object->Destroy())
		{
			return nullptr;
		}

		auto *player = dynamic_cast<Player *>(object);

		if (player && player->GetHP() <= 0)
		{
			return nullptr;
		}
		else
		{
			return object;
		}
	}

	return nullptr;
}

// 描画に必要な武器の状態を読み取る
const AttackController &Player::GetAttackController() const
{
	return m_Attacks;
}
