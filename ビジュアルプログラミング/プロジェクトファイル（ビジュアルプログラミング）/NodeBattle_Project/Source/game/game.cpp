#include "main.h"
#include "game.h"
#include "manager.h"
#include "nodeEditor.h"
#include "result.h"
#include "camera.h"
#include "field.h"
#include "player.h"
#include "nodeRuntime.h"
#include "nodesSaveLoad.h"
#include "input.h"
#include "moji.h"
#include "button.h"
#include "polyTex.h"
#include "mouseInput.h"
#include "stageObstacle.h"
#include <sstream>
#include <iomanip>
#include "nodeGraphData.h"
#include "stageData.h"
#include "nodeBase.h"

// ステージ選択は、編集・対戦・結果の画面をまたいで残す
int Game::m_StageNumber = 0;

bool Game::m_OpenStageExample = false;
// 切り替え前の終了でも、まだプレイヤーへ渡していないノードを解放する
Game::~Game()
{
	NodeGraphData::Delete(m_PreparedPlayer);
	NodeGraphData::Delete(m_PreparedEnemy);
}

// CPU読み込み：プレイヤーと敵のノードを先に読む。画面上のプレイヤーはまだ生成しない
bool Game::InitCPU()
{
	if (!Scene::InitCPU())
	{
		return false;
	}
	NodesSaveLoad save;
	// ノードを一時保存し、プレイヤー生成後に所有権を渡す
	m_PreparedPlayer = save.NodeRuntimeLoad("nodeSaveData/PlayerData.json");

	if (!save.EnemyRuntimeLoad(GetEnemySavePath(), m_PreparedEnemy, m_PreparedEnemyHP))
	{
		m_Error = save.GetMessage();
	}

	m_CPUPrepared = true;

	return true;
}

bool Game::InitGPU()
{
	if (!Scene::InitGPU())
	{
		return false;
	}
	// 描画データを準備し、対戦画面を初期化する
	return Init();
}

// 初期化処理を行う
bool Game::Init()
{
	if (m_Initialized)
	{
		return true;
	}

	m_Initialized = true;

	AddGameObject3D<Camera>(0);

	AddGameObject3D<Field>(2);

	// 対戦中の操作は、画面下のボタンからも使えるようにする
	AddGameObject2D<PolyTex>(8)->SetTexRule(Vector2(10, 650), 0, Vector2(1260, 65), "asset\\texture\\kuro.png")->SetColor(XMFLOAT4(1, 1, 1, 0.8f));
	const wchar_t *controls[] = {L"停止 / 再開", L"編集に戻る", L"カメラ1：追従", L"カメラ2：横", L"カメラ3：俯瞰"};
	for (int i = 0; i < 5; ++i)
	{
		auto *button = AddGameObject2D<Button>(9);
		button->SetName(controls[i]);
		button->SetCaptionSize(20);
		button->SetTexRule(Vector2(25 + i * 248.0f, 660), 0, Vector2(235, 45), "asset\\texture\\wallpaper\\editor.jpg");
	}

	if (m_StageNumber == 3)
	{
		AddGameObject3D<StageObstacle>(2);
	}

	NodesSaveLoad save;

	// 全ステージに共通の外周の壁を置く
	for (int side = 0; side < 4; ++side)
	{
		AddGameObject3D<StageObstacle>(2)->SetArenaSide(side);
	}

	m_Player = AddGameObject3D<Player>(3);

	m_Player->SetName(L"player");

	m_Player->SetTeam(Team::TeamA);

	if (m_CPUPrepared)
	{
		m_Player->SetRunNodes(std::move(m_PreparedPlayer));
		m_PreparedPlayer.clear();
	}
	else
	{
		m_Player->SetRunNodes(save.NodeRuntimeLoad("nodeSaveData/PlayerData.json"));
	}

	m_Enemy = AddGameObject3D<Player>(3);

	m_Enemy->SetName(L"enemy");

	m_Enemy->SetTeam(Team::TeamB);

	// HPと行動は、自由対戦・チュートリアル・実践の保存データから読み込む
	if (m_CPUPrepared)
	{
		m_Enemy->SetInitialHP(m_PreparedEnemyHP);
		m_Enemy->SetRunNodes(std::move(m_PreparedEnemy));
		m_PreparedEnemy.clear();
	}
	else
	{
		LoadEnemyData(m_Enemy, m_Error);
	}

	// ステージでは攻撃ノードで戦い、棒に触れるだけのダメージを使わない
	if (m_StageNumber != 0)
	{
		m_Player->SetSwordDamageEnabled(false);

		m_Enemy->SetSwordDamageEnabled(false);
	}

	m_Player->SetPosition(Vector3(0, 1.5f, 0));

	m_Enemy->SetPosition(Vector3(0, 1.5f, 4));

	m_Player->SetTargetObject(m_Enemy);

	m_Enemy->SetTargetObject(m_Player);

	// 2Pはカウントダウンの時点から、1Pの方を向いておく
	Vector3 direction = m_Player->GetPosition() - m_Enemy->GetPosition();

	m_Enemy->SetRotation(Vector3(0, std::atan2(direction.x, direction.z), 0));

	std::wstring message;

	if (!NodeRuntime::Validate(m_Player->GetRunNodes(), message))
	{
		m_Error = L"プレイヤー：" + message;
	}
	else if (m_Error.empty() && !NodeRuntime::Validate(m_Enemy->GetRunNodes(), message))
	{
		m_Error = L"相手：" + message;
	}

	ShowCursor(TRUE);

	m_CPUPrepared = false;

	return true;
}

