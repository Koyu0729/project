#pragma once

#include "scene.h"
#include "uiHitBox.h"

class Result : public Scene
{

	// 対戦結果を結果画面へ設定する
	static std::wstring m_Outcome;

  public:
	// CPU側のデータを準備する
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

	static void SetOutcome(const std::wstring &outcome)
	{
		m_Outcome = outcome;
	}

	static const std::wstring &GetOutcome()
	{
		return m_Outcome;
	}

	// ADX用
	SePlayParams m_params;					// 再生パラメータ
	CriAtomEx3dSourceHn m_Source = nullptr; // 3Dソースハンドル
	CriAtomExPlaybackId m_SourceId = -1;	// 再生ID
};
