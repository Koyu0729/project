// 共通定義とノードエディター
#include "main.h"
#include "nodeEditor.h"

// 画面・プレイヤー・フィールド
#include "game.h"
#include "gameMenu.h"
#include "manager.h"
#include "polyTex.h"
#include "camera.h"
#include "field.h"
#include "button.h"
#include "player.h"
#include "stageObstacle.h"

// ノードの生成・保存・実行
#include "nodeCreateList.h"
#include "nodesSaveLoad.h"
#include "nodeGraphData.h"
#include "nodeRuntime.h"
#include "nodeCreate.h"

// キーボードとマウスの入力
#include "input.h"
#include "mouseInput.h"

// 文字とエフェクトの表示
#include "moji.h"
#include "bullet.h"
#include "explosion.h"

// 数値計算と表示用の文字列
#include <algorithm>
#include <cmath>
#include <sstream>
#include <iomanip>

// 保存・読み込み用のデータを解放する
NodeEditor::~NodeEditor()
{
	delete m_SaveLoad;
}

// CPU読み込み：保存・サンプルのJSONを先に解析する。ノードの表示やボタンはまだ生成しない
bool NodeEditor::InitCPU()
{
	if (!Scene::InitCPU())
	{
		return false;
	}
	// 事前解析したJSONを、編集画面の初期化まで保持する
	m_SaveLoad = new NodesSaveLoad();
	if (!Game::IsPracticeStage())
	{
		m_SaveLoad->PrepareCPU("nodeSaveData/PlayerData.json");
		m_SaveLoad->PrepareCPU("nodeSaveData/Tutorial3.json");
	}

	if (!Game::GetStageSamplePath().empty())
	{
		m_SaveLoad->PrepareCPU(Game::GetStageSamplePath());
	}

	return true;
}

bool NodeEditor::InitGPU()
{
	if (!Scene::InitGPU())
	{
		return false;
	}
	// 描画データを準備し、編集画面を初期化する
	return Init();
}

// 初期化処理を行う
bool NodeEditor::Init()
{
	if (m_Initialized)
	{
		return true;
	}

	m_Initialized = true;

	AddGameObject3D<Camera>(0);

	m_Background = AddGameObject2D<PolyTex>(0);

	m_Background->SetTexRule(Vector2(0, 0), 0, Vector2(SCREEN_WIDTH, SCREEN_HEIGHT), "asset\\texture\\wallpaper\\editor.jpg");

	m_RoutePanel = AddGameObject2D<PolyTex>(0);

	m_RoutePanel->SetTexRule(Vector2(0, 0), 0, Vector2(SCREEN_WIDTH, SCREEN_HEIGHT), "asset\\texture\\wallpaper\\editor.jpg");

	m_RoutePanel->SetColor(XMFLOAT4(1, 1, 1, 0.8f));
	m_RoutePanel->SetVisible(false);

	// テスト・実行方法・カメラを選ぶボタンの名前
	const wchar_t *captions[] = {L"テスト実行", L"編集に戻る", L"一時停止 / 再開", L"1ノード進む", L"ルートを隠す", L"最初から実行", L"実行：1回", L"カメラ1：追従", L"カメラ2：横", L"カメラ3：俯瞰"};

	for (int i = 0; i < 10; ++i)
	{
		m_Controls[i] = AddGameObject2D<Button>(9);

		// テスト実行と連続実行のボタンを、それぞれの位置に配置する
		float controlPositionX;

		if (i == 6)
		{
			controlPositionX = 980.0f;
		}
		else if (i == 0)
		{
			controlPositionX = 1040.0f;
		}
		else
		{
			controlPositionX = 20.0f + (i - 1) * 240.0f;
		}

		float controlPositionY;

		if (i == 6)
		{
			controlPositionY = 54.0f;
		}
		else
		{
			controlPositionY = 104.0f;
		}

		m_Controls[i]->SetTexRule(Vector2(controlPositionX,
										  controlPositionY),
								  0, Vector2(220, 42), "asset\\texture\\wallpaper\\editor.jpg");

		if (i >= 7)
		{
			m_Controls[i]->SetTexRule(Vector2(20.0f + (i - 7) * 180.0f, 154), 0, Vector2(160, 36), "asset\\texture\\wallpaper\\editor.jpg");
		}

		m_Controls[i]->SetName(captions[i]);

		int captionSize;

		if (i >= 7)
		{
			captionSize = 16;
		}
		else
		{
			captionSize = 18;
		}

		m_Controls[i]->SetCaptionSize(captionSize);
	}

	// ステージを選ぶボタンと、保存して挑戦するボタン
	const wchar_t *stageCaptions[] = {L"ステージ選択", L"サンプルを開く", L"保存して挑戦"};

	for (int i = 0; i < 3; ++i)
	{
		auto *button = AddGameObject2D<Button>(9);

		button->SetName(stageCaptions[i]);

		button->SetCaptionSize(18);

		button->SetTexRule(Vector2(330 + i * 210.0f, 104), 0, Vector2(195, 42), "asset\\texture\\wallpaper\\editor.jpg");
	}

	// キーボードを覚えていなくても、セーブとロードを開けるようにする
	for (int i = 0; i < 2; ++i)
	{
		auto *button = AddGameObject2D<Button>(9);

		const wchar_t *saveButtonName;

		if (i == 0)
		{
			saveButtonName = L"セーブ";
		}
		else
		{
			saveButtonName = L"ロード";
		}

		button->SetName(saveButtonName);

		button->SetCaptionSize(18);

		button->SetTexRule(Vector2(20 + i * 155.0f, 104), 0, Vector2(145, 42), "asset\\texture\\wallpaper\\editor.jpg");
	}

	m_TestPlayer = AddGameObject3D<Player>(3);

	// 上部には短い案内だけを残し、キー一覧はヘルプで開く
	m_HelpPanel = AddGameObject2D<PolyTex>(9);

	m_HelpPanel->SetTexRule(Vector2(340, 190), 0, Vector2(600, 470), "asset\\texture\\kuro.png");

	m_HelpPanel->SetVisible(false);

	m_HelpButton = AddGameObject2D<Button>(9);

	m_HelpButton->SetTexRule(Vector2(800, 12), 0, Vector2(150, 34), "asset\\texture\\wallpaper\\editor.jpg");

	m_HelpButton->SetName(L"操作ヘルプ");

	m_HelpButton->SetCaptionSize(17);

	const wchar_t *sampleNames[] = {
		L"1  接近と槍\n近づいて敵を向き、突く", L"2  条件と攻撃\n距離で行動を変える", L"3  HPと逃走\nHPが減ったら離れる",
		L"4  射撃と回避\n敵を向いて撃つ", L"5  自分が回る\n回転した後に剣で斬る", L"6  敵の周りを回る\n周囲を移動した後に突く",
		L"7  槍で突く\n中距離から攻撃", L"8  剣で斬る\n近距離で素早く攻撃", L"9  作戦の組み合わせ\n条件・盾・射撃を使う"};

	for (int i = 0; i < 9; ++i)
	{
		m_SampleButtons[i] = AddGameObject2D<Button>(9);

		m_SampleButtons[i]->SetTexRule(Vector2(365 + (i % 2) * 280.0f, 255 + (i / 2) * 72.0f), 0, Vector2(255, 60), "asset\\texture\\wallpaper\\editor.jpg");

		m_SampleButtons[i]->SetName(sampleNames[i]);

		m_SampleButtons[i]->SetCaptionSize(16);

		m_SampleButtons[i]->SetVisible(false);
	}

	m_TestPlayer->SetName(L"player");

	m_TestPlayer->SetTeam(Team::TeamA);

	m_TestTarget = AddGameObject3D<Player>(3);

	m_TestTarget->SetName(L"testTarget");

	m_TestTarget->SetPosition(Vector3(0, 1.5f, 4));

	m_TestTarget->SetRotation(Vector3(0, XM_PI, 0));

	m_TestPlayer->SetTargetObject(m_TestTarget);

	AddGameObject3D<StageObstacle>(2)->SetVisible(false);

	// 練習でも対戦と同じ床・四方の壁を使う
	AddGameObject3D<Field>(2)->SetVisible(false);

	for (int side = 0; side < 4; ++side)
	{
		auto *wall = AddGameObject3D<StageObstacle>(2);

		wall->SetArenaSide(side);
		wall->SetVisible(false);
	}

	m_CreateList = AddGameObject2D<NodeCreateList>(9);

	m_CreateList->SetVisible(false);

	// 初回の描画前に、一覧の文字と背景も隠しておく
	m_CreateList->Update();

	m_NodeManager = new NodeManager();

	if (!m_SaveLoad)
	{
		m_SaveLoad = new NodesSaveLoad();
	}

	InitSaveSlots();

	// 実践へ入ったときは、以前の保存やチュートリアルを自動で開かない
	bool valid = false;

	if (!Game::IsPracticeStage())
	{
		auto saved = m_SaveLoad->NodeRuntimeLoad("nodeSaveData/PlayerData.json");

		std::wstring validation;

		valid = NodeRuntime::Validate(saved, validation);

		NodeGraphData::Delete(saved);
	}

	std::string graphPath;

	if (Game::m_OpenStageExample)
	{
		graphPath = Game::GetStageSamplePath();
	}
	else if (valid)
	{
		graphPath = "nodeSaveData/PlayerData.json";
	}
	else
	{
		graphPath = "nodeSaveData/Tutorial3.json";
	}

	if (Game::IsPracticeStage())
	{
		// 実践では攻略サンプルを出さず、スタートから作れるようにする
		auto *start = NodeCreate::CreateLogic(NodeCategory::Start, NodeType::Start, m_NodeManager->CreateNodeID());

		m_NodeManager->RegisterNode(start);

		SetViews(NodeCreate::CreateView(start, Vector2(60, 300)));
	}
	else
	{
		LoadGraph(graphPath);
	}

	m_SaveLoad->ClearPrepared();

	Game::m_OpenStageExample = false;

	if (!valid && !Game::IsPracticeStage())
	{
		m_Message = L"条件と行動のサンプルを開きました。Lで既存の保存を編集できます";
	}

	if (Game::IsPracticeStage())
	{
		m_Message = L"実践はサンプルなし。自分のノードを組むか、Lで保存したノードを読み込んで挑戦してください";
	}

	RefreshScreen();

	ShowCursor(TRUE);

	return true;
}

