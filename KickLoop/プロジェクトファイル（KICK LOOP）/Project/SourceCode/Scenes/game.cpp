#include "main.h"
#include "manager.h"
#include "game.h"

#include "camera.h"
#include "player.h"

#include "sky.h"

//FBXオブジェクト


//背景オブジェクト
#include "backgroundObjectManager.h"

//ステージブロック
#include "stageBlockManager.h"

//床ブロック
#include "groundBlockManager.h"

//ゴールobj
#include "goalBuilding.h"

//UI
#include "speedLineEffect.h"

#include "gameTime.h"

#include "comboCounterUI.h"
#include "bonusGauge.h"
#include "distanceCounter.h"

#include "polygon2D.h"

//エフェクト
#include "particleEffectManager.h"
#include "particle.h"

//ロードデータ
#include "gameDataLoader.h"

//シーン
#include "result.h"

//リザルトデータ
#include "GameData.h"

//オプションUI
#include "optionUI.h"

void Game::SetLoadData(const GameLoadData& loadData)
{
	m_LoadData = loadData;
	m_HasLoadData = true;
}

void Game::LoadDataIfNeeded()
{
	if (m_HasLoadData)
	{
		return;
	}

	if (!GameDataLoader::LoadStageData("asset\\data\\stageData.json", m_LoadData))
	{
		printf("stageData.json の読み込みに失敗したため、デフォルトデータを使用します。\n");

		m_LoadData = GameDataLoader::CreateDefaultGameLoadData();
	}

	m_HasLoadData = true;
}

void Game::BuildScene()
{
	Camera* camera = AddGameObject<Camera>(0);

	if (camera != nullptr)
	{
		camera->SetYawPitch(0.0, -0.30f);
	}

	AddGameObject<Sky>(1);

	//AddGameObject<AnimatedObject>(2);

	Player* player = AddGameObject<Player>(2);

	if (player != nullptr)
	{
		player->SetMainState(PlayerMainState::Wait);
		player->SetVisible(false);
	}

	StageBlockManager* stageBlockManager =
		AddGameObject<StageBlockManager>(3);

	if (stageBlockManager != nullptr)
	{
		stageBlockManager->SetObstaclePresets(
			m_LoadData.floorObstaclePresets,
			m_LoadData.wallObstaclePresets
		);
	}

	GoalBuilding* goalBuilding =
		AddGameObject<GoalBuilding>(3);

	(void)goalBuilding;

	BackgroundObjectManager* backgroundObjectManager =
		AddGameObject<BackgroundObjectManager>(3);

	(void)backgroundObjectManager;

	GroundBlockManager* groundBlockManager =
		AddGameObject<GroundBlockManager>(3);

	if (groundBlockManager != nullptr)
	{
		groundBlockManager->SetGroundData(
			m_LoadData.ground
		);

		groundBlockManager->CreateFloorTiles(
			this,
			stageBlockManager,
			true
		);
	}

	ParticleEffectManager* particleEffectManager =
		AddGameObject<ParticleEffectManager>(7);

	Particle* fireWorkParticle =
		AddGameObject<Particle>(7);

	Particle* floatParticle =
		AddGameObject<Particle>(7);

	Particle* rainParticle =
		AddGameObject<Particle>(7);

	Particle* playerSparkParticle =
		AddGameObject<Particle>(7);

	Particle* playerLandingParticle =
		AddGameObject<Particle>(7);

	if (particleEffectManager != nullptr)
	{
		particleEffectManager->SetParticle(
			ParticleEffectType::FireWork,
			fireWorkParticle
		);

		particleEffectManager->SetParticle(
			ParticleEffectType::Float,
			floatParticle
		);

		particleEffectManager->SetParticle(
			ParticleEffectType::Rain,
			rainParticle
		);

		particleEffectManager->SetParticle(
			ParticleEffectType::PlayerSpark,
			playerSparkParticle
		);

		particleEffectManager->SetParticle(
			ParticleEffectType::PlayerLanding,
			playerLandingParticle
		);
	}

	BonusGaugeUI* gauge =
		AddGameObject<BonusGaugeUI>(8);

	if (gauge != nullptr)
	{
		gauge->SetSize(
			32.0f,
			180.0f
		);
	}

	AddGameObject<SpeedLineEffect>(8);
	AddGameObject<ComboCounterUI>(8);
	AddGameObject<DistanceCounter>(8);
	AddGameObject<Time>(8);

	Polygon2D* readme =
		AddGameObject<Polygon2D>(8);

	if (readme != nullptr)
	{
		readme->SetTexture("asset\\texture\\readme.png");

		float width = 238.5f;
		float height = 543.0f;
		float marginX = 20.0f;
		float marginY = 20.0f;

		readme->SetRect(
			(float)SCREEN_WIDTH - width - marginX,
			(float)SCREEN_HEIGHT - height - marginY,
			width,
			height
		);
	}

	AddGameObject<OptionUI>(9);

	m_BgmPlayer = AudioManager::CreatePlayer();

	SePlayParams bgm;
	bgm.cueName = "randBGM";
	bgm.categoryId = BGM;
	bgm.volume = 0.3f;
	bgm.loopCount = CRIATOMPLAYER_FORCE_LOOP;

	m_BgmPlaybackId = AudioManager::PlaySeStart(m_BgmPlayer, bgm);
}

