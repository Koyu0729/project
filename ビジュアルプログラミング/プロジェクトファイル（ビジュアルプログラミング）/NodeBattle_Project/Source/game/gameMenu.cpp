#include "main.h"
#include "manager.h"
#include "gameMenu.h"
#include "game.h"
#include "nodeEditor.h"
#include "polyTex.h"
#include "moji.h"
#include "camera.h"
#include "input.h"
#include "button.h"

// 通常のメニュー表示では、ステージの種類を選ぶ画面から開始する
StageMenuPage GameMenu::m_InitialPage = StageMenuPage::Main;
// CPU側の追加読み込みは使わない
bool GameMenu::InitCPU()
{
	if (!Scene::InitCPU())
	{
		return false;
	}

	return true;
}

// GPU側の追加読み込みは使わない
bool GameMenu::InitGPU()
{
	if (!Scene::InitGPU())
	{
		return false;
	}

	return true;
}

// ステージ選択の背景とボタンを用意する
bool GameMenu::Init()
{
	AddGameObject3D<Camera>(0);

	AddGameObject2D<PolyTex>(1)->SetTexRule(Vector2(0, 0), 0, Vector2(SCREEN_WIDTH, SCREEN_HEIGHT), "asset\\texture\\wallpaper\\titleBattle.png");

	// 草原の背景を残しながら、一覧の文字とボタンを読みやすくする
	AddGameObject2D<PolyTex>(2)->SetTexRule(Vector2(40, 110), 0, Vector2(1200, 530), "asset\\texture\\kuro.png")->SetColor(XMFLOAT4(1, 1, 1, 0.82f));

	// 入口3つと、チュートリアル9つ、実践3つ、戻るボタン
	const wchar_t *mainNames[] = {L"自由に編集", L"チュートリアルステージ", L"実践ステージ"};

	for (int i = 0; i < 3; ++i)
	{
		auto *button = AddGameObject2D<Button>(3);

		button->SetName(mainNames[i]);
		button->SetCaptionSize(22);
		button->SetTexRule(Vector2(130 + i * 350.0f, 460), 0, Vector2(320, 80), "asset\\texture\\wallpaper\\editor.jpg");
	}

	for (int i = 0; i < Game::TutorialCount; ++i)
	{
		auto *button = AddGameObject2D<Button>(3);

		button->SetName(L"ステージ" + std::to_wstring(i + 1));
		button->SetCaptionSize(22);
		button->SetTexRule(Vector2(80 + (i % 4) * 280.0f, 360 + (i / 4) * 100.0f), 0, Vector2(260, 80), "asset\\texture\\wallpaper\\editor.jpg");
	}

	// 実践は自分のノードで挑戦する3ステージ
	for (int number = 1; number <= Game::PracticeCount; ++number)
	{
		auto *button = AddGameObject2D<Button>(3);

		button->SetName(L"実践ステージ" + std::to_wstring(number));

		button->SetCaptionSize(22);

		button->SetTexRule(Vector2(130 + (number - 1) * 350.0f, 460), 0, Vector2(320, 80), "asset\\texture\\wallpaper\\editor.jpg");
	}

	auto *back = AddGameObject2D<Button>(3);

	back->SetName(L"戻る");

	back->SetCaptionSize(22);

	back->SetTexRule(Vector2(30, 650), 0, Vector2(320, 50), "asset\\texture\\wallpaper\\editor.jpg");

	ShowPage(m_InitialPage);

	m_InitialPage = StageMenuPage::Main;

	// メニューのBGM
	m_params.cueName = "bgm";

	m_params.categoryId = BGM;

	m_params.is3D = false;

	m_params.loopCount = CRIATOMPLAYER_FORCE_LOOP;

	m_params.volume = 0.1f;

	m_Source = criAtomEx3dSource_Create(nullptr, nullptr, 0);

	m_SourceId = AudioManager::PlaySeStart(m_params, m_Source);

	return true;
}

// BGMを止めて画面を解放する
void GameMenu::Uninit()
{
	if (m_SourceId != -1)
	{
		AudioManager::StopPlayback(m_SourceId, true);

		m_SourceId = -1;
	}

	if (m_Source)
	{
		criAtomEx3dSource_Destroy(m_Source);

		m_Source = nullptr;
	}

	Scene::Uninit();
}