// 終了処理を行う
void NodeEditor::Uninit()
{
	if (!m_Initialized)
	{
		return;
	}

	m_Initialized = false;

	DestroyWindow(m_SaveNameInput);

	m_SaveNameInput = nullptr;

	StopTest();

	ClearViews();

	Scene::Uninit();

	delete m_NodeManager;

	m_NodeManager = nullptr;

	delete m_SaveLoad;

	m_SaveLoad = nullptr;
}

// 毎フレームの処理
void NodeEditor::Update()
{
	// 枠を選んでいる間は、背後のノード・ショートカットを操作しない
	if (m_SlotOpen)
	{
		Scene::Update();

		UpdateSaveSlots();

		return;
	}

	// モーダル中はScene::Updateも呼ばず、裏のドラッグやノード生成を進めない
	if (m_ShowHelp || m_ShowSamples)
	{
		UpdateOverlay();

		return;
	}

	HandleTestInput();

	Scene::Update();

	if (!m_TestScreen && m_HelpButton->GetHitBox()->Clicked())
	{
		SetOverlay(true, false);

		return;
	}

	for (int i = 0; i < 10; ++i)
	{
		if (m_Controls[i]->GetVisible() && m_Controls[i]->GetHitBox()->Clicked())
		{
			HandleControl(i);

			return;
		}
	}

	if (m_TestScreen)
	{
		// ルート表示中は表示位置と倍率だけを変え、ノード編集には進まない
		if (m_ShowRoute)
		{
			MoveAllNodes();
		}

		UpdateTestResult();

		return;
	}

	NodeSelect();

	NodeDelete();

	NodeConnect();

	NodeCreateInput();

	EditSettings();

	MoveAllNodes();

	UpdateEditorShortcuts();
}

// エディターと実行ルート、状態を描く
void NodeEditor::Draw()
{
	Renderer::ClearBackground(0.025f, 0.035f, 0.055f);

	UpdateConnections();

	// 一覧を開いているときは、一覧の下で文字が重ならないようにする
	bool paletteOpen = !m_TestScreen && m_CreateList->GetVisible();

	std::vector<NodeView *> covered;

	if (paletteOpen)
	{
		for (auto *view : m_NodeViews)
		{
			const Vector2 nodePosition = view->GetRect()->GetPosition();

			const Vector2 nodeSize = view->GetRect()->GetScale();

			const Vector2 listPosition = m_CreateList->GetPosition();

			const Vector2 listSize = m_CreateList->GetScale();

			if (view->GetVisible() && nodePosition.x < listPosition.x + listSize.x && nodePosition.x + nodeSize.x > listPosition.x && nodePosition.y < listPosition.y + listSize.y && nodePosition.y + nodeSize.y > listPosition.y)
			{
				view->SetVisible(false);

				covered.push_back(view);
			}
		}
	}

	if (!m_ShowHelp && !m_ShowSamples)
	{
		GetGameObject<Camera>()->Update();
	}

	m_HelpPanel->SetVisible((m_ShowHelp || m_ShowSamples) && !m_TestScreen && !m_SlotOpen);

	m_HelpButton->SetVisible(!m_TestScreen && !m_SlotOpen);

	if (m_ShowHelp || m_ShowSamples)
	{
		m_HelpButton->SetName(L"閉じる（Esc）");
	}
	else
	{
		m_HelpButton->SetName(L"操作ヘルプ");
	}

	// 練習画面にノード図を重ねる。表示位置と倍率だけを変更できる
	if (m_TestScreen)
	{
		auto *current = m_TestPlayer->GetNodeRuntime()->GetCurrentNode();
		for (auto *view : m_NodeViews)
		{
			view->UpdateView(m_Zoom, m_Offset, false);
			view->SetHighlighted(current && current->GetID() == view->GetNode()->GetID());
			view->Update();
		}
	}
	Scene::Draw();

	for (auto *view : covered)
	{
		view->SetVisible(true);
	}

	FontData font;

	font.font = L"メイリオ";

	font.fontSize = 18;

	font.Color = D2D1::ColorF(D2D1::ColorF::White);

	font.shadowColor = D2D1::ColorF(D2D1::ColorF::Black);

	font.shadowOffset = D2D1::Point2F(2, 2);

	Moji::Instance().SetFont(font);

	if (DrawSaveSlots())
	{
		return;
	}

	// 見出し・操作・状態を、文字サイズと色でも区別する

	if (paletteOpen)
	{
		DrawStyledText(font, Game::GetStageName(), 20, 12, 24, D2D1::ColorF(0.4f, 0.85f, 1.0f));

		DrawStyledText(font, L"追加するノードを選択  /  ホイールで一覧をスクロール", 20, 50, 16, D2D1::ColorF(0.72f, 0.76f, 0.82f));
	}
	else if (!m_TestScreen)
	{
		DrawStyledText(font, Game::GetStageName(), 20, 12, 24, D2D1::ColorF(0.4f, 0.85f, 1.0f));

		DrawStyledText(font, L"右クリックで追加  /  丸から丸へドラッグして接続", 20, 50, 16, D2D1::ColorF(0.72f, 0.76f, 0.82f));
	}
	else
	{
		DrawStatusText(L"テスト実行画面   F5：再実行  F6：一時停止・再開  F7：次へ  F8：編集へ戻る", 12);

		DrawStatusText(GetMessage() + L"   " + GetGameObject<Camera>()->GetViewName() + L"（F1 / F2 / F3）", 40);

		const wchar_t *executionText1;

		if (m_TestLoop)
		{
			executionText1 = L"連続実行";
		}
		else
		{
			executionText1 = L"1回実行";
		}

		DrawStatusText(L"H：HP 30% / 100%   V：敵あり / なし   " + std::wstring(executionText1) + L" / " + std::to_wstring(m_TestPlayer->GetNodeRuntime()->GetPass()) + L"周目", 68);
	}

	if (!m_TestScreen && !paletteOpen)
	{
		DrawStyledText(font, L"状態：" + GetMessage(), 20, 160, 16, D2D1::ColorF(0.65f, 0.9f, 0.65f));
	}

	if (m_TestScreen)
	{
		auto *r = m_TestPlayer->GetNodeRuntime();

		DrawStatusText(L"盾残量：" + std::to_wstring(m_TestPlayer->GetGuardPercent()) + L"%（構え続けると尽きる／下ろすと回復）", 590);

		DrawStatusText(r->GetLastCondition(), 620);
	}
	else if (m_SelectedNode && !paletteOpen && !m_ShowHelp && !m_ShowSamples)
	{
		DrawStyledText(font, L"選択：" + m_SelectedNode->GetNode()->GetDisplayName() + L"   ↑↓ / ←→で設定を変更", 20, 194, 16, D2D1::ColorF(1.0f, 0.82f, 0.4f));
	}
	else if (!m_TestScreen && !paletteOpen && !m_ShowHelp && !m_ShowSamples && Game::m_StageNumber != 0)
	{
		DrawStyledText(font, Game::GetStageHint(), 20, 194, 16, D2D1::ColorF(0.72f, 0.76f, 0.82f));
	}

	if (m_PreviewVisible)
	{
		std::wostringstream value;

		value << std::fixed << std::setprecision(2);

		Vector3 position = m_TestPlayer->GetPosition();

		value << L"テスト結果  HP " << m_TestPlayer->GetHP() << L"  位置 X:" << position.x << L" Z:" << position.z;

		value << L"  向き:" << m_TestPlayer->GetRotation().y * 180 / XM_PI << L"度  敵HP:" << m_TestTarget->GetHP();

		DrawStatusText(value.str(), 665);
	}

	// 詳しい操作は背景付きのヘルプにまとめ、普段の編集画面から外す
	if (m_ShowSamples && !m_TestScreen)
	{
		DrawStyledText(font, L"サンプルを選ぶ", 370, 210, 24, D2D1::ColorF(0.4f, 0.85f, 1.0f));

		DrawStyledText(font, L"選ぶと編集中のノードを入れ替えます。", 370, 616, 16, D2D1::ColorF(1.0f, 0.82f, 0.4f));
	}

	if (m_ShowHelp && !m_TestScreen)
	{
		DrawStyledText(font, L"操作ヘルプ", 370, 210, 24, D2D1::ColorF(0.4f, 0.85f, 1.0f));

		DrawStyledText(font, L"ノードの編集", 370, 255, 18, D2D1::ColorF(1.0f, 0.82f, 0.4f));

		DrawStyledText(font, L"右クリック：追加  /  出力の丸 → 入力の丸：接続", 370, 286, 16, D2D1::ColorF(D2D1::ColorF::White));

		DrawStyledText(font, L"Delete：削除  /  ↑↓・←→：選択ノードの設定", 370, 313, 16, D2D1::ColorF(D2D1::ColorF::White));

		DrawStyledText(font, L"ホイール：拡大・縮小  /  中ボタンドラッグ：移動", 370, 340, 16, D2D1::ColorF(D2D1::ColorF::White));

		DrawStyledText(font, L"テストと保存", 370, 384, 18, D2D1::ColorF(1.0f, 0.82f, 0.4f));

		DrawStyledText(font, L"F5：再実行  /  F6：一時停止  /  F7：次へ  /  F8：編集", 370, 415, 16, D2D1::ColorF(D2D1::ColorF::White));

		DrawStyledText(font, L"P：セーブ  /  L：ロード  /  Enter：保存して挑戦", 370, 442, 16, D2D1::ColorF(D2D1::ColorF::White));

		DrawStyledText(font, L"F9：ステージ選択  /  1：メニュー", 370, 469, 16, D2D1::ColorF(D2D1::ColorF::White));

		if (!Game::IsPracticeStage())
		{
			DrawStyledText(font, L"サンプル", 370, 513, 18, D2D1::ColorF(1.0f, 0.82f, 0.4f));

			DrawStyledText(font, L"F10：サンプル一覧  /  2〜0：種類ごとの例", 370, 544, 16, D2D1::ColorF(D2D1::ColorF::White));
		}

		DrawStyledText(font, L"条件は「はい」「いいえ」の両方をつなぎます。", 370, 596, 16, D2D1::ColorF(0.72f, 0.76f, 0.82f));
	}
}

