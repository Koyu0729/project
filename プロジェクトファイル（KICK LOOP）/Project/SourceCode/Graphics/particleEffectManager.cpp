#include "particleEffectManager.h"

#include "particle.h"

#include "manager.h"
#include "scene.h"
#include "player.h"

#include <cstdlib>

void ParticleEffectManager::Init()
{
	for (int i = 0; i < PARTICLE_EFFECT_TYPE_MAX; i++)
	{
		m_Particles[i] = nullptr;
	}

	m_IsParticleSetup = false;

	m_IsRainActive = RandomChance(3);
	m_RainInterval = 1;
	m_RainTimer = 0;
	m_RainEmitCountPerFrame = 100;
	m_RainCenter = Vector3(0.0f, 16.0f, 25.0f);
	m_RainRange = Vector3(60.0f, 10.0f, 70.0f);

	m_IsFloorFloatActive = true;
	m_FloorFloatInterval = 4;
	m_FloorFloatTimer = 0;
	m_FloorFloatEmitCount = 8;
	m_FloorFloatY = 0.25f;
	m_FloorFloatCenter = Vector3(25.0f, 0.0f, 25.0f);
	m_FloorFloatRange = Vector3(45.0f, 0.0f, 70.0f);
}

void ParticleEffectManager::Uninit()
{
	for (int i = 0; i < PARTICLE_EFFECT_TYPE_MAX; i++)
	{
		m_Particles[i] = nullptr;
	}

	m_IsParticleSetup = false;
}

void ParticleEffectManager::Update()
{
	if (!m_IsParticleSetup)
	{
		SetupParticles();
	}

	if (!m_IsParticleSetup)
	{
		return;
	}

	UpdateRain();
	UpdateFloorFloat();
}

void ParticleEffectManager::Draw()
{
}

void ParticleEffectManager::SetParticle(
	ParticleEffectType type,
	Particle* particle
)
{
	int index = ToIndex(type);

	if (index < 0 || index >= PARTICLE_EFFECT_TYPE_MAX)
	{
		return;
	}

	m_Particles[index] = particle;
	m_IsParticleSetup = false;
}

// エフェクトの種類を配列番号に変換する
int ParticleEffectManager::ToIndex(ParticleEffectType type) const
{
	return static_cast<int>(type);
}

Particle* ParticleEffectManager::GetParticle(ParticleEffectType type)
{
	int index = ToIndex(type);

	if (index < 0 || index >= PARTICLE_EFFECT_TYPE_MAX)
	{
		return nullptr;
	}

	return m_Particles[index];
}

bool ParticleEffectManager::AreParticlesReady()
{
	for (int i = 0; i < PARTICLE_EFFECT_TYPE_MAX; i++)
	{
		if (m_Particles[i] == nullptr)
		{
			return false;
		}

		if (!m_Particles[i]->IsReady())
		{
			return false;
		}
	}

	return true;
}

void ParticleEffectManager::SetupParticles()
{
	if (!AreParticlesReady())
	{
		return;
	}

	Particle* fireWorkParticle = GetParticle(ParticleEffectType::FireWork);
	Particle* floatParticle = GetParticle(ParticleEffectType::Float);
	Particle* rainParticle = GetParticle(ParticleEffectType::Rain);
	Particle* playerSparkParticle = GetParticle(ParticleEffectType::PlayerSpark);
	Particle* playerLandingParticle = GetParticle(ParticleEffectType::PlayerLanding);

	if (fireWorkParticle != nullptr)
	{
		fireWorkParticle->SetTexture("asset\\texture\\particle.png");
	}

	if (floatParticle != nullptr)
	{
		floatParticle->SetTexture("asset\\texture\\particle.png");
	}

	if (rainParticle != nullptr)
	{
		rainParticle->SetTexture("asset\\texture\\drop.png");
	}

	if (playerSparkParticle != nullptr)
	{
		playerSparkParticle->SetTexture("asset\\texture\\spark.png");
	}

	if (playerLandingParticle != nullptr)
	{
		playerLandingParticle->SetTexture("asset\\texture\\spark.png");
	}

	m_IsParticleSetup = true;
}

// 指定範囲の乱数を返す
float ParticleEffectManager::RandomRange(
	float minValue,
	float maxValue
)
{
	float rate = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
	return minValue + (maxValue - minValue) * rate;
}

// 指定確率で発生可否を決める
bool ParticleEffectManager::RandomChance(int denominator)
{
	if (denominator <= 1)
	{
		return true;
	}

	return rand() % denominator == 0;
}

// 指定範囲に雨の粒子を出す
void ParticleEffectManager::UpdateRain()
{
	if (!m_IsRainActive)
	{
		return;
	}

	Particle* rainParticle = GetParticle(ParticleEffectType::Rain);

	if (rainParticle == nullptr)
	{
		return;
	}

	Scene* scene = Manager::GetScene();

	if (scene == nullptr)
	{
		return;
	}

	Player* player = scene->GetGameObject<Player>();

	if (player == nullptr)
	{
		return;
	}

	Vector3 playerPosition = player->GetPosition();

	m_RainTimer--;

	if (m_RainTimer > 0)
	{
		return;
	}

	m_RainTimer = m_RainInterval;

	for (int i = 0; i < m_RainEmitCountPerFrame; i++)
	{
		Vector3 position;

		position.x = playerPosition.x + RandomRange(-m_RainRange.x, m_RainRange.x);
		position.y = playerPosition.y + m_RainCenter.y + RandomRange(0.0f, m_RainRange.y);
		position.z = playerPosition.z + RandomRange(-m_RainRange.z, m_RainRange.z);

		rainParticle->EmitRain(
			position,
			1
		);
	}
}

