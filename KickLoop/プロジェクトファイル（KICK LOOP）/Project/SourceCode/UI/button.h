#pragma once

#include "gameObject.h"

class Button : public GameObject
{
private:
	float m_X = 0.0f;
	float m_Y = 0.0f;
	float m_Width = 0.0f;
	float m_Height = 0.0f;

	// ボタンが有効かどうかのフラグ
	bool m_Enabled = true;

	// 前回の左クリック状態を保存するフラグ
	bool m_PrevLeft = false;

	// クリックされたときに呼び出される関数
	std::function<void()> m_OnClick;

public:
	void Init() override;
	void Update() override;
	void Uninit() override {}

	// クリック判定用の矩形を外から設定
	void SetRect(float x, float y, float w, float h)
	{
		m_X = x;
		m_Y = y;
		m_Width = w;
		m_Height = h;
	}

	bool IsPushedOnce();
};