void Game::Init()
{
	m_GoalCameraFinishWaitFrame = 0;
	m_HasChangedToResult = false;

	LoadDataIfNeeded();

	BuildScene();
}

void Game::Uninit()
{
	if (m_BgmPlaybackId != -1)
	{
		AudioManager::StopPlayback(m_BgmPlaybackId, true);
		m_BgmPlaybackId = -1;
	}

	if (m_BgmPlayer != nullptr)
	{
		AudioManager::DestroyPlayer(m_BgmPlayer);
		m_BgmPlayer = nullptr;
	}

	Scene::Uninit();

	kickUI = nullptr;

	m_HasLoadData = false;

	m_GoalCameraFinishWaitFrame = 0;
	m_HasChangedToResult = false;
}

void Game::Update()
{
	Scene::Update();

#ifdef _DEBUG
	if (GetAsyncKeyState('F') & 0x0001)
	{
		Scene* scene = Manager::GetScene();

		if (scene != nullptr)
		{
			ParticleEffectManager* particleEffectManager =
				scene->GetGameObject<ParticleEffectManager>();

			if (particleEffectManager != nullptr)
			{
				Vector3 fireworkCenter =
					Vector3(25.0f, 18.0f, 13.0f);

				const int fireworkCount = 8;

				const float radius = 8.0f;
				const float height = 5.0f;

				for (int i = 0; i < fireworkCount; i++)
				{
					float angle =
						XM_2PI *
						(float)i /
						(float)fireworkCount;

					Vector3 fireworkPosition =
						fireworkCenter;

					fireworkPosition.x += cosf(angle) * radius;
					fireworkPosition.z += sinf(angle) * radius;
					fireworkPosition.y += height;

					particleEffectManager->EmitFireWork(
						fireworkPosition,
						300
					);
				}
			}
		}
	}
#endif

	if (m_BgmPlayer != nullptr)
	{
		AudioManager::ApplyVolumeToPlayer(
			m_BgmPlayer,
			BGM,
			0.3f
		);
	}

	if (m_HasChangedToResult)
	{
		return;
	}

	Player* player =
		GetGameObject<Player>();

	Time* time =
		GetGameObject<Time>();

	Camera* camera =
		GetGameObject<Camera>();

	if (time != nullptr && time->IsTimeUp())
	{
		m_HasChangedToResult =
			true;

		if (player != nullptr)
		{
			GameData::ResultDistance =
				player->GetProgressDistance();
		}
		else
		{
			GameData::ResultDistance =
				0.0f;
		}

		Manager::SetScene<Result>();
		return;
	}

	if (player != nullptr && player->IsDead())
	{
		m_HasChangedToResult =
			true;

		GameData::ResultDistance =
			player->GetProgressDistance();

		Manager::SetScene<Result>();
		return;
	}

	if (camera == nullptr)
	{
		return;
	}

	if (!camera->IsGoalCameraFinished())
	{
		return;
	}

	m_GoalCameraFinishWaitFrame++;

	if (m_GoalCameraFinishWaitFrame < 600)
	{
		return;
	}

	m_HasChangedToResult =
		true;

	if (player != nullptr)
	{
		GameData::ResultDistance =
			player->GetProgressDistance();
	}
	else
	{
		GameData::ResultDistance =
			0.0f;
	}

	Manager::SetScene<Result>();
}

void Game::Draw()
{
	Scene::Draw();
}