// 床の近くに上昇する粒子を出す
void ParticleEffectManager::UpdateFloorFloat()
{
	if (!m_IsFloorFloatActive)
	{
		return;
	}

	Particle* floatParticle = GetParticle(ParticleEffectType::Float);

	if (floatParticle == nullptr)
	{
		return;
	}

	Scene* scene = Manager::GetScene();

	if (scene == nullptr)
	{
		return;
	}

	Player* player = scene->GetGameObject<Player>();

	if (player == nullptr)
	{
		return;
	}

	Vector3 playerPosition = player->GetPosition();

	m_FloorFloatTimer--;

	if (m_FloorFloatTimer > 0)
	{
		return;
	}

	m_FloorFloatTimer = m_FloorFloatInterval;

	for (int i = 0; i < m_FloorFloatEmitCount; i++)
	{
		Vector3 position;

		position.x = playerPosition.x + RandomRange(-m_FloorFloatRange.x, m_FloorFloatRange.x);
		position.y = m_FloorFloatY;
		position.z = playerPosition.z + RandomRange(-m_FloorFloatRange.z, m_FloorFloatRange.z);

		floatParticle->EmitFloatUp(
			position,
			1
		);
	}
}

void ParticleEffectManager::SetRainActive(bool active)
{
	m_IsRainActive = active;
}

void ParticleEffectManager::SetRainArea(
	Vector3 center,
	Vector3 range
)
{
	m_RainCenter = center;
	m_RainRange = range;
}

void ParticleEffectManager::SetRainAmount(
	int emitCountPerFrame,
	int interval
)
{
	m_RainEmitCountPerFrame = emitCountPerFrame;
	m_RainInterval = interval;

	if (m_RainEmitCountPerFrame < 0)
	{
		m_RainEmitCountPerFrame = 0;
	}

	if (m_RainInterval < 1)
	{
		m_RainInterval = 1;
	}
}

void ParticleEffectManager::SetFloorFloatActive(bool active)
{
	m_IsFloorFloatActive = active;
}

void ParticleEffectManager::SetFloorFloatArea(
	Vector3 center,
	Vector3 range,
	float floorY
)
{
	m_FloorFloatCenter = center;
	m_FloorFloatRange = range;
	m_FloorFloatY = floorY;
}

void ParticleEffectManager::SetFloorFloatAmount(
	int emitCount,
	int interval
)
{
	m_FloorFloatEmitCount = emitCount;
	m_FloorFloatInterval = interval;

	if (m_FloorFloatEmitCount < 0)
	{
		m_FloorFloatEmitCount = 0;
	}

	if (m_FloorFloatInterval < 1)
	{
		m_FloorFloatInterval = 1;
	}
}

// 花火用の粒子を出す
void ParticleEffectManager::EmitFireWork(
	Vector3 position,
	int count
)
{
	if (!m_IsParticleSetup)
	{
		return;
	}

	Particle* particle = GetParticle(ParticleEffectType::FireWork);

	if (particle == nullptr)
	{
		return;
	}

	particle->EmitFireWork(
		position,
		count
	);
}

void ParticleEffectManager::EmitFloat(
	Vector3 position,
	int count
)
{
	if (!m_IsParticleSetup)
	{
		return;
	}

	Particle* particle = GetParticle(ParticleEffectType::Float);

	if (particle == nullptr)
	{
		return;
	}

	particle->EmitFloatUp(
		position,
		count
	);
}

void ParticleEffectManager::EmitSpark(
	Vector3 position,
	int count
)
{
	if (!m_IsParticleSetup)
	{
		return;
	}

	Particle* particle = GetParticle(ParticleEffectType::PlayerSpark);

	if (particle == nullptr)
	{
		return;
	}

	particle->EmitSpark(
		position,
		count
	);
}

void ParticleEffectManager::EmitLandingSpark(Vector3 position)
{
	if (!m_IsParticleSetup)
	{
		return;
	}

	Particle* particle = GetParticle(ParticleEffectType::PlayerLanding);

	if (particle == nullptr)
	{
		return;
	}

	position.y += 0.15f;

	particle->EmitLanding(
		position,
		180
	);
}

void ParticleEffectManager::EmitWallSlidSpark(Vector3 position)
{
	if (!m_IsParticleSetup)
	{
		return;
	}

	Particle* particle = GetParticle(ParticleEffectType::PlayerSpark);

	if (particle == nullptr)
	{
		return;
	}

	position.y += 0.08f;

	particle->EmitSpark(
		position,
		18
	);
}

void ParticleEffectManager::EmitWallKickSpark(Vector3 position)
{
	if (!m_IsParticleSetup)
	{
		return;
	}

	Particle* particle = GetParticle(ParticleEffectType::PlayerLanding);

	if (particle == nullptr)
	{
		return;
	}

	position.y += 0.1f;

	particle->EmitLanding(
		position,
		180
	);
}

void ParticleEffectManager::EmitRain(
	Vector3 position,
	int count
)
{
	if (!m_IsParticleSetup)
	{
		return;
	}

	Particle* particle = GetParticle(ParticleEffectType::Rain);

	if (particle == nullptr)
	{
		return;
	}

	particle->EmitRain(
		position,
		count
	);
}