// 終了処理を行う
void Game::Uninit()
{
	if (!m_Initialized)
	{
		return;
	}

	m_Initialized = false;

	Scene::Uninit();
}

// 毎フレームの処理
void Game::Update()
{
	// 停止中やエラー表示中でも操作できるよう、対戦の更新より先に判定する

	const wchar_t *cameraButtons[] = {L"カメラ1：追従", L"カメラ2：横", L"カメラ3：俯瞰"};
	for (int i = 0; i < 3; ++i)
	{
		if (IsButtonClicked(cameraButtons[i]))
		{
			GetGameObject<Camera>()->SetViewNumber(i + 1);
		}
	}

	if (Input::GetKeyTrigger(VK_F8) || IsButtonClicked(L"編集に戻る"))
	{
		m_Player->SetCanStartNode(false);

		m_Enemy->SetCanStartNode(false);

		Manager::LoadingSetScene<NodeEditor>();

		return;
	}

	if (!m_Error.empty() || m_Finished)
	{
		return;
	}

	if ((Input::GetKeyTrigger(VK_F6) || IsButtonClicked(L"停止 / 再開")) && m_Countdown <= 0)
	{
		m_Paused = !m_Paused;

		for (auto *player : {m_Player, m_Enemy})
		{
			if (m_Paused)
			{
				player->GetNodeRuntime()->Pause();

				player->ClearNodeCommands();
			}
			else
			{
				player->GetNodeRuntime()->Resume();
			}
		}
	}

	if (m_Paused)
	{
		GetGameObject<Camera>()->Update();

		return;

	} // 一時停止中もカメラを切り替えられる。
	const float dt = GetFrameDeltaSeconds();

	if (m_Countdown > 0)
	{
		m_Countdown -= dt;

		if (m_Countdown <= 0)
		{
			m_Player->SetCanStartNode(true);

			m_Enemy->SetCanStartNode(true);
		}
	}
	else
	{
		m_Elapsed += dt;
	}

	Scene::Update();

	for (auto *player : {m_Player, m_Enemy})
	{
		if (player->GetNodeRuntime()->GetState() == RuntimeState::Error)
		{
			m_Error = player->GetNodeRuntime()->GetMessage();

			m_Player->SetCanStartNode(false);

			m_Enemy->SetCanStartNode(false);

			return;
		}
	}

	if (m_Player->GetHP() <= 0 || m_Enemy->GetHP() <= 0 || m_Elapsed >= 60)
	{
		const int playerHP = m_Player->GetHP(), enemyHP = m_Enemy->GetHP();

		// ステージのクリア判定と自由対戦の勝敗を表示する
		const wchar_t *outcome;

		if (m_StageNumber != 0)
		{
			if (playerHP > 0 && enemyHP <= 0)
			{
				outcome = L"ステージクリア！";
			}
			else
			{
				outcome = L"再挑戦しよう";
			}
		}
		else if (playerHP == enemyHP)
		{
			outcome = L"引き分け";
		}
		else if (playerHP > enemyHP)
		{
			outcome = L"勝利！";
		}
		else
		{
			outcome = L"敗北";
		}

		Result::SetOutcome(outcome);

		m_Finished = true;

		m_Player->SetCanStartNode(false);

		m_Enemy->SetCanStartNode(false);

		Manager::LoadingSetScene<Result>();
	}
}

