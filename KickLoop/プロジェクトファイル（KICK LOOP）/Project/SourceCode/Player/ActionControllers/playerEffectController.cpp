#include "playerEffectController.h"

#include "manager.h"
#include "scene.h"

#include "speedLineEffect.h"
#include "particleEffectManager.h"

#include "player.h"
#include "camera.h"
#include "stageBlockManager.h"
#include "groundBlockManager.h"
#include "backgroundObjectManager.h"

#include <cmath>

// 呼び出し側がScene内の配置を意識しなくて済むように、このクラス内で参照をまとめる。
SpeedLineEffect* PlayerEffectController::GetSpeedLineEffect()
{
	Scene* scene = Manager::GetScene();

	if (scene == nullptr)
	{
		return nullptr;
	}

	return scene->GetGameObject<SpeedLineEffect>();
}

ParticleEffectManager* PlayerEffectController::GetParticleEffectManager()
{
	Scene* scene = Manager::GetScene();

	if (scene == nullptr)
	{
		return nullptr;
	}

	return scene->GetGameObject<ParticleEffectManager>();
}

void PlayerEffectController::Disable(bool resetSpeedRate)
{
	SpeedLineEffect* speedLineEffect = GetSpeedLineEffect();

	if (speedLineEffect == nullptr)
	{
		return;
	}

	speedLineEffect->SetActiveEffect(false);

	if (resetSpeedRate)
	{
		speedLineEffect->SetSpeedRate(0.0f);
	}
}

void PlayerEffectController::SetSpeedRate(float speedRate)
{
	SpeedLineEffect* speedLineEffect = GetSpeedLineEffect();

	if (speedLineEffect == nullptr)
	{
		return;
	}

	if (speedRate < 0.0f)
	{
		speedRate = 0.0f;
	}

	if (speedRate > 1.0f)
	{
		speedRate = 1.0f;
	}

	speedLineEffect->SetSpeedRate(speedRate);

	if (speedRate > 0.0f)
	{
		speedLineEffect->SetActiveEffect(true);
	}
	else
	{
		speedLineEffect->SetActiveEffect(false);
	}
}


// 移動の速さに合わせて速度線を調整する
void PlayerEffectController::SetSpeedRateFromVelocity(
	Vector3 velocity,
	float appearSpeed,
	float maxSpeed
)
{
	float speed = sqrtf(velocity.x * velocity.x + velocity.y * velocity.y + velocity.z * velocity.z);

	float speedRate = 0.0f;

	if (maxSpeed > appearSpeed)
	{
		speedRate = (speed - appearSpeed) / (maxSpeed - appearSpeed);
	}

	SetSpeedRate(speedRate);
}

// 移動の速さに合わせて速度線を調整する
void PlayerEffectController::SetSpeedRateFromVelocity(
	Vector3 velocity,
	float virtualForwardMove,
	float appearSpeed,
	float maxSpeed
)
{
	float speed = sqrtf(velocity.x * velocity.x + velocity.y * velocity.y + virtualForwardMove * virtualForwardMove);

	float speedRate = 0.0f;

	if (maxSpeed > appearSpeed)
	{
		speedRate = (speed - appearSpeed) / (maxSpeed - appearSpeed);
	}

	SetSpeedRate(speedRate);
}


// 壁滑りに合わせて速度線の向きを変える
void PlayerEffectController::SetWallSlideEffect(
	bool isActive,
	float directionX
)
{
	SpeedLineEffect* speedLineEffect = GetSpeedLineEffect();

	if (speedLineEffect == nullptr)
	{
		return;
	}

	speedLineEffect->SetWallSlideEffect(
		isActive,
		directionX
	);
}

// 速度線のブースト演出を再生する。
void PlayerEffectController::PlayBoost()
{
	SpeedLineEffect* speedLineEffect = GetSpeedLineEffect();

	if (speedLineEffect == nullptr)
	{
		return;
	}

	speedLineEffect->SetActiveEffect(true);
	speedLineEffect->PlayBoost();
}

void PlayerEffectController::EmitSpark(
	Vector3 position,
	int count
)
{
	ParticleEffectManager* particleEffectManager = GetParticleEffectManager();

	if (particleEffectManager == nullptr)
	{
		return;
	}

	particleEffectManager->EmitSpark(
		position,
		count
	);
}

void PlayerEffectController::EmitLandingSpark(Vector3 position)
{
	ParticleEffectManager* particleEffectManager = GetParticleEffectManager();

	if (particleEffectManager == nullptr)
	{
		return;
	}

	particleEffectManager->EmitLandingSpark(position);
}

