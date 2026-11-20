#include "main.h"
#include "tutorial.h"

#include "camera.h"
#include "input.h"
#include "manager.h"
#include "particle.h"
#include "particleEffectManager.h"
#include "player.h"
#include "sky.h"
#include "speedLineEffect.h"
#include "stageBlockManager.h"
#include "title.h"

Tutorial::Tutorial(int roomIndex) : m_RoomIndex(roomIndex)
{
}

void Tutorial::Init()
{
	Camera* camera = AddGameObject<Camera>(SceneLayer::Camera);
	camera->SetYawPitch(0.0f, -0.30f);

	AddGameObject<Sky>(SceneLayer::DepthSorted);

	m_Player = AddGameObject<Player>(SceneLayer::Foreground);
	m_Player->SetVisible(false);

	StageBlockManager* stage = AddGameObject<StageBlockManager>(SceneLayer::SceneObjects);
	m_Room.Build(*this, *stage, m_RoomIndex);

	camera->SetSceneReferences(m_Player, stage);
	m_Player->SetSceneReferences(camera, stage, nullptr);

	InitEffects();
	m_Guide.Init(*this, m_RoomIndex);
}

void Tutorial::InitEffects()
{
	ParticleEffectManager* effects = AddGameObject<ParticleEffectManager>(SceneLayer::Particles);

	const ParticleEffectType types[] =
	{
		ParticleEffectType::FireWork,
		ParticleEffectType::Float,
		ParticleEffectType::Rain,
		ParticleEffectType::PlayerSpark,
		ParticleEffectType::PlayerLanding,
	};

	for (ParticleEffectType type : types)
	{
		effects->SetParticle(type, AddGameObject<Particle>(SceneLayer::Particles));
	}

	effects->SetRainActive(false);
	effects->SetFloorFloatActive(false);

	AddGameObject<SpeedLineEffect>(SceneLayer::Hud);
}

void Tutorial::Update()
{
	if (Input::GetKeyTrigger(VK_ESCAPE))
	{
		Manager::SetScene<Title>(true);
		return;
	}

	if (Input::GetKeyTrigger('R'))
	{
		Manager::SetNextScene(new Tutorial(m_RoomIndex), true);
		return;
	}

	if (m_IsCleared && Input::GetKeyTrigger(VK_RETURN))
	{
		if (!m_Room.IsFinal())
		{
			Manager::SetNextScene(new Tutorial(m_RoomIndex + 1), true);
		}
		else
		{
			Manager::SetScene<Title>(true);
		}

		return;
	}

	Scene::Update();

	if (!m_IsCleared && m_Player->IsDead())
	{
		Manager::SetNextScene(new Tutorial(m_RoomIndex), true);
		return;
	}

	if (!m_IsCleared)
	{
		m_IsCleared = m_Lesson.CheckClear(m_Room, *m_Player);
	}

	m_Guide.Update(m_Room, *m_Player, m_IsCleared);
}