// 画面に描く処理
void Game::Draw()
{
	if (m_StageNumber != 0)
	{
		Renderer::ClearBackground(0.025f, 0.035f, 0.055f);
	}

	GetGameObject<Camera>()->Update();

	Scene::Draw();

	FontData font;

	font.font = L"メイリオ";

	font.fontSize = 22;

	font.Color = D2D1::ColorF(D2D1::ColorF::White);

	font.shadowColor = D2D1::ColorF(D2D1::ColorF::Black);

	font.shadowOffset = D2D1::Point2F(2, 2);

	Moji::Instance().SetFont(font);

	std::wstring stageDescription;

	if (m_StageNumber == 0)
	{
		stageDescription = L"自由対戦  HPを0にすれば勝利。60秒経過時は残りHPで判定";
	}
	else
	{
		stageDescription = GetStageName() + L"  60秒以内に敵を倒せばクリア";
	}

	DrawStatusText(stageDescription, 20);

	if (m_StageNumber != 0)
	{
		DrawStatusText(GetStageHint(), 585);

		auto *node = m_Enemy->GetNodeRuntime()->GetCurrentNode();

		std::wstring state;

		if (m_Countdown > 0)
		{
			state = L"敵：開始を待っています";
		}
		else
		{
			state = L"敵：接近中";
		}

		if (node && node->GetType() == NodeType::Action_Wait)
		{
			// 攻撃へ直結する待機だけを準備とする。休憩時間の長さには依存しない
			const bool ready = IsAttackPreparation(node);

			if (ready)
			{
				state = L"敵：攻撃の準備中！";
			}
			else
			{
				state = L"敵：休み中  攻撃のチャンス！";
			}

			D2D1::ColorF::Enum enemyStateColor;

			if (ready)
			{
				enemyStateColor = D2D1::ColorF::Yellow;
			}
			else
			{
				enemyStateColor = D2D1::ColorF::White;
			}

			font.Color = D2D1::ColorF(enemyStateColor);

			Moji::Instance().SetFont(font);
		}
		else if (node && node->GetCategory() == NodeCategory::AttackMelee)
		{
			state = L"敵：" + node->GetName() + L"！";
		}

		if (node && node->GetType() == NodeType::Attack_Gun)
		{
			state = L"敵：射撃の構え！盾を向けるか横へかわそう";
		}
		else if (m_Enemy->IsGuarding())
		{
			state = L"敵：盾を構えています。横や背後が狙い目！";
		}

		DrawStatusText(state, 620);

		font.Color = D2D1::ColorF(D2D1::ColorF::White);

		Moji::Instance().SetFont(font);
	}

	DrawStatusText(L"自分 HP " + std::to_wstring(m_Player->GetHP()) + L" / 相手 HP " + std::to_wstring(m_Enemy->GetHP()), 55);

	std::wostringstream timer;

	timer << std::fixed << std::setprecision(1);

	if (m_Countdown > 0)
	{
		timer << L"開始まで " << m_Countdown << L"秒";
	}
	else
	{
		timer << L"残り " << (std::max)(0.0f, 60.0f - m_Elapsed) << L"秒";
	}

	const wchar_t *pauseText;

	if (m_Paused)
	{
		pauseText = L"  一時停止中";
	}
	else
	{
		pauseText = L"";
	}

	DrawStatusText(timer.str() + (pauseText), 90);

	DrawStatusText(L"盾残量  自分 " + std::to_wstring(m_Player->GetGuardPercent()) + L"% / 相手 " + std::to_wstring(m_Enemy->GetGuardPercent()) + L"%（下ろすと回復）", 200);

	if (!m_Error.empty())
	{
		DrawStatusText(m_Error + L"  F8で編集画面へ", 130);
	}
	else
	{
		DrawStatusText(m_Player->GetNodeRuntime()->GetMessage(), 130);

		DrawStatusText(m_Player->GetNodeRuntime()->GetLastCondition(), 165);
	}

	DrawStatusText(GetGameObject<Camera>()->GetViewName(), 240);
}

// メニューから挑戦するステージを選ぶ
void Game::SelectStage(int number, bool openExample)
{
	const int firstStage = StageData::GetFirstStageID();
	const int lastStage = StageData::GetLastStageID();

	if (number < firstStage)
	{
		number = firstStage;
	}
	else if (number > lastStage)
	{
		number = lastStage;
	}

	m_StageNumber = StageData::FindStageDefinition(number).stageID;
	m_OpenStageExample = openExample && !IsPracticeStage();
}

// 選んだチュートリアルのメニュー番号を返す
int Game::GetTutorialNumber()
{
	const StageDefinition &stage = StageData::FindStageDefinition(m_StageNumber);

	if (stage.type == StageType::Tutorial)
	{
		return stage.menuNumber;
	}

	return 0;
}

// メニューの項目数も、ステージ一覧から取得する
const int Game::TutorialCount = StageData::CountStages(StageType::Tutorial);
const int Game::PracticeCount = StageData::CountStages(StageType::Practice);
// チュートリアルのメニュー番号から、内部番号を取得する
int Game::GetTutorialStage(int tutorialNumber)
{
	return StageData::FindStageID(StageType::Tutorial, tutorialNumber);
}