// 保存したノードと配置を読み込む
bool NodeEditor::LoadGraph(const std::string &path)
{
	StopTest();

	std::vector<NodeBase *> nodes;
	std::vector<NodePlacement> placements;
	bool loaded = m_SaveLoad->NodeEditorLoad(path, nodes, placements);

	if (loaded)
	{
		ClearViews();
		m_NodeManager->ClearLogic();

		for (size_t index = 0; index < nodes.size(); ++index)
		{
			m_NodeManager->RegisterNode(nodes[index]);
			SetViews(NodeCreate::CreateView(nodes[index], placements[index].position));
		}

		const Port ports[] = {Port::OutDef, Port::OutTrue, Port::OutFalse};
		for (auto *node : nodes)
		{
			for (Port port : ports)
			{
				NodeBase *next = node->GetNextForPort(port);
				if (next)
				{
					m_NodeManager->RegisterConnect(node, next, port);
				}
			}
		}

		m_Zoom = 1;

		m_Offset = Vector2(0, 0);

		float right = 0, bottom = 0;

		for (auto *view : m_NodeViews)
		{
			right = (std::max)(right, view->GetPosition().x + 240);

			bottom = (std::max)(bottom, view->GetPosition().y + 120);
		}

		m_Zoom = (std::min)(1.0f, (std::min)(1200.0f / (std::max)(right, 1.0f), 470.0f / (std::max)(bottom - 150.0f, 1.0f)));

		m_Offset = Vector2(25, 160.0f - 150.0f * m_Zoom);

		for (auto *view : m_NodeViews)
		{
			view->UpdateView(m_Zoom, m_Offset, false);
		}
	}

	SetMessage(m_SaveLoad->GetMessage());

	UpdateConnections();

	return loaded;
}

// ノードの見た目と線を解放する
void NodeEditor::ClearViews()
{
	if (m_DragLine)
	{
		m_DragLine->SetDestroy();

		m_DragLine = nullptr;
	}

	for (auto *view : m_NodeViews)
	{
		view->Uninit();

		view->SetVisible(false);

		view->SetDestroy();
	}

	m_NodeViews.clear();

	for (auto *line : m_ConnectedLines)
	{
		line->SetDestroy();
	}

	m_ConnectedLines.clear();

	m_SelectedNode = nullptr;

	m_StartNode = nullptr;

	m_StartPort = Port::PortNull;

	m_Connecting = false;
}

// 一覧で選んだ種類のノードを置く
void NodeEditor::NodeCreateInput()
{
	if (Mouse::RightClick())
	{
		m_CreateList->SetVisible(true);
	}

	if (!m_CreateList->GetHitBox()->OverlapAABB() && Mouse::LeftClick())
	{
		m_CreateList->SetVisible(false);
	}

	if (!m_CreateList->GetSelect())
	{
		return;
	}

	auto category = m_CreateList->GetSelectedCategory();

	bool hasStart = false;

	for (auto *node : m_NodeManager->GetNodes())
	{
		if (node->GetCategory() == NodeCategory::Start)
		{
			hasStart = true;
		}
	}

	if (category == NodeCategory::Start && hasStart)
	{
		SetMessage(L"スタートは1個だけ置けます");
	}
	else
	{
		auto *logic = NodeCreate::CreateLogic(category, m_CreateList->GetSelectedType(), m_NodeManager->CreateNodeID());

		if (logic)
		{
			m_NodeManager->RegisterNode(logic);

			Vector2 position = (m_CreateList->GetReleasedPos() - m_Offset) / m_Zoom;

			SetViews(NodeCreate::CreateView(logic, position));
		}
	}

	m_CreateList->SelectReset();

	m_CreateList->SetVisible(false);
}

// クリックしたノードを選ぶ
void NodeEditor::NodeSelect()
{
	if (!Mouse::LeftClick())
	{
		return;
	}

	m_SelectedNode = nullptr;

	for (auto *view : m_NodeViews)
	{
		if (view->GetRect()->GetHitBox()->Clicked())
		{
			m_SelectedNode = view;

			break;
		}
	}

	for (auto *view : m_NodeViews)
	{
		view->SetHighlighted(view == m_SelectedNode);
	}
}

// 選んだノードと、それにつながる線を消す
void NodeEditor::NodeDelete()
{
	if (!Input::GetKeyTrigger(VK_DELETE) || !m_SelectedNode)
	{
		return;
	}

	auto *view = m_SelectedNode;

	auto *node = view->GetNode();

	if (m_StartNode == view)
	{
		if (m_DragLine)
		{
			m_DragLine->SetDestroy();
		}

		m_DragLine = nullptr;

		m_StartNode = nullptr;

		m_Connecting = false;
	}

	view->Uninit();

	view->SetVisible(false);

	view->SetDestroy();

	m_NodeViews.erase(std::remove(m_NodeViews.begin(), m_NodeViews.end(), view), m_NodeViews.end());

	m_SelectedNode = nullptr;

	m_NodeManager->UnregisterConnect(node);

	UpdateConnections();
}