void PlayerEffectController::EmitWallSlidSpark(Vector3 position)
{
	ParticleEffectManager* particleEffectManager = GetParticleEffectManager();

	if (particleEffectManager == nullptr)
	{
		return;
	}

	particleEffectManager->EmitWallSlidSpark(position);
}

// 壁キック時の火花を発生させる。
void PlayerEffectController::EmitWallKickSpark(Vector3 position)
{
	ParticleEffectManager* particleEffectManager = GetParticleEffectManager();

	if (particleEffectManager == nullptr)
	{
		return;
	}

	particleEffectManager->EmitWallKickSpark(position);
}

void PlayerEffectController::StartGoalEffect(Vector3 playerPosition)
{
	if (m_IsGoalEffect)
	{
#ifdef _DEBUG
		printf("[GoalEffect] Start ignored. already active\n");
#endif
		return;
	}

#ifdef _DEBUG
	printf(
		"[GoalEffect] Start %.2f %.2f %.2f\n",
		playerPosition.x,
		playerPosition.y,
		playerPosition.z
	);
#endif

	m_IsGoalEffect = true;
	m_GoalEffectFrame = 0;
	m_IsGoalCameraStarted = false;

	SpawnGoalFireworks(
		playerPosition
	);
}

void PlayerEffectController::UpdateGoalEffect(Vector3 playerPosition)
{
	if (!m_IsGoalEffect)
	{
		return;
	}

	m_GoalEffectFrame++;

	if (m_GoalFireworkInterval > 0)
	{
		if (m_GoalEffectFrame % m_GoalFireworkInterval == 0)
		{
			SpawnGoalFireworks(
				playerPosition
			);
		}
	}

	if (!m_IsGoalCameraStarted && m_GoalEffectFrame >= m_GoalCameraStartFrame)
	{
		m_IsGoalCameraStarted = true;

		StartGoalCameraSequence();

		SpawnGoalFireworks(
			playerPosition
		);
	}
}


// ゴール用の花火を出す
void PlayerEffectController::SpawnGoalFireworks(Vector3 playerPosition)
{
#ifdef _DEBUG
	printf(
		"[GoalFirework] playerPosition %.2f %.2f %.2f\n",
		playerPosition.x,
		playerPosition.y,
		playerPosition.z
	);
#endif
	Scene* scene = Manager::GetScene();

	if (scene == nullptr)
	{
		return;
	}

	ParticleEffectManager* particleEffectManager = scene->GetGameObject<ParticleEffectManager>();

	if (particleEffectManager == nullptr)
	{
		return;
	}

	const int fireworkCount = 8;

	const float radius = 8.0f;
	const float height = 5.0f;

	for (int i = 0; i < fireworkCount; i++)
	{
		float angle = XM_2PI * (float)i / (float)fireworkCount;

		Vector3 fireworkPosition = playerPosition;

		fireworkPosition.x += cosf(angle) * radius;
		fireworkPosition.z += sinf(angle) * radius;
		fireworkPosition.y += height;

#ifdef _DEBUG
		printf(
			"[GoalFirework] emit %.2f %.2f %.2f count=%d\n",
			fireworkPosition.x,
			fireworkPosition.y,
			fireworkPosition.z,
			140
		);
#endif

		particleEffectManager->EmitFireWork(
			fireworkPosition,
			140
		);
	}
}

// ゴール演出用のカメラシーケンスを開始する。
void PlayerEffectController::StartGoalCameraSequence()
{
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

	Camera* camera = scene->GetGameObject<Camera>();

	if (camera == nullptr)
	{
		return;
	}

	//StageBlockManager* stageBlockManager = scene->GetGameObject<StageBlockManager>();

	//if (stageBlockManager != nullptr)
	//{
	//	stageBlockManager->RebuildPassedStageBlocks();
	//}

	GroundBlockManager* groundBlockManager = scene->GetGameObject<GroundBlockManager>();

	if (groundBlockManager != nullptr)
	{
		groundBlockManager->RebuildPassedGroundBlocks();
	}

	BackgroundObjectManager* backgroundObjectManager = scene->GetGameObject<BackgroundObjectManager>();

	if (backgroundObjectManager != nullptr)
	{
		backgroundObjectManager->RebuildPassedBackgroundObjects();
	}

	camera->StartGoalCameraSequence(
		Vector3(25.0f, 35.0f, 13.0f),
		Vector3(25.0f, 35.0f, player->GetPosition().z - 15.0f),
		Vector3(25.0f, player->GetPosition().y + 7.0f, player->GetPosition().z + 10.0f)
	);

#ifdef _DEBUG
	printf("\n goal start \n");
#endif
}
