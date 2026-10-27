#pragma once

#include "scene.h"

#include <d2d1.h>

#include "nodeCreate.h"

#include "nodeView.h"

#include "drawLine.h"

#include "nodeManager.h"
#include "nodesSaveLoad.h"

struct FontData;

class Button;

class PolyTex;

class NodeCreateList;

class NodesSaveLoad;

class Player;

class NodeEditor : public Scene
{

	// ノードの配置・選択・接続と画面の移動
	NodeManager *m_NodeManager = nullptr;

	std::vector<NodeView *> m_NodeViews;

	NodeView *m_SelectedNode = nullptr;

	DrawLine *m_DragLine = nullptr;

	std::vector<DrawLine *> m_ConnectedLines;

	bool m_Connecting = false;

	NodeView *m_StartNode = nullptr;

	Port m_StartPort = Port::PortNull;

	NodeCreateList *m_CreateList = nullptr;

	float m_Zoom = 1, m_ZoomMin = 0.3f, m_ZoomMax = 2;

	Vector2 m_Offset{}, m_PrevMouse{};

	// 保存枠の画面・名前入力・確認状態
	NodesSaveLoad *m_SaveLoad = nullptr;

	PolyTex *m_SlotPanel = nullptr;

	Button *m_SlotButtons[7]{};

	Button *m_SlotManageButtons[10]{};

	HWND m_SaveNameInput = nullptr;

	int m_SlotOperation = 0;

	bool m_SlotOpen = false;

	bool m_SlotSaving = false;

	bool m_SlotChallenge = false;

	int m_OverwriteSlot = 0;

	// テスト用のプレイヤーと実行状態
	Player *m_TestPlayer = nullptr;

	Player *m_TestTarget = nullptr;

	bool m_TestScreen = false;

	bool m_TestLoop = false;

	bool m_ShowRoute = true;

	bool m_TestActive = false;

	bool m_PreviewVisible = false;

	// ヘルプとサンプル一覧の表示状態
	bool m_ShowHelp = false;

	Button *m_HelpButton = nullptr;

	PolyTex *m_HelpPanel = nullptr;

	bool m_ShowSamples = false;

	Button *m_SampleButtons[9]{};

	// 編集画面の背景と操作ボタン
	PolyTex *m_Background = nullptr;

	PolyTex *m_RoutePanel = nullptr;

	Button *m_Controls[10]{};

	// GPU準備後のInitで、ボタンやプレイヤーを二重に作らないための状態
	bool m_Initialized = false;

  public:
	~NodeEditor() override;

	bool InitCPU() override;

	bool InitGPU() override;

	// 初期化処理を行う
	bool Init() override;

	// 終了処理を行う
	void Uninit() override;

	// 毎フレームの処理
	void Update() override;

	// 画面に描く処理
	void Draw() override;

	// 保存したノードと配置を読み込む
	bool LoadGraph(const std::string &path);

	// ノードの見た目と線を解放する
	void ClearViews();

	// 一覧で選んだ種類のノードを置く
	void NodeCreateInput();

	// クリックしたノードを選ぶ
	void NodeSelect();

	// 選んだノードと、それにつながる線を消す
	void NodeDelete();

	// 出力と入力をドラッグでつなぐ
	void NodeConnect();

	// 選んだノードの時間・距離・角度などを変える
	void EditSettings();

	// ホイールで拡大縮小し、中ボタンで画面を動かす
	void MoveAllNodes();

	// ノードの場所に合わせて接続の線を更新する
	void UpdateConnections();

	// 編集中のノードの接続を確認し、結果を説明欄へ渡す
	bool ValidateGraph();

	// 画面に出す説明
  private:
	std::wstring m_Message = L"右クリックでノードを追加。出力から入力へドラッグして接続";

  public:
	// 保存画面の準備と説明の描画
	void InitSaveSlots();
	// 枠選択の開始・操作・終了と、保存後の対戦開始
	void OpenSaveSlots(bool saving, bool challenge = false);

	void CloseSaveSlots();

	void UpdateSaveSlots();

	void ChooseSaveSlot(int slot);

	void CommitSaveSlot(int slot, bool overwrite);

	// 各枠の名前変更・削除と、日本語入力用の欄

	void ManageSaveSlot(int slot, bool deleting);

	void ConfirmSaveSlot();

	// 編集したノードをコピーして、練習を始める
	void StartTest(bool pause = false);

	// 練習を止めて、編集に戻る
	void StopTest();

	// 押されたボタンに合わせて、実行やカメラを切り替える
	void HandleControl(int index);

	// 練習中の入力と実行結果を更新する
	void HandleTestInput();

	void UpdateTestResult();

	void SetOverlay(bool help, bool samples);

	void UpdateOverlay();

	void ChooseSample(int index);

	void UpdateEditorShortcuts();

	// 編集とテストで、見せるものを切り替える
	void RefreshScreen();

	bool DrawSaveSlots();

	void DrawStyledText(const FontData &font, const std::wstring &value, float x, float y, float size, const D2D1_COLOR_F &color);

	void DrawStatusText(const std::wstring &value, float y);

	// ファイルへ渡す配置データを集める
	std::vector<NodePlacement> GetPlacements() const;

	// 画面に表示する説明を取得する
	const std::wstring &GetMessage() const;

	// 画面に表示する説明を設定する
	void SetMessage(std::wstring value);

	// 保存画面のEscはゲーム終了ではなくキャンセルに使う
	bool IsSaveSlotsOpen() const
	{
		return m_SlotOpen;
	}

	bool IsEditorDialogOpen() const
	{
		return m_SlotOpen || m_ShowHelp || m_ShowSamples;
	}

	void SetViews(NodeView *view)
	{
		if (view)
		{
			m_NodeViews.push_back(view);
		}
	}

	const std::vector<NodeView *> &GetViews() const
	{
		return m_NodeViews;
	}

	// 画面の状態を確認するための読み取り・操作関数

	NodeManager *GetNodeManager() const;

	NodeCreateList *GetCreateList() const;

	float GetZoom() const;

	void SetZoom(float value);

	Vector2 GetOffset() const;

	void SetOffset(Vector2 value);

	void SetPrevMouse(Vector2 value);

	NodesSaveLoad *GetSaveLoad() const;


	Button *GetSlotButtons(int index) const;

	HWND GetSaveNameInput() const;

	bool GetSlotOpen() const;

	int GetOverwriteSlot() const;

	Player *GetTestPlayer() const;

	Player *GetTestTarget() const;

	bool GetTestScreen() const;

	bool GetTestLoop() const;

	void SetTestLoop(bool value);

	bool GetShowRoute() const;

	bool GetTestActive() const;

	bool GetShowHelp() const;

	bool GetShowSamples() const;

	Button *GetSampleButtons(int index) const;

	PolyTex *GetBackground() const;

	PolyTex *GetRoutePanel() const;
};
