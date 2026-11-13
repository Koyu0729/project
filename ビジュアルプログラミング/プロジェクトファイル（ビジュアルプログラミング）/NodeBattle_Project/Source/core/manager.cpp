#include "main.h"
#include "manager.h"
#include "renderer.h"
#include "input.h"
#include "texture.h"
#include "modelRenderer.h"

#include "scene.h"
#include "title.h"
#include "game.h"

ManagerState Manager::m_State = ManagerState::Default;

Scene *Manager::m_Scene = nullptr;
Scene *Manager::m_SceneNext = nullptr;
Fade Manager::m_Fade;
Scene *Manager::m_PreparingScene = nullptr;
bool Manager::m_NextGPUStarted = false;
bool Manager::m_LoadFailed = false;
void Manager::Init()
{
	Renderer::Init();
	Input::Init();

	m_Fade.Init(); // フェード初期化

	m_Scene = new Title();
	m_Scene->Init();
}

Scene *(*Manager::m_CreateScene)() = nullptr;

// ロード用
std::atomic<bool> Manager::m_Loading{false};
std::thread Manager::m_LoadThread;
// ロード完了を待ち、シーン・共有リソース・デバイスの順に終了する
void Manager::Uninit()
{
	if (m_LoadThread.joinable())
	{
		m_LoadThread.join();
	}

	m_Loading = false;
	ClearPendingScene();
	m_LoadFailed = false;
	m_CreateScene = nullptr;

	if (m_Scene)
	{
		m_Scene->Uninit();
		delete m_Scene;
		m_Scene = nullptr;
	}

	m_Fade.Uninit();
	ModelRenderer::UnloadAll();
	Texture::UnloadAll();
	Input::Uninit();
	Renderer::Uninit();
	m_State = ManagerState::Default;
}

// 切り替えの順番：暗転 → CPU読み込み → GPU用データ作成 → 次の画面を表示
void Manager::Update()
{
	Input::Update();
	m_Scene->Update();
	m_Fade.Update();

	switch (m_State)
	{
	case ManagerState::Default:
		break;

	// 1. 暗転後に読み込み画面を表示し、別スレッドでInitCPUを開始する
	case ManagerState::CreateLoadScene:

		if (m_Fade.IsFinished() && m_Fade.GetState() == Fade::FadeOut)
		{
			// 現在のシーンを終了し、読み込み画面へ切り替える
			if (m_Scene)
			{
				m_Scene->Uninit();
				delete m_Scene;
			}

			m_Scene = new LoadingScene();
			m_Scene->Init();

			m_Fade.StartFadeIn(1.0f);
			m_State = ManagerState::LoadingScene;

			if (!StartLoading())
			{
				ReturnToTitle();
			}
		}
		break;

	// 2. 別スレッドのファイル読み込みが終わるまで、この画面を更新・描画する
	case ManagerState::LoadingScene:

		if (!m_Loading && m_Fade.GetState() == Fade::None)
		{
			if (m_LoadThread.joinable())
			{
				m_LoadThread.join();
			}

			if (m_LoadFailed)
			{
				ReturnToTitle();
				break;
			}

			// 初期化が失敗したらタイトルへ戻す
			if (m_SceneNext)
			{
				m_NextGPUStarted = true;
				m_PreparingScene = m_SceneNext;

				// 3. 読み込んだデータから描画用バッファを作り、次の画面を初期化する
				// この処理はメイン側で行うため、処理中は画面の更新も待つ。失敗したらタイトルへ戻す
				bool initialized = false;
				try
				{
					initialized = m_SceneNext->InitGPU() && m_SceneNext->Init();
				}
				catch (...)
				{
					initialized = false;
				}

				if (!initialized)
				{
					ReturnToTitle();
					break;
				}

				m_PreparingScene = nullptr;
				m_Fade.StartFadeOut(1.0f);
				m_State = ManagerState::ChangeScene;
			}
		}
		break;

	// 4. 準備済みの次シーンへ切り替え、フェードインで表示する
	case ManagerState::ChangeScene:

		if (m_Fade.IsFinished() && m_Fade.GetState() == Fade::FadeOut)
		{
			// ロード完了後、実際にシーン切り替え
			if (m_Scene)
			{
				m_Scene->Uninit();
				delete m_Scene;
			}

			m_Scene = m_SceneNext;
			m_SceneNext = nullptr;
			m_NextGPUStarted = false;

			m_Fade.StartFadeIn(1.0f);
			m_State = ManagerState::Default;
		}
		break;
	}

}