// 出力と入力をドラッグでつなぐ
void NodeEditor::NodeConnect()
{
	if (!m_Connecting && Mouse::LeftClick())
	{
		for (auto *view : m_NodeViews)
		{
			for (Port port : {Port::OutDef, Port::OutTrue, Port::OutFalse, Port::In})
			{
				if (view->InsidePort(port))
				{
					m_StartNode = view;

					m_StartPort = port;

					m_Connecting = true;

					m_DragLine = AddGameObject2D<DrawLine>(8);

					m_DragLine->SetColor(XMFLOAT4(1, 1, 1, 1));

					break;
				}
			}

			if (m_Connecting)
			{
				break;
			}
		}
	}

	if (!m_Connecting)
	{
		return;
	}

	m_DragLine->SetPoints(m_StartNode->GetPortCenter(m_StartPort), Mouse::Pos());

	if (!Mouse::LeftRelease())
	{
		return;
	}

	bool connected = false;

	for (auto *view : m_NodeViews)
	{
		if (view == m_StartNode)
		{
			continue;
		}

		for (Port port : {Port::In, Port::OutDef, Port::OutTrue, Port::OutFalse})
		{
			if (!view->InsidePort(port))
			{
				continue;
			}

			if (m_StartPort != Port::In && port == Port::In)
			{
				connected = m_NodeManager->RegisterConnect(m_StartNode->GetNode(), view->GetNode(), m_StartPort);
			}
			else if (m_StartPort == Port::In && port != Port::In)
			{
				connected = m_NodeManager->RegisterConnect(view->GetNode(), m_StartNode->GetNode(), port);
			}

			if (connected)
			{
				SetMessage(L"接続しました");
			}
			else
			{
				SetMessage(m_NodeManager->GetMessage());
			}

			break;
		}

		if (connected)
		{
			break;
		}
	}

	m_DragLine->SetDestroy();

	m_DragLine = nullptr;

	m_Connecting = false;

	m_StartNode = nullptr;

	m_StartPort = Port::PortNull;

	UpdateConnections();
}

// 選んだノードの時間・距離・角度などを変える
void NodeEditor::EditSettings()
{
	if (!m_SelectedNode)
	{
		return;
	}

	auto *node = m_SelectedNode->GetNode();

	NodeSettings s = node->GetSettings();

	int vertical;

	if (Input::GetKeyTrigger(VK_UP))
	{
		vertical = 1;
	}
	else if (Input::GetKeyTrigger(VK_DOWN))
	{
		vertical = -1;
	}
	else
	{
		vertical = 0;
	}

	int horizontal;

	if (Input::GetKeyTrigger(VK_RIGHT))
	{
		horizontal = 1;
	}
	else if (Input::GetKeyTrigger(VK_LEFT))
	{
		horizontal = -1;
	}
	else
	{
		horizontal = 0;
	}

	if (node->GetType() == NodeType::If_Stats || node->GetType() == NodeType::If_TargetHP || node->GetType() == NodeType::If_Random)
	{
		s.hpPercent = (std::max)(0.0f, (std::min)(100.0f, s.hpPercent + vertical * 5));
	}
	else if (node->GetType() == NodeType::Rotate_SpinLeft || node->GetType() == NodeType::Rotate_SpinRight)
	{
		s.duration = (std::max)(0.1f, (std::min)(10.0f, s.duration + vertical * 0.1f));

		s.angleDegrees = (std::max)(15.0f, (std::min)(360.0f, s.angleDegrees + horizontal * 15));

		horizontal = 0;
	}
	else if ((node->GetCategory() == NodeCategory::Rotate && node->GetType() != NodeType::Rotate_ToTarget) || node->GetType() == NodeType::If_TargetAhead)
	{
		s.angleDegrees = (std::max)(1.0f, (std::min)(360.0f, s.angleDegrees + vertical * 15));
	}
	else if (node->GetCategory() != NodeCategory::Start && node->GetCategory() != NodeCategory::If)
	{
		s.duration = (std::max)(0.1f, (std::min)(10.0f, s.duration + vertical * 0.1f));
	}

	s.distance = (std::max)(0.1f, (std::min)(100.0f, s.distance + horizontal * 0.1f));

	node->SetSettings(s);
}

// ホイールで拡大縮小し、中ボタンで画面を動かす
void NodeEditor::MoveAllNodes()
{
	if (m_CreateList->GetVisible() && m_CreateList->GetHitBox()->OverlapAABB())
	{
		return;
	}

	// 入力側で30倍になっているホイールを戻してから拡大縮小する
	float wheel = Mouse::Wheel() / 30.0f;

	if (fabsf(wheel) > 0.01f)
	{
		float previous = m_Zoom;

		m_Zoom = (std::max)(m_ZoomMin, (std::min)(m_ZoomMax, m_Zoom * std::pow(1.025f, (std::max)(-4.0f, (std::min)(4.0f, wheel)))));

		Vector2 mouse = Mouse::Pos();

		m_Offset += (mouse - m_Offset) * (1.0f - m_Zoom / previous);
	}

	if (Mouse::MiddleClick())
	{
		m_PrevMouse = Mouse::Pos();
	}

	if (Mouse::MiddleHold())
	{
		Vector2 now = Mouse::Pos();

		m_Offset += now - m_PrevMouse;

		m_PrevMouse = now;
	}

	for (auto *view : m_NodeViews)
	{
		view->UpdateView(m_Zoom, m_Offset, !m_TestScreen && !m_Connecting);
	}
}

// ノードの場所に合わせて接続の線を更新する
void NodeEditor::UpdateConnections()
{
	const auto &connections = m_NodeManager->GetSaveConnections();

	while (m_ConnectedLines.size() < connections.size())
	{
		m_ConnectedLines.push_back(AddGameObject2D<DrawLine>(8));
	}

	while (m_ConnectedLines.size() > connections.size())
	{
		m_ConnectedLines.back()->SetDestroy();

		m_ConnectedLines.pop_back();
	}

	for (size_t i = 0; i < connections.size(); ++i)
	{
		const auto &c = connections[i];

		NodeView *from = nullptr;

		NodeView *to = nullptr;

		for (auto *v : m_NodeViews)
		{
			if (v->GetNode()->GetID() == c.fromID)
			{
				from = v;
			}

			if (v->GetNode()->GetID() == c.toID)
			{
				to = v;
			}
		}

		if (!from || !to)
		{
			continue;
		}

		m_ConnectedLines[i]->SetPoints(from->GetPortCenter(c.fromPort), to->GetPortCenter(Port::In));

		// 今の周で実際に通った接続だけを黄色にする。条件の未選択側は元の色を残す
		bool active = false;
		if (m_TestScreen)
		{
			for (const auto &step : m_TestPlayer->GetNodeRuntime()->GetRoute())
			{
				if (step.fromID == c.fromID && step.port == c.fromPort && step.toID == c.toID)
				{
					active = true;
					break;
				}
			}
		}

		XMFLOAT4 color;

		if (active)
		{
			color = XMFLOAT4(1, 0.9f, 0.1f, 1);
		}
		else if (c.fromPort == Port::OutTrue)
		{
			color = XMFLOAT4(0.3f, 1, 0.6f, 1);
		}
		else if (c.fromPort == Port::OutFalse)
		{
			color = XMFLOAT4(1, 0.5f, 0.4f, 1);
		}
		else
		{
			color = XMFLOAT4(0.8f, 0.85f, 1, 1);
		}

		m_ConnectedLines[i]->SetColor(color);
	}
}

// 編集中のノードの接続を確認し、結果を説明欄へ渡す
bool NodeEditor::ValidateGraph()
{
	return NodeRuntime::Validate(m_NodeManager->GetNodes(), m_Message);
}

