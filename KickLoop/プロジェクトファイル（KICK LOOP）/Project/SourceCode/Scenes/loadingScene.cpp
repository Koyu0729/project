#include "main.h"
#include "manager.h"
#include "loadingScene.h"
#include "game.h"

#include "camera.h"
#include "loadPercent.h"
#include "polygon2D.h"

#include "gameDataLoader.h"

void LoadingScene::Init()
{
	AddGameObject<Camera>(0);

	m_Background = AddGameObject<Polygon2D>(2);

	if (m_Background != nullptr)
	{
		m_Background->SetTexture("asset\\texture\\loadWall.png");

		m_Background->SetRect(
			0.0f,
			0.0f,
			SCREEN_WIDTH,
			SCREEN_HEIGHT
		);
	}

	LoadPercent* loadPercent = AddGameObject<LoadPercent>(3);

	if (loadPercent != nullptr)
	{
		loadPercent->SetTexture("asset\\texture\\load.png");
	}

	m_IsThreadStarted = false;
	m_LoadFinished = false;
	m_LoadSuccess = false;
	m_NextSceneRequested = false;

	StartLoadThread();
}

void LoadingScene::StartLoadThread()
{
	if (m_IsThreadStarted)
	{
		return;
	}

	m_IsThreadStarted = true;

	m_LoadThread = std::thread([this]()
		{
#ifdef _DEBUG
			printf("ロード開始\n");
#endif

			GameLoadData loadData;

			bool loadSuccess = GameDataLoader::LoadStageData(
				"asset\\data\\stageData.json",
				loadData
			);

			if (!loadSuccess)
			{
#ifdef _DEBUG
				printf("stageData.json の読み込みに失敗!!!デフォルトデータを使用\n");
#endif

				loadData = GameDataLoader::CreateDefaultGameLoadData();
			}

			{
				std::lock_guard<std::mutex> lock(m_LoadMutex);

				m_LoadData = loadData;
				m_LoadSuccess = true;
				m_LoadFinished = true;
			}

#ifdef _DEBUG
			printf("ロード完了\n");
#endif
		});
}

void LoadingScene::Uninit()
{
	if (m_LoadThread.joinable())
	{
		m_LoadThread.join();
	}

	Scene::Uninit();

	m_Background = nullptr;

	m_IsThreadStarted = false;
	m_LoadFinished = false;
	m_LoadSuccess = false;
	m_NextSceneRequested = false;
}

void LoadingScene::Update()
{
	Scene::Update();

	if (m_NextSceneRequested)
	{
		return;
	}

	bool loadFinished = false;
	bool loadSuccess = false;
	GameLoadData loadData;

	{
		std::lock_guard<std::mutex> lock(m_LoadMutex);

		if (m_LoadFinished)
		{
			loadFinished = true;
			loadSuccess = m_LoadSuccess;
			loadData = m_LoadData;

			m_LoadFinished = false;
		}
	}

	if (!loadFinished)
	{
		return;
	}

	if (m_LoadThread.joinable())
	{
		m_LoadThread.join();
	}

	if (!loadSuccess)
	{
#ifdef _DEBUG
		printf("LoadingScene: ロードに失敗しました\n");
#endif
		return;
	}

	Game* game = new Game();

	game->SetLoadData(loadData);

	game->Init();

	m_NextSceneRequested = true;

	Manager::SetNextScene(game, false, false);
}

void LoadingScene::Draw()
{
	Scene::Draw();
}