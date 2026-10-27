#pragma once

#include "main.h"
#include "gameObject.h"
#include "vector3.h"

class Particle;

// ParticleEffectManagerが管理するParticleの用途。
enum class ParticleEffectType
{
	FireWork = 0,
	Float,
	Rain,
	PlayerSpark,
	PlayerLanding,

	Max
};

// 複数のParticleを用途別に保持し、ゲーム側から使いやすい発生関数を提供する。
// 実際のGPU更新と描画はParticle本体が担当する。
class ParticleEffectManager : public GameObject
{
private:
	static const int PARTICLE_EFFECT_TYPE_MAX = static_cast<int>(ParticleEffectType::Max);

private:
	// FireWork / Float / Rain / PlayerSpark / PlayerLanding 用のParticle。
	Particle* m_Particles[PARTICLE_EFFECT_TYPE_MAX]{};

	// 全Particleが生成済みになってから、用途ごとのテクスチャを設定する。
	bool m_IsParticleSetup = false;

	// プレイヤー周辺に雨粒を発生させるための設定。
	bool m_IsRainActive = false;
	int m_RainInterval = 1;
	int m_RainTimer = 0;
	int m_RainEmitCountPerFrame = 100;

	Vector3 m_RainCenter = Vector3(0.0f, 16.0f, 25.0f);
	Vector3 m_RainRange = Vector3(60.0f, 10.0f, 70.0f);

	// 床付近に上昇する粒子を発生させるための設定。
	bool m_IsFloorFloatActive = true;
	int m_FloorFloatInterval = 4;
	int m_FloorFloatTimer = 0;
	int m_FloorFloatEmitCount = 8;

	float m_FloorFloatY = 0.25f;

	Vector3 m_FloorFloatCenter = Vector3(0.0f, 0.0f, 25.0f);
	Vector3 m_FloorFloatRange = Vector3(45.0f, 0.0f, 70.0f);

public:
	void Init() override;
	void Uninit() override;
	void Update() override;
	void Draw() override;

	void SetParticle(
		ParticleEffectType type,
		Particle* particle
	);

	void SetRainActive(bool active);

	void SetRainArea(
		Vector3 center,
		Vector3 range
	);

	void SetRainAmount(
		int emitCountPerFrame,
		int interval
	);

	void SetFloorFloatActive(bool active);

	void SetFloorFloatArea(
		Vector3 center,
		Vector3 range,
		float floorY
	);

	void SetFloorFloatAmount(
		int emitCount,
		int interval
	);

	void EmitFireWork(
		Vector3 position,
		int count
	);

	void EmitFloat(
		Vector3 position,
		int count
	);

	void EmitSpark(
		Vector3 position,
		int count
	);

	void EmitLandingSpark(Vector3 position);
	void EmitWallSlidSpark(Vector3 position);
	void EmitWallKickSpark(Vector3 position);

	void EmitRain(
		Vector3 position,
		int count
	);

private:
	float RandomRange(
		float minValue,
		float maxValue
	);

	bool RandomChance(int denominator);

	int ToIndex(ParticleEffectType type) const;

	Particle* GetParticle(ParticleEffectType type);

	bool AreParticlesReady();
	void SetupParticles();

	void UpdateRain();
	void UpdateFloorFloat();
};
