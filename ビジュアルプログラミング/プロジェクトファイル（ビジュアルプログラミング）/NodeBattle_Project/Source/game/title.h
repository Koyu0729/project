#pragma once

#include "scene.h"
#include "moji.h"

#include "uiHitBox.h"

class Title : public Scene

{

	// タイトル画面だけで使う文字の書式と描画
	FontData m_TitleFont;
	void DrawTitleText(const wchar_t *value, float x, float y, float size, D2D1_COLOR_F color);

  public:
	bool InitCPU() override;

	bool InitGPU() override;

	bool Init() override;

	void Uninit() override;

	void Update() override;

	void Draw() override;

	// ADX用

	SePlayParams m_params; // 再生パラメータ

	CriAtomEx3dSourceHn m_Source = nullptr; // 3Dソースハンドル

	CriAtomExPlaybackId m_SourceId = -1; // 再生ID
};