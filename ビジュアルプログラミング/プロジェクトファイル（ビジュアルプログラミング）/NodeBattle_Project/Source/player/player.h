#pragma once

#include "gameObject3D.h"
#include "kosuCRI.h"

#include "nodeList.h"
#include "combatBalance.h"
#include "attackController.h"
#include "playerNodeControl.h"
#include "playerDraw.h"
#include <algorithm>

class ModelRenderer;

class NodeBase;

class NodeRuntime;

class PlayerRotateList;

class PlayerMovementList;

enum class Team
{
	Team_Null = -1,

	TeamA,
	TeamB,
	TeamC,

	Team_Max
};

class Player : public GameObject3D 
{

	// 行動要求の処理と描画の担当
	PlayerNodeControl m_NodeControl{*this};
	PlayerDraw m_Draw{*this};

  private:
	struct If
	{
		bool findObject = false;

		bool rio = false;

		bool stats = false;
	};

	struct Execute
	{
		bool attackMelee = false;

		bool attackRange = false;

		bool action = false;

		bool move = false;

		bool rotate = false;
	};

	struct Stats
	{
		int maxHp = 100;	// 最大HP
		int hp = maxHp;		// HP
		int maxFuel = 100;	// 制限時間
		int fuel = maxFuel; // 残り制限時間

		float speed = 0.02f;	 // 移動速度
		float moveStable = 1.0f; // 移動速度係数（立地が悪いと遅くなる）

		float aimSpeed = 0.05f; // 照準を合わせる速さの設定値（現在は未使用）
		float armStable = 1.0f; // 照準の安定性の設定値（現在は未使用）

		// 盾ノードを実行している間だけ、正面からの攻撃を防ぐ
		bool Shield = false;
	};

	Stats m_Stats;

	// 武器の動作・命中判定・共通クールタイム
	AttackController m_Attacks{*this};

	If m_If;

	Execute m_Execute;

	// チーム
	Team m_Team = Team::Team_Null;

	GameObject3D *m_TargetObject = nullptr;

	// 盾を構えたい状態と、残りの持続時間
	bool m_GuardRequested = false;

	float m_GuardEnergy = CombatBalance::ShieldCapacity;

	// 盾の残り時間を減らし、下ろしている間は回復する
	void UpdateGuard(float dt);

	// 前のフレームの位置
	Vector3 m_OldPos = {};

  public:
	// ADX用
	SePlayParams m_params;					// 再生パラメータ
	CriAtomEx3dSourceHn m_Source = nullptr; // 3Dソースハンドル
	CriAtomExPlaybackId m_SourceId = -1;	// 再生ID
	// 初期化処理を行う
	void Init() override;

	// 終了処理を行う
	void Uninit() override;

	// 毎フレームの処理
	void Update() override;

	// 画面に描く処理
	void Draw() override;

	// ノードの実行と、行動用のクラスを用意する
	void InitNodeLogic();

	// ノードの実行と、行動用のクラスを解放する
	void UninitNodeLogic();

	// 実行するノードを入れ替えて、プレイヤーと結びつける
	void SetRunNodes(std::vector<NodeBase *> nodes);

	// 前のフレームの行動が残らないように消す
	void ClearNodeCommands();

	// ノードから受け取った回転を実行する
	void CommandRotate();

	// ノードから受け取った移動を実行する
	void CommandMove();

	// 旧形式の行動要求をクリアする
	void CommandAction();

	// 槍または剣の攻撃を始め、攻撃の向きを固定する
	void CommandAttackMelee();

	// 射撃の待ち時間を見て、弾を撃つ
	void CommandAttackRange();

	// HPからダメージ分を引く
	void TakeDamage(int num, const Vector3 *attackOrigin = nullptr);

	// 練習を最初から試せるように、HPと位置を戻す
	void ResetForTest();

	// 実行終了時に武器の状態を戻す
	void ResetAttacks();

	// 相手が見つかる距離にいるか調べる
	bool HasVisibleTarget() const;

	// 前方の移動先に壁があるか調べる
	bool HasObstacleAhead(float distance) const;

	// 構え・近接攻撃・発射直後をまとめて確認する
	bool IsAttacking() const;

	// 構え・突き・戻りに合わせて、棒の長さを決める
	float GetMeleeExtension() const;

	// 剣の振りに合わせて、武器だけの向きを決める
	float GetMeleeSwingHeading() const;

	// 正面の棒と、相手の体が重なっているか調べる
	bool SwordTouches(Player *target, const Vector3 &origin, float heading, float length = 1.2f) const;

	// ノードから行動要求を受け取る
	void RequestCategory(NodeCategory category)
	{
		m_NodeControl.AddCategory(category);
	}

	// ノードから行動要求を受け取る
	void RequestAttackMelee(NodeType meleeType)
	{
		m_NodeControl.SetMelee(meleeType);

		m_NodeControl.AddCategory(NodeCategory::AttackMelee);
	}

	// ノードから行動要求を受け取る
	void RequestAttackRange(NodeType rangeType)
	{
		m_NodeControl.SetRange(rangeType);

		m_NodeControl.AddCategory(NodeCategory::AttackRange);
	}

	// ノードから行動要求を受け取る
	void RequestAction(NodeType actionType)
	{
		m_NodeControl.SetAction(actionType);

		m_NodeControl.AddCategory(NodeCategory::Action);
	}

	// ノードから行動要求を受け取る
	void RequestMove(NodeType moveType)
	{
		m_NodeControl.SetMovement(moveType);

		m_NodeControl.AddCategory(NodeCategory::Movement);
	}