// 保存枠のボタンと、日本語入力欄を用意する
void NodeEditor::InitSaveSlots()
{
	// 保存枠の画面は、ノードやステージのボタンより手前に置く
	m_SlotPanel = AddGameObject2D<PolyTex>(9);

	m_SlotPanel->SetTexRule(Vector2(0, 0), 0, Vector2(SCREEN_WIDTH, SCREEN_HEIGHT), "asset\\texture\\wallpaper\\editor.jpg");

	m_SlotPanel->SetVisible(false);

	for (int i = 0; i < 7; ++i)
	{
		m_SlotButtons[i] = AddGameObject2D<Button>(9);

		m_SlotButtons[i]->SetCaptionSize(16);

		float slotPositionY;

		if (i < 5)
		{
			slotPositionY = 200 + i * 75.0f;
		}
		else if (i == 5)
		{
			slotPositionY = 600.0f;
		}
		else
		{
			slotPositionY = 390.0f;
		}

		m_SlotButtons[i]->SetTexRule(Vector2(350, slotPositionY),
									 0, Vector2(580, 60), "asset\\texture\\wallpaper\\editor.jpg");

		const wchar_t *slotButtonName;

		if (i == 5)
		{
			slotButtonName = L"キャンセル";
		}
		else if (i == 6)
		{
			slotButtonName = L"この枠へ上書きする";
		}
		else
		{
			slotButtonName = L"";
		}

		m_SlotButtons[i]->SetName(slotButtonName);

		m_SlotButtons[i]->SetVisible(false);
	}

	// 保存名はWindowsの入力欄を使い、日本語IMEで入力できるようにする
	m_SaveNameInput = CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", L"", WS_CHILD | ES_AUTOHSCROLL,
									  350, 345, 580, 32, GetGameWindow(), nullptr, GetModuleHandleW(nullptr), nullptr);

	SendMessageW(m_SaveNameInput, EM_SETLIMITTEXT, 24, 0);
	SendMessageW(m_SaveNameInput, WM_SETFONT, reinterpret_cast<WPARAM>(GetStockObject(DEFAULT_GUI_FONT)), TRUE);

	for (int i = 0; i < 10; ++i)
	{
		m_SlotManageButtons[i] = AddGameObject2D<Button>(9);
		m_SlotManageButtons[i]->SetCaptionSize(18);
		int managePositionX1;

		if (i % 2 == 0)
		{
			managePositionX1 = 940;
		}
		else
		{
			managePositionX1 = 1050;
		}

		m_SlotManageButtons[i]->SetTexRule(Vector2(managePositionX1, 200 + (i / 2) * 75.0f), 0, Vector2(100, 60), "asset\\texture\\wallpaper\\editor.jpg");
		const wchar_t *managePositionX2;

		if (i % 2 == 0)
		{
			managePositionX2 = L"名前変更";
		}
		else
		{
			managePositionX2 = L"削除";
		}

		m_SlotManageButtons[i]->SetName(managePositionX2);
		m_SlotManageButtons[i]->SetVisible(false);
	}
}

// P・L・保存して挑戦から、共通の5枠の選択画面を開く
void NodeEditor::OpenSaveSlots(bool saving, bool challenge)
{
	m_CreateList->SetVisible(false);

	m_CreateList->Update();

	m_SlotOpen = true;

	m_SlotSaving = saving;

	m_SlotChallenge = challenge;

	m_OverwriteSlot = 0;

	m_SlotOperation = 0;

	SetWindowTextW(m_SaveNameInput, L"");
	SetWindowPos(m_SaveNameInput, HWND_TOP, 350, 165, 580, 30, SWP_NOACTIVATE);
	SendMessageW(m_SaveNameInput, 0x1501, FALSE, reinterpret_cast<LPARAM>(L"保存名（空欄なら今の名前を引き継ぎます）"));
	int saveInputVisibility;

	if (saving)
	{
		saveInputVisibility = SW_SHOW;
	}
	else
	{
		saveInputVisibility = SW_HIDE;
	}

	ShowWindow(m_SaveNameInput, saveInputVisibility);

	bool imported = m_SaveLoad->InitializeSaveSlots();

	bool full = true;

	for (int slot = 1; slot <= NodesSaveLoad::SaveSlotCount; ++slot)
	{
		m_SlotButtons[slot - 1]->SetName(m_SaveLoad->GetSlotCaption(slot));

		m_SlotButtons[slot - 1]->SetVisible(true);

		m_SlotManageButtons[(slot - 1) * 2]->SetVisible(m_SaveLoad->HasSlot(slot));

		m_SlotManageButtons[(slot - 1) * 2 + 1]->SetVisible(m_SaveLoad->HasSlot(slot));

		full = full && m_SaveLoad->HasSlot(slot);
	}

	m_SlotButtons[5]->SetVisible(true);

	m_SlotButtons[6]->SetVisible(false);

	m_SlotButtons[6]->SetName(L"この枠へ上書きする");

	m_SlotPanel->SetVisible(true);

	if (!imported)
	{
		SetMessage(m_SaveLoad->GetMessage());
	}
	else if (saving)
	{
		if (full)
		{
			SetMessage(L"5枠すべて保存済みです。上書きする枠を選んでください");
		}
		else
		{
			SetMessage(L"空き枠に保存できます。保存済みの枠を選ぶと上書きの確認へ進みます");
		}
	}
	else
	{
		SetMessage(L"空き枠はロードできません。保存済みの枠を選んでください");
	}

	RefreshScreen();
}

// 保存・読み込みの画面を閉じて、編集のボタンとノードを戻す
void NodeEditor::CloseSaveSlots()
{
	ShowWindow(m_SaveNameInput, SW_HIDE);

	SetFocus(GetGameWindow());

	for (auto *button : m_SlotManageButtons)
	{
		button->SetVisible(false);
	}

	m_SlotOpen = false;

	m_OverwriteSlot = 0;

	m_SlotChallenge = false;

	m_SlotPanel->SetVisible(false);

	for (auto *button : m_SlotButtons)
	{
		button->SetVisible(false);
	}

	RefreshScreen();
}

// 枠のボタン、上書きの決定、キャンセルを処理する
void NodeEditor::UpdateSaveSlots()
{
	if (Input::GetKeyTrigger(VK_ESCAPE) || m_SlotButtons[5]->GetHitBox()->Clicked())
	{
		CloseSaveSlots();

		SetMessage(L"キャンセルしました。保存データは変更していません");

		return;
	}

	if (m_OverwriteSlot > 0)
	{
		if (m_SlotButtons[6]->GetHitBox()->Clicked())
		{

			ConfirmSaveSlot();
		}

		return;
	}

	for (int slot = 1; slot <= NodesSaveLoad::SaveSlotCount; ++slot)
	{

		for (int action = 0; action < 2; ++action)
		{
			auto *button = m_SlotManageButtons[(slot - 1) * 2 + action];

			if (button->GetVisible() && button->GetHitBox()->Clicked())
			{
				ManageSaveSlot(slot, action == 1);

				return;
			}
		}

		if (m_SlotButtons[slot - 1]->GetHitBox()->Clicked())
		{
			ChooseSaveSlot(slot);

			return;
		}
	}
}

// 空き枠なら保存し、保存済みなら上書きの確認を表示する
void NodeEditor::ChooseSaveSlot(int slot)
{
	if (m_SlotSaving)
	{
		if (m_SaveLoad->HasSlot(slot))
		{
			m_OverwriteSlot = slot;

			for (auto *button : m_SlotManageButtons)
			{
				button->SetVisible(false);
			}

			for (int i = 0; i < NodesSaveLoad::SaveSlotCount; ++i)
			{
				m_SlotButtons[i]->SetVisible(false);
			}

			m_SlotButtons[6]->SetVisible(true);

			SetMessage(L"保存済みの枠です。上書きするか、キャンセルを選んでください");
		}
		else
		{
			CommitSaveSlot(slot, false);
		}

		return;
	}

	if (!m_SaveLoad->HasSlot(slot))
	{
		SetMessage(L"この枠は空きです。保存済みの枠を選んでください");

		return;
	}

	if (LoadGraph(m_SaveLoad->GetSlotPath(slot)))
	{
		CloseSaveSlots();

		SetMessage(L"枠" + std::to_wstring(slot) + L"をロードしました");

		UpdateConnections();
	}
	else
	{
		SetMessage(m_SaveLoad->GetMessage() + L"。編集中のノードは残しています");
	}
}

// 指定された枠へ保存し、挑戦の場合だけ実行用データも更新する
void NodeEditor::CommitSaveSlot(int slot, bool overwrite)
{

	wchar_t name[25]{};

	GetWindowTextW(m_SaveNameInput, name, 25);

	if (!m_SaveLoad->SaveSlot(slot, m_NodeManager, GetPlacements(), overwrite, name))
	{
		SetMessage(m_SaveLoad->GetMessage());

		return;
	}

	bool challenge = m_SlotChallenge;

	std::wstring savedMessage = m_SaveLoad->GetMessage();

	if (challenge && !m_SaveLoad->NodeEditorSave("nodeSaveData/PlayerData.json", m_NodeManager, GetPlacements()))
	{
		SetMessage(savedMessage + L"。対戦用データの更新に失敗しました");

		return;
	}

	CloseSaveSlots();

	SetMessage(savedMessage);

	if (challenge)
	{
		Manager::LoadingSetScene<Game>();
	}
}

