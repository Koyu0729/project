#include "main.h"
#include "manager.h"
#include "renderer.h"

// シーン
#include "title.h"

// 次のシーン
#include "gameMenu.h"

// シーン背景
#include "polyTex.h"

// ロゴ
#include "moji.h"

// UI
#include "camera.h"
#include "input.h"

#include "button.h"

bool Title::InitCPU()
{
	if (!Scene::InitCPU())
	{
		return false;
	}

	return true;
}

bool Title::InitGPU()
{
	if (!Scene::InitGPU())
	{
		return false;
	}

	return true;
}

// 草原で赤と青が戦うタイトル背景と、開始ボタンを用意する
bool Title::Init()
{
	m_TitleFont.font = L"ZeroGothic";
	m_TitleFont.shadowOffset = D2D1::Point2F(0, 0);

	AddGameObject3D<Camera>(0);

	// タイトル背景を画面全体に表示する
	AddGameObject2D<PolyTex>(2)->SetTexRule(Vector2(0, 0), 0, Vector2(SCREEN_WIDTH, SCREEN_HEIGHT), "asset\\texture\\wallpaper\\titleBattle.png");

	// 文字の背景を暗くして、草の上でも読みやすくする
	AddGameObject2D<PolyTex>(3)->SetTexRule(Vector2(35, 30), 0, Vector2(760, 185), "asset\\texture\\kuro.png")->SetColor(XMFLOAT4(1, 1, 1, 0.72f));

	auto *start = AddGameObject2D<Button>(8);
	start->SetName(L"はじめる");
	start->SetCaptionSize(24);
	start->SetTexRule(Vector2(490, 545), 0, Vector2(300, 65), "asset\\texture\\wallpaper\\editor.jpg");

	// 使用している音声ライブラリのロゴ
	AddGameObject2D<PolyTex>(9)->SetTexRule(Vector2(30, SCREEN_HEIGHT - 80), 0, Vector2(65, 65), "asset\\texture\\criwarelogo_PNG\\criware_logo01.png");

	// タイトルのBGMを再生する
	m_params.cueName = "bgm";
	m_params.categoryId = BGM;
	m_params.is3D = false;
	m_params.loopCount = CRIATOMPLAYER_FORCE_LOOP;
	m_params.volume = 0.1f;
	m_Source = criAtomEx3dSource_Create(nullptr, nullptr, 0);
	m_SourceId = AudioManager::PlaySeStart(m_params, m_Source);
	ShowCursor(TRUE);

	return true;
}
void Title::Uninit()
{
	// BGM停止（即時停止）
	if (m_SourceId != -1)
	{
		AudioManager::StopPlayback(m_SourceId, true);
		m_SourceId = -1;
	}

	// ソース破棄
	if (m_Source)
	{
		criAtomEx3dSource_Destroy(m_Source);
		m_Source = nullptr;
	}

	Scene::Uninit();
}

// 開始ボタンか決定キーでステージ選択へ進む
void Title::Update()
{
	Scene::Update();

	if (GetGameObject<Button>(L"はじめる")->GetHitBox()->Clicked() || Input::GetKeyTrigger(VK_RETURN) || Input::GetKeyTrigger(VK_SPACE))
	{
		Manager::LoadingSetScene<GameMenu>();
	}
}

// 見出しと、色分けした対戦・ノードの説明を重ねる
void Title::Draw()
{
	Scene::Draw();

	DrawTitleText(L"NODE / BATTLE SYSTEM", 60, 45, 16, D2D1::ColorF(0.6f, 0.95f, 0.85f));
	DrawTitleText(L"ビジュアルプログラミング", 60, 80, 48, D2D1::ColorF(1, 1, 1));
	DrawTitleText(L"動きをつなぐ。作戦で勝つ。", 60, 160, 24, D2D1::ColorF(0.85f, 0.95f, 0.85f));
	DrawTitleText(L"Enter / Space でも開始", 505, 625, 16, D2D1::ColorF(1, 1, 1));
}

// タイトルの書式を設定して文字を描く
void Title::DrawTitleText(const wchar_t *value, float x, float y, float size, D2D1_COLOR_F color)
{
	m_TitleFont.fontSize = size;
	m_TitleFont.Color = color;
	Moji::Instance().SetFont(m_TitleFont);
	Moji::Instance().Draw(value, D2D1::Point2F(x, y));
}