// 次のチュートリアルへ進む。最後・自由対戦・実践は自由対戦へ戻す
int Game::GetNextTutorialStage()
{
	int number = GetTutorialNumber();

	if (number > 0)
	{
		return GetTutorialStage(number + 1);
	}

	return StageData::GetFirstStageID();
}

// 選んだ実践ステージのメニュー番号を返す
int Game::GetPracticeNumber()
{
	const StageDefinition &stage = StageData::FindStageDefinition(m_StageNumber);

	if (stage.type == StageType::Practice)
	{
		return stage.menuNumber;
	}

	return 0;
}

// 実践のメニュー番号から、内部番号を取得する
int Game::GetPracticeStage(int number)
{
	return StageData::FindStageID(StageType::Practice, number);
}

// チュートリアルとは別の実践を選んでいるか確認する
bool Game::IsPracticeStage()
{
	return StageData::FindStageDefinition(m_StageNumber).type == StageType::Practice;
}

// 画面に出すステージの名前
std::wstring Game::GetStageName()
{
	return StageData::FindStageDefinition(m_StageNumber).name;
}

// ノードを組むときに参考にする説明
std::wstring Game::GetStageHint()
{
	return StageData::FindStageDefinition(m_StageNumber).hint;
}

// 実践のサンプルは空欄。自由対戦・チュートリアルは一覧のパスを返す
std::string Game::GetStageSamplePath()
{
	return StageData::FindStageDefinition(m_StageNumber).samplePath;
}

// 選んだステージごとに、敵の保存ファイルを切り替える
std::string Game::GetEnemySavePath()
{
	if (IsPracticeStage())
	{
		return "nodeSaveData/EnemyPractice" + std::to_string(GetPracticeNumber()) + ".json";
	}

	if (m_StageNumber == 0)
	{
		return "nodeSaveData/EnemyData.json";
	}

	return "nodeSaveData/EnemyStage" + std::to_string(GetTutorialNumber()) + ".json";
}

// 対戦と練習で、同じ保存データのHPと行動ノードを敵へ渡す
bool Game::LoadEnemyData(Player *enemy, std::wstring &message)
{
	NodesSaveLoad save;

	// 検証が終わるまで、敵本体のHPとノードは変更しない
	std::vector<NodeBase *> nodes;

	int hp = 100;

	if (!save.EnemyRuntimeLoad(GetEnemySavePath(), nodes, hp))
	{
		message = save.GetMessage();

		return false;
	}

	enemy->SetInitialHP(hp);

	enemy->SetRunNodes(std::move(nodes));

	return true;
}

// 対戦のエラー説明を消す
void Game::ClearError()
{
	m_Error.clear();
}

// 構え時間と休憩時間が同じでも、接続された行動から区別する
bool Game::IsAttackPreparation(const NodeBase *node)
{
	if (!node || node->GetType() != NodeType::Action_Wait)
	{
		return false;
	}

	const NodeBase *next = node->GetNext();

	return next && (next->GetCategory() == NodeCategory::AttackMelee || next->GetCategory() == NodeCategory::AttackRange);
}

// 対戦画面のボタンを更新して、クリックを確認する
bool Game::IsButtonClicked(const wchar_t *name)
{
	auto *button = GetGameObject<Button>(name);
	button->Update();
	return Mouse::LeftClick() && button->GetHitBox()->OverlapAABB();
}

// 対戦中の状態を左側に影付きで表示する
void Game::DrawStatusText(const std::wstring &value, float y)
{
	Moji::Instance().Draw(value, D2D1::Point2F(25, y), 1, D2D1_DRAW_TEXT_OPTIONS_NONE, true);
}

// 対戦中のプレイヤーを取得する
Player *Game::GetPlayer() const
{
	return m_Player;
}

// 対戦中の敵を取得する
Player *Game::GetEnemy() const
{
	return m_Enemy;
}

// CPU側で読み込んだプレイヤーのノードを取得する
const std::vector<NodeBase *> &Game::GetPreparedPlayer() const
{
	return m_PreparedPlayer;
}

// CPU側で読み込んだ敵のノードを取得する
const std::vector<NodeBase *> &Game::GetPreparedEnemy() const
{
	return m_PreparedEnemy;
}

// 画面の初期化状態を取得する
bool Game::GetInitialized() const
{
	return m_Initialized;
}

// 対戦開始までの時間を設定する
void Game::SetCountdown(float value)
{
	m_Countdown = value;
}

// 対戦の経過時間を取得する
float Game::GetElapsed() const
{
	return m_Elapsed;
}

// 対戦の経過時間を設定する
void Game::SetElapsed(float value)
{
	m_Elapsed = value;
}

// 対戦が終了したかどうかを取得する
bool Game::GetFinished() const
{
	return m_Finished;
}

// エラーの説明を取得する
const std::wstring &Game::GetError() const
{
	return m_Error;
}
