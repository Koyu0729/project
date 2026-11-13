#pragma once

namespace CombatBalance
{
	// 射撃は構えた向きに発射し、近距離には使えない
	constexpr float ShotWindup = 0.3f;
	constexpr float ShotActionTime = 0.45f;

	// 盾以外のすべての攻撃は、この共通クールタイムを使う
	constexpr float AttackCooldown = 0.5f;
	constexpr float ShotMinimumDistance = 1.4f;
	constexpr float BulletSpeed = 0.1f;
	constexpr float BulletRadius = 0.4f;
	constexpr int BulletDamage = 10;

	// 近接攻撃と、正面の武器の接触ダメージ
	constexpr int MeleeDamage = 10;
	constexpr int ContactDamage = 10;

	// 盾は正面120度を防ぎ、構え続けると尽きる（下ろして回復する）
	constexpr float ShieldCapacity = 1.2f;
	constexpr float ShieldRecoveryPerSecond = 0.6f;
	constexpr float ShieldFrontDot = 0.5f;
}