void Manager::Draw()
{
	Renderer::Begin();

	m_Scene->Draw();

	// フェードを一番上に描画
	m_Fade.Draw();

	Renderer::End();
}

// ロード画面を表示した後に、CPUの読み込みを開始する
bool Manager::StartLoading()
{
	m_Loading = true;

	// std::threadは失敗を例外で返すため、ここだけでfalseへ変換する
	try
	{
		m_LoadThread = std::thread(&Manager::LoadSceneCPU);
	}
	catch (...)
	{
		m_Loading = false;
		return false;
	}

	return true;
}

// 切り替え中止時の解放：CPUデータだけならdelete、GPU初期化を開始した後はUninitも呼ぶ
void Manager::ClearPendingScene()
{
	if (m_SceneNext)
	{
		// GPUの初期化を開始したシーンは、途中失敗でもUninitでリソースを解放する
		if (m_NextGPUStarted)
		{
			m_SceneNext->Uninit();
		}

		delete m_SceneNext;
		m_SceneNext = nullptr;
	}

	m_NextGPUStarted = false;
	m_PreparingScene = nullptr;
}

// 前のロードを待ち、次のシーンの生成方法を予約する
void Manager::PrepareSceneLoad(Scene *(*createScene)())
{
	if (m_LoadThread.joinable())
	{
		m_LoadThread.join();
	}

	m_Loading = false;
	ClearPendingScene();
	m_LoadFailed = false;
	m_CreateScene = createScene;
	m_Fade.StartFadeOut(1.0f);
	m_State = ManagerState::CreateLoadScene;
}

// 生成途中のシーンはここで管理し、成功した場合だけManagerへ渡す
void Manager::LoadSceneCPU()
{
	// 画像の読み込みに必要なCOMを、このスレッドでも初期化する
	const HRESULT com = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
	if (FAILED(com))
	{
		m_LoadFailed = true;
		m_Loading = false;
		return;
	}

	Scene *next = nullptr;
	bool loaded = false;
	// ファイルの解析やメモリ確保の失敗も、タイトルへ戻るための結果にする
	try
	{
		if (m_CreateScene)
		{
			next = m_CreateScene();
			if (next)
			{
				loaded = next->InitCPU();
			}
		}
	}
	catch (...)
	{
		loaded = false;
	}

	if (loaded)
	{
		m_SceneNext = next;
	}
	else
	{
		// CPUの読み込みだけで終了したシーンを解放する
		delete next;
		m_LoadFailed = true;
	}

	CoUninitialize();
	// メイン側はこの通知の後にjoinして、読み込み結果を確認する
	m_Loading = false;
}

// CPUまたはGPUのロード失敗時に、準備中のシーンを解放してタイトルへ戻す
void Manager::ReturnToTitle()
{
	ClearPendingScene();
	m_LoadFailed = false;
	m_CreateScene = nullptr;
	m_Scene->Uninit();
	delete m_Scene;
	m_Scene = new Title();
	m_Scene->Init();
	m_State = ManagerState::Default;
	m_Fade.StartFadeIn(1.0f);
	OutputDebugStringA("Scene loading failed; returned to title.\n");
}

// CPUの読み込みスレッドの終了を待つ
void Manager::WaitForLoading()
{
	if (m_LoadThread.joinable())
	{
		m_LoadThread.join();
	}
}

// シーンの切り替え状態を取得する
ManagerState Manager::GetState()
{
	return m_State;
}