// 名前変更と削除は対象の名前を表示してから、決定ボタンで確定する
void NodeEditor::ManageSaveSlot(int slot, bool deleting)
{
	m_OverwriteSlot = slot;

	if (deleting)
	{
		m_SlotOperation = 2;
	}
	else
	{
		m_SlotOperation = 1;
	}

	for (int i = 0; i < 5; ++i)
	{
		m_SlotButtons[i]->SetVisible(false);
	}

	for (auto *button : m_SlotManageButtons)
	{
		button->SetVisible(false);
	}

	const wchar_t *confirmButtonName;

	if (deleting)
	{
		confirmButtonName = L"この枠を削除する";
	}
	else
	{
		confirmButtonName = L"名前を変更する";
	}

	m_SlotButtons[6]->SetName(confirmButtonName);
	m_SlotButtons[6]->SetVisible(true);

	if (deleting)
	{
		SetMessage(L"削除する名前を確認してください。キャンセルするとデータは残ります");
	}
	else
	{
		SetMessage(L"新しい名前を入力してください（24文字まで・日本語も使えます）");
	}

	SetWindowPos(m_SaveNameInput, HWND_TOP, 350, 345, 580, 32, SWP_NOACTIVATE);

	SetWindowTextW(m_SaveNameInput, m_SaveLoad->GetSlotName(slot).c_str());
	int manageInputVisibility;

	if (deleting)
	{
		manageInputVisibility = SW_HIDE;
	}
	else
	{
		manageInputVisibility = SW_SHOW;
	}

	ShowWindow(m_SaveNameInput, manageInputVisibility);

	if (!deleting)
	{
		SetFocus(m_SaveNameInput);
		SendMessageW(m_SaveNameInput, EM_SETSEL, 0, -1);
	}
}

// 確認ボタンを押した操作だけを保存データへ反映する
void NodeEditor::ConfirmSaveSlot()
{
	if (m_SlotOperation != 0)
	{
		wchar_t name[25]{};

		GetWindowTextW(m_SaveNameInput, name, 25);

		bool success;

		if (m_SlotOperation == 2)
		{
			success = m_SaveLoad->DeleteSlot(m_OverwriteSlot);
		}
		else
		{
			success = m_SaveLoad->RenameSlot(m_OverwriteSlot, name);
		}

		if (success)
		{
			bool saving = m_SlotSaving;
			bool challenge = m_SlotChallenge;

			CloseSaveSlots();
			OpenSaveSlots(saving, challenge);
		}
		else
		{
			SetMessage(m_SaveLoad->GetMessage());
		}
	}
	else
	{
		CommitSaveSlot(m_OverwriteSlot, true);
	}
}

// 編集したノードをコピーして、練習を始める
void NodeEditor::StartTest(bool pause)
{
	StopTest();

	std::wstring validationMessage;
	if (!NodeRuntime::Validate(m_NodeManager->GetNodes(), validationMessage))
	{
		SetMessage(validationMessage);
		return;
	}

	m_TestPlayer->ResetForTest();

	m_TestTarget->ResetForTest();

	m_TestTarget->SetPosition(Vector3(0, 1.5f, 4));

	m_TestTarget->SetRotation(Vector3(0, XM_PI, 0));

	m_TestPlayer->SetTargetObject(m_TestTarget);

	m_TestPlayer->SetRunNodes(m_SaveLoad->CloneNodes(m_NodeManager->GetNodes()));

	m_TestPlayer->SetNodeLoop(m_TestLoop);

	m_TestPlayer->SetCanStartNode(true);

	m_TestPlayer->SetSwordDamageEnabled(Game::m_StageNumber == 0);

	m_TestTarget->SetSwordDamageEnabled(Game::m_StageNumber == 0);

	if (Game::GetTutorialNumber() >= 4 || Game::IsPracticeStage())
	{
		std::wstring enemyMessage;

		if (!Game::LoadEnemyData(m_TestTarget, enemyMessage))
		{
			StopTest();

			SetMessage(enemyMessage);

			return;
		}

		m_TestTarget->SetTargetObject(m_TestPlayer);

		m_TestTarget->SetNodeLoop(true);

		m_TestTarget->SetCanStartNode(true);
	}

	// テストを初期化し、一時停止を選んだ場合は行動開始前に止める。
	auto *runtime = m_TestPlayer->GetNodeRuntime();

	runtime->Start(m_TestLoop);

	m_TestPlayer->MarkNodeRuntimeStarted();

	if (pause)
	{
		runtime->Pause();

		if (m_TestTarget->CanStartNode())
		{
			m_TestTarget->GetNodeRuntime()->Start(true);

			m_TestTarget->MarkNodeRuntimeStarted();

			m_TestTarget->GetNodeRuntime()->Pause();
		}
	}

	m_TestScreen = true;

	m_TestActive = true;

	m_PreviewVisible = true;

	RefreshScreen();

	m_CreateList->SetVisible(false);

	if (m_TestLoop)
	{
		SetMessage(L"連続テスト中：最後まで進むとスタートへ戻ります");
	}
	else
	{
		SetMessage(L"単発テスト中：F6 一時停止 / F7 次のノード / F8 停止");
	}
}

// 練習を止めて、編集に戻る
void NodeEditor::StopTest()
{
	m_TestActive = false;

	m_TestScreen = false;

	m_PreviewVisible = false;

	RefreshScreen();

	for (auto *bullet : GetGameObjects<Bullet>())
	{
		bullet->SetDestroy();
	}

	for (auto *effect : GetGameObjects<Explosion>())
	{
		effect->SetDestroy();
	}

	if (m_TestPlayer)
	{
		m_TestPlayer->SetCanStartNode(false);

		m_TestPlayer->GetNodeRuntime()->Stop();

		m_TestTarget->SetCanStartNode(false);

		m_TestTarget->GetNodeRuntime()->Stop();

		m_TestPlayer->ClearNodeCommands();
	}

	SetMessage(L"停止しました。ノードを編集できます");
}

// 押されたボタンに合わせて、実行やカメラを切り替える
void NodeEditor::HandleControl(int index)
{
	if (index >= 7 && index <= 9)
	{
		auto *camera = GetGameObject<Camera>();

		camera->SetViewNumber(index - 6);

		camera->Update();
	}
	else if (index == 0 || index == 5)
	{
		StartTest();
	}
	else if (index == 6)
	{
		m_TestLoop = !m_TestLoop;

		m_TestPlayer->SetNodeLoop(m_TestLoop);

		m_TestPlayer->GetNodeRuntime()->SetLoop(m_TestLoop);

		if (m_TestScreen && !m_TestActive && m_TestLoop)
		{
			StartTest();
		}
		else
		{
			RefreshScreen();

			if (!m_TestScreen)
			{
				if (m_TestLoop)
				{
					SetMessage(L"連続実行を選択しました。テスト実行で開始");
				}
				else
				{
					SetMessage(L"1回実行を選択しました");
				}
			}
		}
	}
	else if (index == 1)
	{
		StopTest();
	}
	else if (index == 4)
	{
		m_ShowRoute = !m_ShowRoute;

		RefreshScreen();
	}
	else if (m_TestActive)
	{
		auto *runtime = m_TestPlayer->GetNodeRuntime();

		if (index == 2)
		{
			if (runtime->GetState() == RuntimeState::Paused)
			{
				runtime->Resume();
			}
			else
			{
				runtime->Pause();
			}
		}

		if (index == 3)
		{
			runtime->Pause();

			runtime->Step();
		}
	}
}