	// ノードから行動要求を受け取る
	void RequestRotate(NodeType rotateType)
	{
		m_NodeControl.SetRotate(rotateType);

		m_NodeControl.AddCategory(NodeCategory::Rotate);
	}

	void MarkNodeRuntimeStarted()
	{
		m_NodeControl.SetRuntimeStarted(true);
	}

	void SetCanStartNode(bool b)
	{
		m_NodeControl.SetCanStartNode(b);

		if (!b)
		{
			CancelMeleeAttack();

			CancelRangeAttack();

			SetGuarding(false);
		}
	}

	bool CanStartNode() const
	{
		return m_NodeControl.GetCanStartNode();
	}

	// ステータスのゲッター
	const Stats *GetStats() const
	{
		return &m_Stats;
	}

	int GetHP() const
	{
		return m_Stats.hp;
	}

	float GetMoveSpeed() const
	{
		return m_Stats.speed * 60.0f * m_NodeControl.GetNodeDeltaSeconds();
	}

	void SetNodeDeltaSeconds(float dt)
	{
		m_NodeControl.SetNodeDeltaSeconds(dt);
	}

	void SetNodeMoveDistance(float distance)
	{
		m_NodeControl.SetNodeMoveDistance(distance);
	}

	float GetNodeMoveDistance() const
	{
		return m_NodeControl.GetNodeMoveDistance();
	}

	// 発射前の構えと発射直後の状態を止める
	void CancelRangeAttack()
	{
		m_Attacks.CancelRangeAttack();
	}

	// 近接攻撃を止める
	void CancelMeleeAttack()
	{
		m_Attacks.CancelMeleeAttack();
	}

	void SetSwordEnabled(bool enabled)
	{
		m_Attacks.SetSwordEnabled(enabled);
	}

	// 正面の棒に触れるだけでダメージを与えるか決める
	void SetSwordDamageEnabled(bool enabled)
	{
		m_Attacks.SetSwordDamageEnabled(enabled);
	}

	// 接触ダメージの設定を確認する
	bool IsSwordDamageEnabled() const
	{
		return m_Attacks.IsSwordDamageEnabled();
	}

	bool IsSwordEnabled() const
	{
		return m_Attacks.IsSwordEnabled();
	}

	// 射撃の構え中かを確認する
	bool IsRangePending() const
	{
		return m_Attacks.IsRangePending();
	}

	bool IsMeleePending() const
	{
		return m_Attacks.IsMeleePending();
	}

	void SetNodeLoop(bool loop)
	{
		m_NodeControl.SetNodeLoop(loop);
	}

	NodeRuntime *GetNodeRuntime() const
	{
		return m_NodeControl.GetNodeRun();
	}

	// 保存データの初期HPを適用し、100を超えるHPも切り捨てない
	void SetInitialHP(int hp)
	{
		m_Stats.maxHp = (std::max)(100, hp);

		m_Stats.hp = hp;
	}

	void SetTestHP(int hp)
	{
		m_Stats.hp = (std::max)(0, (std::min)(m_Stats.maxHp, hp));
	}

	// 盾ノードの開始・終了で、防御の状態を切り替える
	void SetGuarding(bool guarding)
	{
		m_GuardRequested = guarding;

		m_Stats.Shield = guarding && m_GuardEnergy > 0.00001f;
	}

	// 盾の残量を0～100%で画面に表示する
	int GetGuardPercent() const
	{
		return static_cast<int>(100.0f * m_GuardEnergy / CombatBalance::ShieldCapacity + 0.5f);
	}

	bool IsGuarding() const
	{
		return m_Stats.Shield;
	}

	void SetTargetObject(GameObject3D *target)
	{
		m_TargetObject = target;
	}

	// 相手がまだ存在するか確認してから返す
	GameObject3D *GetTargetObject() const;

	std::vector<NodeBase *> GetRunNodes()
	{
		return m_NodeControl.GetRunNodes();
	}

	void SetTeam(Team team)
	{
		m_Team = team;
	}

	Team GetTeam() const
	{
		return m_Team;
	}

	void SetIfFindObject(bool i)
	{
		m_If.findObject = i;
	}

	bool GetIfFindObject()
	{
		return m_If.findObject;
	}

	void SetIfRangeIO(bool i)
	{
		m_If.rio = i;
	}

	bool GetIfRangeIO()
	{
		return m_If.rio;
	}

	void SetIfStats(bool i)
	{
		m_If.stats = i;
	}

	bool GetIfStats()
	{
		return m_If.stats;
	}

	void SetExecutingAttackMelee(bool e)
	{
		m_Execute.attackMelee = e;
	}

	bool IsExecutingAttackMelee() const
	{
		return m_Execute.attackMelee;
	}

	void SetExecutingAttackRange(bool e)
	{
		m_Execute.attackRange = e;
	}

	bool IsExecutingAttackRange() const
	{
		return m_Execute.attackRange;
	}

	void SetExecutingAction(bool e)
	{
		m_Execute.action = e;
	}

	bool IsExecutingAction() const
	{
		return m_Execute.action;
	}

	void SetExecutingMove(bool e)
	{
		m_Execute.move = e;
	}

	bool IsExecutingMove() const
	{
		return m_Execute.move;
	}

	void SetExecutingRotate(bool e)
	{
		m_Execute.rotate = e;
	}

	bool IsExecutingRotate() const
	{
		return m_Execute.rotate;
	}

	// 描画に必要な武器の状態を読み取る
	const AttackController &GetAttackController() const;

}; 
