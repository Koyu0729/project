#pragma once
#include "kosuCRI.h"

#include "scene.h"

// 最初のメニューと、2種類のステージ一覧
enum class StageMenuPage
{
	Main,
	Tutorial,
	Practice
};

class GameMenu : public Scene
{
	// 表示中のステージ一覧の種類
	StageMenuPage m_Page = StageMenuPage::Main;

	// 非表示のボタンには入力を渡さない
	bool IsButtonClicked(const wchar_t *name);

  public:
	// 結果画面や編集画面から戻る際に表示する一覧を指定する
	static StageMenuPage m_InitialPage;
	// CPU側の追加読み込みは使わない
	bool InitCPU() override;

	// GPU側の追加読み込みは使わない
	bool InitGPU() override;

	// メニューの背景とボタンを用意する
	bool Init() override;

	// BGMと画面を解放する
	void Uninit() override;

	// 押されたボタンに合わせて一覧や編集画面を開く
	void Update() override;

	// 表示中の一覧の説明を描画する
	void Draw() override;

	// 一覧のボタンをまとめて切り替える
	void ShowPage(StageMenuPage page);

	// BGMを再生・停止するための情報
	SePlayParams m_params;

	CriAtomEx3dSourceHn m_Source = nullptr;

	CriAtomExPlaybackId m_SourceId = -1;
};