// テストのキー操作と、敵の一時停止の同期
void NodeEditor::HandleTestInput()
{
	if (Input::GetKeyTrigger(VK_F8))
	{
		StopTest();
	}

	if (Input::GetKeyTrigger(VK_F5))
	{
		StartTest();
	}

	if (Input::GetKeyTrigger(VK_F6) && m_TestActive)
	{
		auto *r = m_TestPlayer->GetNodeRuntime();

		if (r->GetState() == RuntimeState::Paused)
		{
			r->Resume();
		}
		else
		{
			r->Pause();
		}
	}

	if (Input::GetKeyTrigger(VK_F7))
	{
		if (!m_TestActive)
		{
			StartTest(true);
		}

		auto *r = m_TestPlayer->GetNodeRuntime();

		r->Pause();

		r->Step();
	}

	if (m_TestActive)
	{
		if (Input::GetKeyTrigger('H'))
		{
			int testHP;

			if (m_TestPlayer->GetHP() > 50)
			{
				testHP = 30;
			}
			else
			{
				testHP = 100;
			}

			m_TestPlayer->SetTestHP(testHP);
		}

		if (Input::GetKeyTrigger('V'))
		{
			Player *testTarget;

			if (m_TestPlayer->GetTargetObject())
			{
				testTarget = nullptr;
			}
			else
			{
				testTarget = m_TestTarget;
			}

			m_TestPlayer->SetTargetObject(testTarget);
		}
	}

	// 敵の実行状態も練習の一時停止・再開に合わせる
	if (m_TestActive && (Game::GetTutorialNumber() >= 4 || Game::IsPracticeStage()) && m_TestTarget->CanStartNode())
	{
		if (m_TestPlayer->GetNodeRuntime()->GetState() == RuntimeState::Paused)
		{
			m_TestTarget->GetNodeRuntime()->Pause();
		}
		else
		{
			m_TestTarget->GetNodeRuntime()->Resume();
		}
	}
}

// 実行結果・選ばれたルート・死亡時の終了を更新する
void NodeEditor::UpdateTestResult()
{
	auto *r = m_TestPlayer->GetNodeRuntime();

	SetMessage(r->GetMessage());

	for (auto *v : m_NodeViews)
	{
		v->SetHighlighted(v->GetNode()->GetID() == r->GetCurrentID());
	}

	if (r->GetState() == RuntimeState::Completed || r->GetState() == RuntimeState::Error)
	{
		m_TestActive = false;

		m_TestPlayer->SetCanStartNode(false);

		m_TestTarget->SetCanStartNode(false);

		m_TestTarget->GetNodeRuntime()->Stop();
	}

	if (m_TestPlayer->GetHP() <= 0)
	{
		m_TestActive = false;

		m_TestPlayer->SetCanStartNode(false);

		m_TestTarget->SetCanStartNode(false);

		m_TestTarget->GetNodeRuntime()->Stop();

		SetMessage(L"倒されました。F5でやり直すか、F8でノードを直そう");
	}
}

// ダイアログを開いた瞬間に、背後で継続しているドラッグと接続を解除する
void NodeEditor::SetOverlay(bool help, bool samples)
{
	m_ShowHelp = help;

	m_ShowSamples = samples && !Game::IsPracticeStage();

	if (m_ShowHelp || m_ShowSamples)
	{
		m_CreateList->SetVisible(false);

		m_CreateList->SelectReset();

		m_CreateList->Update();

		for (auto *view : m_NodeViews)
		{
			view->UpdateView(m_Zoom, m_Offset, false);
		}

		if (m_DragLine)
		{
			m_DragLine->SetDestroy();

			m_DragLine = nullptr;
		}

		m_Connecting = false;

		m_StartNode = nullptr;

		m_StartPort = Port::PortNull;
	}

	RefreshScreen();
}

// ダイアログ自身のボタンだけに入力を渡し、ノード・カメラ・ショートカットを更新しない
void NodeEditor::UpdateOverlay()
{
	m_HelpButton->Update();

	if (Input::GetKeyTrigger(VK_ESCAPE) || (Mouse::LeftClick() && m_HelpButton->GetHitBox()->OverlapAABB()))
	{
		SetOverlay(false, false);

		return;
	}

	if (!m_ShowSamples)
	{
		return;
	}

	for (int i = 0; i < 9; ++i)
	{
		auto *button = m_SampleButtons[i];

		button->Update();

		if (Mouse::LeftClick() && button->GetHitBox()->OverlapAABB())
		{
			ChooseSample(i);

			return;
		}
	}
}

// 一覧で選んだサンプルだけを読み込む。実践にはサンプルを渡さない
void NodeEditor::ChooseSample(int index)
{
	if (!m_ShowSamples || Game::IsPracticeStage() || index < 0 || index >= 9)
	{
		return;
	}

	SetOverlay(false, false);

	LoadGraph("nodeSaveData/Tutorial" + std::to_string(index + 1) + ".json");
}

// 保存画面・サンプル・ステージへ進む編集用のキー操作
void NodeEditor::UpdateEditorShortcuts()
{
	if (Input::GetKeyTrigger('P') || GetGameObject<Button>(L"セーブ")->GetHitBox()->Clicked())
	{
		OpenSaveSlots(true);

		return;
	}

	if (Input::GetKeyTrigger('L') || GetGameObject<Button>(L"ロード")->GetHitBox()->Clicked())
	{
		OpenSaveSlots(false);

		return;
	}

	if (Input::GetKeyTrigger('K'))
	{
		LoadGraph(Game::GetEnemySavePath());
	}

	if (Input::GetKeyTrigger('J'))
	{
		ClearViews();

		m_NodeManager->ClearLogic();

		SetMessage(L"スタートを置いて作り始めましょう");
	}

	if (!Game::IsPracticeStage())
	{
		if (Input::GetKeyTrigger('2'))
		{
			LoadGraph("nodeSaveData/Tutorial1.json");
		}

		if (Input::GetKeyTrigger('3'))
		{
			LoadGraph("nodeSaveData/Tutorial2.json");
		}

		if (Input::GetKeyTrigger('4'))
		{
			LoadGraph("nodeSaveData/Tutorial3.json");
		}

		if (Input::GetKeyTrigger('5'))
		{
			LoadGraph("nodeSaveData/Tutorial4.json");
		}

		if (Input::GetKeyTrigger('6'))
		{
			LoadGraph("nodeSaveData/Tutorial5.json");
		}

		if (Input::GetKeyTrigger('7'))
		{
			LoadGraph("nodeSaveData/Tutorial6.json");
		}

		if (Input::GetKeyTrigger('8'))
		{
			LoadGraph("nodeSaveData/Tutorial7.json");
		}

		// 剣の近距離攻撃を試すサンプル
		if (Input::GetKeyTrigger('9'))
		{
			LoadGraph("nodeSaveData/Tutorial8.json");
		}

		// 条件分岐・回り込み・盾・射撃を試すサンプル
		if (Input::GetKeyTrigger('0'))
		{
			LoadGraph("nodeSaveData/Tutorial9.json");
		}
	}

	// 選んだ種類の一覧へ戻ってステージを変える
	if (Input::GetKeyTrigger(VK_F9) || GetGameObject<Button>(L"ステージ選択")->GetHitBox()->Clicked())
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

		return;
	}

	// 自分で試したあとに、同じステージのサンプルを開ける
	if (!Game::IsPracticeStage() && (Input::GetKeyTrigger(VK_F10) || GetGameObject<Button>(L"サンプルを開く")->GetHitBox()->Clicked()))
	{
		SetOverlay(false, true);
	}

	if (Input::GetKeyTrigger(VK_RETURN) || GetGameObject<Button>(L"保存して挑戦")->GetHitBox()->Clicked())
	{
		if (ValidateGraph())
		{
			OpenSaveSlots(true, true);

			return;
		}
	}

	if (Input::GetKeyTrigger('1'))
	{
		Manager::LoadingSetScene<GameMenu>();
	}

	UpdateConnections();
}

