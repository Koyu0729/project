#pragma once
#include "nodeList.h"
#include "vector3.h"
#include "playerRangeList.h"
#include <unordered_set>

class Player;

// 武器の動作・接触判定・攻撃共通の待ち時間を管理する
class AttackController
{
	// 攻撃するキャラクター。位置・向き・チームはキャラクターから取得する
	Player &m_Player;

	// 射撃時に弾を生成する
	PlayerRangeList m_RangeList;

	// 近接攻撃の種類・経過時間・命中済みの相手と、共通クールタイム
	NodeType m_ActiveMeleeType = NodeType::Type_Null;

	bool m_MeleePending = false;

	float m_AttackCooldown = 0.0f;

	float m_MeleeElapsed = 0;

	std::unordered_set<Player *> m_MeleeHits;

	// 射撃の構え・発射直後の状態と、開始時の位置・向き
	float m_RangeAttackTime = 0;

	bool m_RangePending = false;

	float m_RangeElapsed = 0;

	float m_RangeHeading = 0;

	Vector3 m_RangeOrigin{};

	// 近接攻撃を開始した位置・向き
	float m_MeleeHeading = 0;

	Vector3 m_MeleeOrigin{};

	// 正面の武器の表示設定と、継続して接触している相手
	bool m_SwordEnabled = true;

	bool m_SwordDamageEnabled = true;

	std::unordered_set<Player *> m_SwordContacts;

	// キャラクターの向きを攻撃開始時の軸に固定する
	void SetHeading(float heading);

	// 剣・槍の動作と命中判定を更新する
	void UpdateMelee(float dt);

	// 射撃の構えが完了したら弾を生成する
	void UpdateRangeAttack(float dt);

  public:
	// 所有するキャラクターを設定する
	explicit AttackController(Player &player);

	// テスト再開始時の攻撃状態を初期化する
	void Reset();
	// 共通の待ち時間と攻撃の経過時間を進める
	void Update(float dt);

	// 正面の武器の接触を調べ、接触開始時だけダメージを与える
	void UpdateSword(bool damageAllowed);

	// 近接攻撃・射撃を開始する
	void StartMelee(NodeType type);

	void StartRange(NodeType type);
	// 攻撃を中断し、命中済みの記録を消す
	void CancelMeleeAttack();

	void CancelRangeAttack();

	// 武器の表示と接触ダメージを切り替える
	void SetSwordEnabled(bool enabled);
	void SetSwordDamageEnabled(bool enabled);

	// 武器の表示と命中判定に使う形状を取得する
	float GetMeleeExtension() const;
	float GetMeleeSwingHeading() const;
	bool SwordTouches(Player *target, const Vector3 &origin, float heading, float length = 1.2f) const;
	// 表示と条件判定に使う状態を取得する
	bool IsSwordEnabled() const
	{
		return m_SwordEnabled;
	}
	bool IsSwordDamageEnabled() const
	{
		return m_SwordDamageEnabled;
	}
	bool IsMeleePending() const
	{
		return m_MeleePending;
	}
	bool IsRangePending() const
	{
		return m_RangePending;
	}
	bool IsAttacking() const
	{
		return m_MeleePending || m_RangePending || m_RangeAttackTime > 0;
	}
	NodeType GetMeleeType() const
	{
		return m_ActiveMeleeType;
	}

	const Vector3 &GetMeleeOrigin() const
	{
		return m_MeleeOrigin;
	}
	const Vector3 &GetRangeOrigin() const
	{
		return m_RangeOrigin;
	}
	float GetRangeHeading() const
	{
		return m_RangeHeading;
	}
};
