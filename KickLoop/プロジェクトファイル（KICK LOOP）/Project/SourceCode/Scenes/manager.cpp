#include "main.h"
#include "manager.h"
#include "renderer.h"
#include "input.h"
#include "scene.h"
#include "game.h"
#include "title.h"
#include "fade.h"

Scene* Manager::m_Scene = nullptr;
Scene* Manager::m_SceneNext = nullptr;
Fade Manager::m_Fade;
bool Manager::m_LoadSceneInit = true;

//リザルトタイム用
float Manager::m_ResultTime = 0.0f;

void Manager::Init()
{
	Renderer::Init();
	Input::Init();

	m_Fade.Init();  // ← フェード初期化

	m_Scene = new Title();
	m_Scene->Init();
}


void Manager::Uninit()
{
	m_Scene->Uninit();
	delete m_Scene;

	m_Fade.Uninit();

	Texture::Unload();

	Input::Uninit();
	Renderer::Uninit();
}

void Manager::Update()
{
	Input::Update();

	m_Scene->Update();

	m_Fade.Update();


	// シーン切り替え
	if (m_SceneNext != nullptr)
	{
		if (!m_Fade.IsFade() || (m_Fade.IsFinished() && m_Fade.GetState() == Fade::None))
		{
			m_Scene->Uninit();
			delete m_Scene;

			m_Scene = m_SceneNext;

			if (m_LoadSceneInit)
			{
				m_Scene->Init();
			}

			m_SceneNext = nullptr;
			m_LoadSceneInit = true;

			m_Fade.StartFadeIn(1.0f);
		}
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

// 次のシーンと切り替え方法を予約する
void Manager::SetNextScene(Scene* next, bool isFade, bool needInit)
{
	m_SceneNext = next;
	// ロード完了後にInitするかどうか
	m_LoadSceneInit = needInit;
	// フェードするかどうか
	if (isFade)
	{
		m_Fade.StartFadeOut(1.0f);
	}
}