// 編集とテストで、見せるものを切り替える
void NodeEditor::RefreshScreen()
{
	if (m_TestScreen)
	{
		m_ShowHelp = false;

		m_ShowSamples = false;
	}

	if (m_HelpButton && m_HelpPanel)
	{
		m_HelpButton->SetVisible(!m_TestScreen && !m_SlotOpen);

		m_HelpPanel->SetVisible((m_ShowHelp || m_ShowSamples) && !m_TestScreen && !m_SlotOpen);

		for (int index = 0; index < 9; ++index)
		{
			auto *button = m_SampleButtons[index];
			if (button)
			{
				button->SetVisible(m_ShowSamples && !m_TestScreen && !m_SlotOpen);
			}
		}
	}

	if (!m_Background)
	{
		return;
	}

	m_Background->SetVisible(!m_TestScreen);

	m_RoutePanel->SetVisible(m_TestScreen && m_ShowRoute);

	GetGameObject<Field>()->SetVisible(m_TestScreen);

	for (auto *wall : GetGameObjects<StageObstacle>())
	{
		wall->SetVisible(m_TestScreen && (wall->IsArenaBoundary() || Game::m_StageNumber == 3));
	}

	for (auto *view : m_NodeViews)
	{
		view->SetVisible((!m_TestScreen || m_ShowRoute) && !m_SlotOpen);

		view->SetHighlighted(view == m_SelectedNode);
	}

	for (auto *line : m_ConnectedLines)
	{
		line->SetVisible((!m_TestScreen || m_ShowRoute) && !m_SlotOpen);
	}

	if (m_DragLine && m_TestScreen)
	{
		m_DragLine->SetDestroy();

		m_DragLine = nullptr;

		m_Connecting = false;

		m_StartNode = nullptr;
	}

	for (int i = 0; i < 10; ++i)
	{
		bool visibleOnCurrentScreen;

		if (i == 0)
		{
			visibleOnCurrentScreen = !m_TestScreen;
		}
		else
		{
			visibleOnCurrentScreen = m_TestScreen;
		}

		m_Controls[i]->SetVisible(!m_SlotOpen && (i == 6 || (visibleOnCurrentScreen)));
	}

	for (const wchar_t *name : {L"ステージ選択", L"サンプルを開く", L"保存して挑戦", L"セーブ", L"ロード"})
	{
		GetGameObject<Button>(name)->SetVisible(!m_TestScreen && !m_SlotOpen);
	}

	GetGameObject<Button>(L"サンプルを開く")->SetVisible(!Game::IsPracticeStage() && !m_TestScreen && !m_SlotOpen);

	const wchar_t *executionText2;

	if (m_TestLoop)
	{
		executionText2 = L"実行：連続";
	}
	else
	{
		executionText2 = L"実行：1回";
	}

	m_Controls[6]->SetName(executionText2);

	const wchar_t *routeButtonText;

	if (m_ShowRoute)
	{
		routeButtonText = L"ルートを隠す";
	}
	else
	{
		routeButtonText = L"ルートを表示";
	}

	m_Controls[4]->SetName(routeButtonText);
}

// 名前変更・削除・上書きの説明は、保存画面側で描く
bool NodeEditor::DrawSaveSlots()
{
	FontData font;

	font.font = L"メイリオ";
	font.fontSize = 18;
	font.Color = D2D1::ColorF(D2D1::ColorF::White);
	font.shadowColor = D2D1::ColorF(D2D1::ColorF::Black);
	font.shadowOffset = D2D1::Point2F(2, 2);

	if (m_SlotOpen)
	{
		font.fontSize = 28;

		Moji::Instance().SetFont(font);

		// 保存・読込・名前変更・削除に合わせて画面の見出しを変える
		const wchar_t *slotTitle;

		if (m_OverwriteSlot > 0)
		{
			if (m_SlotOperation == 2)
			{
				slotTitle = L"セーブデータの削除確認";
			}
			else if (m_SlotOperation == 1)
			{
				slotTitle = L"セーブデータの名前変更";
			}
			else
			{
				slotTitle = L"セーブデータの上書き確認";
			}
		}
		else if (m_SlotSaving)
		{
			slotTitle = L"セーブする枠を選んでください（5枠まで）";
		}
		else
		{
			slotTitle = L"ロードする枠を選んでください";
		}

		Moji::Instance().Draw(slotTitle, D2D1::Point2F(320, 80), 1);

		font.fontSize = 20;

		Moji::Instance().SetFont(font);

		Moji::Instance().Draw(GetMessage(), D2D1::Point2F(160, 140), 1);

		if (m_SlotSaving && m_SlotOperation == 0)
		{
			Moji::Instance().Draw(L"保存名（24文字まで）", D2D1::Point2F(160, 172), 1);
		}

		if (m_OverwriteSlot > 0)
		{
			const wchar_t *operationText;

			if (m_SlotOperation == 2)
			{
				operationText = L"を削除します。元に戻せません。";
			}
			else if (m_SlotOperation == 1)
			{
				operationText = L"の名前を変更します。";
			}
			else
			{
				operationText = L"を上書きします。";
			}

			Moji::Instance().Draw(m_SaveLoad->GetSlotCaption(m_OverwriteSlot) + operationText,
								  D2D1::Point2F(280, 300), 1);
		}

		return true;
	}

	return false;
}

// 見出しの色と大きさを変えて描き、元の書式に戻す
void NodeEditor::DrawStyledText(const FontData &font, const std::wstring &value, float x, float y, float size, const D2D1_COLOR_F &color)
{
	FontData style = font;

	style.fontSize = size;

	style.Color = color;

	Moji::Instance().SetFont(style);

	Moji::Instance().Draw(value, D2D1::Point2F(x, y), 1, D2D1_DRAW_TEXT_OPTIONS_NONE, true);

	Moji::Instance().SetFont(font);
}

// エディターの状態を左側に影付きで表示する
void NodeEditor::DrawStatusText(const std::wstring &value, float y)
{
	Moji::Instance().Draw(value, D2D1::Point2F(20, y), 1, D2D1_DRAW_TEXT_OPTIONS_NONE, true);
}

// ノードの見た目から、保存に必要な番号と位置だけを取り出す
std::vector<NodePlacement> NodeEditor::GetPlacements() const
{
	std::vector<NodePlacement> placements;
	for (auto *view : m_NodeViews)
	{
		if (view && view->GetNode())
		{
			placements.push_back({view->GetNode()->GetID(), view->GetPosition()});
		}
	}
	return placements;
}

// 画面に表示する説明を取得する
const std::wstring &NodeEditor::GetMessage() const
{
	return m_Message;
}

// 画面に表示する説明を設定する
void NodeEditor::SetMessage(std::wstring value)
{
	m_Message = value;
}

// ノードの管理クラスを取得する
NodeManager *NodeEditor::GetNodeManager() const
{
	return m_NodeManager;
}

// ノードの追加一覧を取得する
NodeCreateList *NodeEditor::GetCreateList() const
{
	return m_CreateList;
}

// 表示倍率を取得する
float NodeEditor::GetZoom() const
{
	return m_Zoom;
}

// 表示倍率を設定する
void NodeEditor::SetZoom(float value)
{
	m_Zoom = value;
}

// 画面の移動量を取得する
Vector2 NodeEditor::GetOffset() const
{
	return m_Offset;
}

// 画面の移動量を設定する
void NodeEditor::SetOffset(Vector2 value)
{
	m_Offset = value;
}

// 前回のマウス位置を設定する
void NodeEditor::SetPrevMouse(Vector2 value)
{
	m_PrevMouse = value;
}

// 保存・読み込みの処理を取得する
NodesSaveLoad *NodeEditor::GetSaveLoad() const
{
	return m_SaveLoad;
}


// 保存枠のボタンを取得する
Button *NodeEditor::GetSlotButtons(int index) const
{
	if (index < 0 || index >= 7)
	{
		return nullptr;
	}

	return m_SlotButtons[index];
}

// 保存名の入力欄を取得する
HWND NodeEditor::GetSaveNameInput() const
{
	return m_SaveNameInput;
}

// 保存画面の表示状態を取得する
bool NodeEditor::GetSlotOpen() const
{
	return m_SlotOpen;
}

// 上書き確認中の枠を取得する
int NodeEditor::GetOverwriteSlot() const
{
	return m_OverwriteSlot;
}

// テスト用のプレイヤーを取得する
Player *NodeEditor::GetTestPlayer() const
{
	return m_TestPlayer;
}

// テスト用の敵を取得する
Player *NodeEditor::GetTestTarget() const
{
	return m_TestTarget;
}

// テスト画面の表示状態を取得する
bool NodeEditor::GetTestScreen() const
{
	return m_TestScreen;
}

// 連続実行の設定を取得する
bool NodeEditor::GetTestLoop() const
{
	return m_TestLoop;
}

// 連続実行の設定を設定する
void NodeEditor::SetTestLoop(bool value)
{
	m_TestLoop = value;
}

// 実行ルートの表示設定を取得する
bool NodeEditor::GetShowRoute() const
{
	return m_ShowRoute;
}

// テストの実行状態を取得する
bool NodeEditor::GetTestActive() const
{
	return m_TestActive;
}

// ヘルプの表示状態を取得する
bool NodeEditor::GetShowHelp() const
{
	return m_ShowHelp;
}

// サンプル一覧の表示状態を取得する
bool NodeEditor::GetShowSamples() const
{
	return m_ShowSamples;
}

// サンプル選択のボタンを取得する
Button *NodeEditor::GetSampleButtons(int index) const
{
	if (index < 0 || index >= 9)
	{
		return nullptr;
	}

	return m_SampleButtons[index];
}

// 背景を取得する
PolyTex *NodeEditor::GetBackground() const
{
	return m_Background;
}

// 実行ルートの背景を取得する
PolyTex *NodeEditor::GetRoutePanel() const
{
	return m_RoutePanel;
}