// ボタンを押したら一覧を切り替えるか、選んだステージの編集に進む
void GameMenu::Update()
{
	Scene::Update();

	if (IsButtonClicked(L"戻る") || (m_Page != StageMenuPage::Main && Input::GetKeyTrigger(VK_ESCAPE)))
	{
		ShowPage(StageMenuPage::Main);

		return;
	}

	if (m_Page == StageMenuPage::Main)
	{
		if (IsButtonClicked(L"チュートリアルステージ"))
		{
			ShowPage(StageMenuPage::Tutorial);
		}
		else if (IsButtonClicked(L"実践ステージ"))
		{
			ShowPage(StageMenuPage::Practice);
		}
		else if (IsButtonClicked(L"自由に編集"))
		{
			Game::SelectStage(0);

			Manager::LoadingSetScene<NodeEditor>();
		}

		return;
	}

	if (m_Page == StageMenuPage::Tutorial)
	{
		for (int i = 0; i < Game::TutorialCount; ++i)
		{
			if (IsButtonClicked((L"ステージ" + std::to_wstring(i + 1)).c_str()))
			{
				Game::SelectStage(Game::GetTutorialStage(i + 1), true);

				Manager::LoadingSetScene<NodeEditor>();

				return;
			}
		}
	}
	else if (m_Page == StageMenuPage::Practice)
	{
		for (int number = 1; number <= Game::PracticeCount; ++number)
		{
			if (IsButtonClicked((L"実践ステージ" + std::to_wstring(number)).c_str()))
			{
				Game::SelectStage(Game::GetPracticeStage(number));

				Manager::LoadingSetScene<NodeEditor>();

				return;
			}
		}
	}
}

// 一覧の種類と、遊び方を表示する
void GameMenu::Draw()
{
	Scene::Draw();

	FontData font;

	font.font = L"メイリオ";

	font.fontSize = 30;

	font.Color = D2D1::ColorF(0.65f, 1.0f, 0.8f);

	Moji::Instance().SetFont(font);

	const wchar_t *instructionPositionY1;

	if (m_Page == StageMenuPage::Main)
	{
		instructionPositionY1 = L"ノードを組んで、敵を倒そう！";
	}
	else if (m_Page == StageMenuPage::Tutorial)
	{
		instructionPositionY1 = L"チュートリアルステージ";
	}
	else
	{
		instructionPositionY1 = L"実践ステージ";
	}

	const wchar_t *heading = instructionPositionY1;

	Moji::Instance().Draw(heading, D2D1::Point2F(340, 170), 1);

	font.fontSize = 22;

	font.Color = D2D1::ColorF(D2D1::ColorF::White);

	Moji::Instance().SetFont(font);

	const wchar_t *instructionPositionY2;

	if (m_Page == StageMenuPage::Main)
	{
		instructionPositionY2 = L"まずはチュートリアルで覚えて、実践ステージに挑戦しよう。";
	}
	else if (m_Page == StageMenuPage::Tutorial)
	{
		instructionPositionY2 = L"1：槍   2：回避   3：壁   4：盾で射撃を防ぐ";
	}
	else
	{
		instructionPositionY2 = L"1：槍の追撃   2：射撃と回避   3：盾・剣・槍（サンプルなし）";
	}

	const wchar_t *description = instructionPositionY2;

	Moji::Instance().Draw(description, D2D1::Point2F(230, 250), 1);

	if (m_Page == StageMenuPage::Tutorial)
	{
		Moji::Instance().Draw(L"5：射撃回避   6：盾の横   7：槍と剣   8：相性の総合練習", D2D1::Point2F(230, 285), 1);
	}

	int instructionPositionY3;

	if (m_Page == StageMenuPage::Tutorial)
	{
		instructionPositionY3 = 315;
	}
	else
	{
		instructionPositionY3 = 330;
	}

	Moji::Instance().Draw(L"ステージを選ぶ → ノードを編集 → 保存して挑戦", D2D1::Point2F(310, instructionPositionY3), 1);

	if (m_Page == StageMenuPage::Tutorial)
	{
		Moji::Instance().Draw(L"9：射撃の距離で槍に勝つ", D2D1::Point2F(380, 580), 1);
	}
}

// 開いている種類のボタンだけを表示し、隠れたボタンは押せなくする
void GameMenu::ShowPage(StageMenuPage page)
{
	m_Page = page;

	for (const wchar_t *name : {L"自由に編集", L"チュートリアルステージ", L"実践ステージ"})
	{
		GetGameObject<Button>(name)->SetVisible(page == StageMenuPage::Main);
	}

	for (int number = 1; number <= Game::TutorialCount; ++number)
	{
		GetGameObject<Button>(L"ステージ" + std::to_wstring(number))->SetVisible(page == StageMenuPage::Tutorial);
	}

	for (int number = 1; number <= Game::PracticeCount; ++number)
	{
		GetGameObject<Button>(L"実践ステージ" + std::to_wstring(number))->SetVisible(page == StageMenuPage::Practice);
	}

	GetGameObject<Button>(L"戻る")->SetVisible(page != StageMenuPage::Main);
}

// 表示中のメニューボタンのクリックを確認する
bool GameMenu::IsButtonClicked(const wchar_t *name)
{
	auto *button = GetGameObject<Button>(name);

	return button->GetVisible() && button->GetHitBox()->Clicked();
}
