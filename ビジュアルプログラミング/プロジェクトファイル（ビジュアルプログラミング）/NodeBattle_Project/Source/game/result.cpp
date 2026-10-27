#include "main.h"
#include "manager.h"
#include "renderer.h"

// シーン
#include "result.h"

// 次のシーン
#include "title.h"
#include "game.h"
#include "gameMenu.h"
#include "nodeEditor.h"

// シーン背景
#include "polyTex.h"

// ロゴ
#include "moji.h"

// UI
#include "camera.h"
#include "input.h"
#include "button.h"
#include "textObjectBase.h"

std::wstring Result::m_Outcome = L"対戦終了";
// CPU側のデータを準備する
bool Result::InitCPU()
{
	if (!Scene::InitCPU())
	{
		return false;
	}

	return true;
}

// 描画に必要なデータを準備する
bool Result::InitGPU()
{
	if (!Scene::InitGPU())
	{
		return false;
	}

	return true;
}

// 初期化処理を行う
bool Result::Init()
{
	AddGameObject3D<Camera>(0);

	const char *backgroundTexture;

	if (m_Outcome == L"勝利！" || m_Outcome == L"ステージクリア！")
	{
		backgroundTexture = "asset\\texture\\wallpaper\\victory.png";
	}
	else if (m_Outcome == L"敗北" || m_Outcome == L"再挑戦しよう")
	{
		backgroundTexture = "asset\\texture\\wallpaper\\defeat.png";
	}

	else
	{
		backgroundTexture = "asset\\texture\\wallpaper\\titleBattle.png";
	}

	auto *wallpaper = AddGameObject2D<PolyTex>(2)
						->SetTexRule(
							Vector2(0.0f, 0.0f),
							0.0f,
							Vector2(SCREEN_WIDTH, SCREEN_HEIGHT),
							backgroundTexture);

	wallpaper->SetName(L"wallpaper");
	wallpaper->AddHitBox(true, true);

	// 結果と操作は左側にまとめ、イラストの駒を隠さないようにする
	AddGameObject2D<PolyTex>(3)->SetTexRule(Vector2(35, 80), 0, Vector2(520, 550), "asset\\texture\\kuro.png")->SetColor(XMFLOAT4(1, 1, 1, 0.78f));

	if (Game::m_StageNumber == 0)
	{
		auto *button = AddGameObject2D<Button>(4);
		button->SetName(L"タイトルへ戻る");
		button->SetCaptionSize(22);
		button->SetTexRule(Vector2(70, 420), 0, Vector2(440, 60), "asset\\texture\\wallpaper\\editor.jpg");
	}
	// ステージでは、ノードを直して再挑戦したり、次のステージへ進める
	if (Game::m_StageNumber != 0)
	{
		std::wstring nextName = L"ステージ" + std::to_wstring(Game::GetTutorialNumber() + 1) + L"へ";

		const wchar_t *names[] = {L"編集して再挑戦", L"ステージ選択", nextName.c_str()};

		for (int i = 0; i < 3; ++i)
		{
			if (i == 2 && (Game::GetNextTutorialStage() == 0 || m_Outcome != L"ステージクリア！"))
			{
				continue;
			}

			auto *button = AddGameObject2D<Button>(4);

			button->SetName(names[i]);

			button->SetCaptionSize(22);

			button->SetTexRule(Vector2(70, 390 + i * 75.0f), 0, Vector2(440, 60), "asset\\texture\\wallpaper\\editor.jpg");
		}
	}

	// サウンド初期化
	m_params.cueName = "bgm";					   // Cue名
	m_params.categoryId = BGM;					   // BGMカテゴリ
	m_params.is3D = false;						   // 位置に依存しない2D音として再生する
	m_params.loopCount = CRIATOMPLAYER_FORCE_LOOP; // ループ再生
	m_params.volume = 0.1f;
	// 先にソースを作成
	m_Source = criAtomEx3dSource_Create(NULL, NULL, 0);

	// ソースを渡して再生開始
	m_SourceId = AudioManager::PlaySeStart(m_params, m_Source);

	ShowCursor(TRUE);

	return true;
}

// 終了処理を行う
void Result::Uninit()
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

// 毎フレームの処理
void Result::Update()
{
	Scene::Update();

	if (Game::m_StageNumber != 0)
	{
		if (GetGameObject<Button>(L"編集して再挑戦")->GetHitBox()->Clicked() || Input::GetKeyTrigger(VK_RETURN))
		{
			Manager::LoadingSetScene<NodeEditor>();
		}
		else if (GetGameObject<Button>(L"ステージ選択")->GetHitBox()->Clicked())
		{
			if (Game::IsPracticeStage())
			{
				GameMenu::m_InitialPage = StageMenuPage::Practice;
			}
			else
			{
				GameMenu::m_InitialPage = StageMenuPage::Tutorial;
			}

			Manager::LoadingSetScene<GameMenu>();
		}
		else if (auto *next = GetGameObject<Button>(L"ステージ" + std::to_wstring(Game::GetTutorialNumber() + 1) + L"へ"))
		{
			if (next->GetHitBox()->Clicked())
			{
				Game::SelectStage(Game::GetNextTutorialStage(), true);

				Manager::LoadingSetScene<NodeEditor>();
			}
		}

		return;
	}

	if (GetGameObject<Button>(L"タイトルへ戻る")->GetHitBox()->Clicked() || Input::GetKeyTrigger(VK_RETURN))
	{
		Manager::LoadingSetScene<Title>();
	}
}

// 結果の文字を左側のパネルへ表示する
void Result::Draw()
{
	Scene::Draw();

	FontData font;
	font.font = L"メイリオ";
	font.fontSize = 22;
	font.Color = D2D1::ColorF(D2D1::ColorF::White);
	Moji::Instance().SetFont(font);

	if (Game::m_StageNumber != 0)
	{
		Moji::Instance().Draw(Game::GetStageName(), D2D1::Point2F(70, 115));
	}

	font.fontSize = 42;
	if (m_Outcome == L"勝利！" || m_Outcome == L"ステージクリア！")
	{
		font.Color = D2D1::ColorF(1, 0.9f, 0.3f);
	}
	Moji::Instance().SetFont(font);
	Moji::Instance().Draw(m_Outcome, D2D1::Point2F(70, 195));

	font.fontSize = 20;
	font.Color = D2D1::ColorF(D2D1::ColorF::White);
	Moji::Instance().SetFont(font);

	const wchar_t *message = L"ノードを組み直して、もう一度挑戦しよう。";
	if (m_Outcome == L"勝利！" || m_Outcome == L"ステージクリア！")
	{
		message = L"ノードの作戦で敵を倒せました！";
	}
	else if (m_Outcome == L"引き分け" || m_Outcome == L"対戦終了")
	{
		message = L"作戦を変えて、もう一度遊んでみよう。";
	}
	Moji::Instance().Draw(message, D2D1::Point2F(70, 290));
}