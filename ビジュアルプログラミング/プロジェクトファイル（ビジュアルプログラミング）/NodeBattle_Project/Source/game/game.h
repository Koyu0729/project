#pragma once
#include "scene.h"
class Player;
class NodeBase;

class Game : public Scene
{

	// 戦闘シーンの初期化・更新・描画を行う

	// 対戦する自分と敵
	Player *m_Player = nullptr;

	Player *m_Enemy = nullptr;

	// CPUロードで検証したノードは、GPU側のプレイヤー作成までこのシーンが所有する
	std::vector<NodeBase *> m_PreparedPlayer;

	std::vector<NodeBase *> m_PreparedEnemy;

	int m_PreparedEnemyHP = 100;

	bool m_CPUPrepared = false;

	// GPU準備後のInitで、オブジェクトを二重に作らないための状態
	bool m_Initialized = false;

	// 開始までの時間と、対戦が始まってからの時間
	float m_Countdown = 3.0f;

	float m_Elapsed = 0.0f;

	// 一時停止と、対戦が終わったかのフラグ
	bool m_Paused = false;

	bool m_Finished = false;

	// ノードに問題があったときの説明
	std::wstring m_Error;

	// 対戦画面のボタンと状態表示
	bool IsButtonClicked(const wchar_t *name);
	void DrawStatusText(const std::wstring &value, float y);

  public:
	// 選んだステージと、最初にサンプルを開くかの情報
	static int m_StageNumber;

	static bool m_OpenStageExample;

	// ステージ一覧から数えた、種類ごとのステージ数
	static const int TutorialCount;

	static const int PracticeCount;
	// 保持している実行用ノードを解放する
	~Game() override;

	bool InitCPU() override;

	// 描画に必要なデータを準備する
	bool InitGPU() override;

	// 初期化処理を行う
	bool Init() override;

	// 終了処理を行う
	void Uninit() override;

	// 毎フレームの処理
	void Update() override;

	// 画面に描く処理
	void Draw() override;

	// メニューから挑戦するステージを選ぶ（0は自由対戦）
	static void SelectStage(int number, bool openExample = false);

	static int GetTutorialNumber();

	static int GetTutorialStage(int tutorialNumber);

	static int GetNextTutorialStage();

	static int GetPracticeNumber();

	static int GetPracticeStage(int number);

	// 実践かどうかと、最初に開くノードのサンプルを確認する
	static bool IsPracticeStage();

	// 編集画面や対戦画面に出す、ステージの名前とヒント
	static std::wstring GetStageName();

	static std::wstring GetStageHint();

	static std::string GetStageSamplePath();

	// 敵の保存先を選び、対戦と練習で共通のHP・行動を読み込む
	static std::string GetEnemySavePath();

	static bool LoadEnemyData(Player *enemy, std::wstring &message);

	// 対戦のエラー説明を消す
	void ClearError();

	// 待機の次に攻撃がある場合だけ、敵の準備として表示する
	static bool IsAttackPreparation(const NodeBase *node);

	// 対戦中のプレイヤーを取得する
	Player *GetPlayer() const;

	// 対戦中の敵を取得する
	Player *GetEnemy() const;

	// CPU側で読み込んだプレイヤーのノードを取得する
	const std::vector<NodeBase *> &GetPreparedPlayer() const;

	// CPU側で読み込んだ敵のノードを取得する
	const std::vector<NodeBase *> &GetPreparedEnemy() const;

	// 画面の初期化状態を取得する
	bool GetInitialized() const;

	// 対戦開始までの時間を設定する
	void SetCountdown(float value);

	// 対戦の経過時間を取得する
	float GetElapsed() const;

	// 対戦の経過時間を設定する
	void SetElapsed(float value);

	// 対戦が終了したかどうかを取得する
	bool GetFinished() const;

	// エラーの説明を取得する
	const std::wstring &GetError() const;
};
