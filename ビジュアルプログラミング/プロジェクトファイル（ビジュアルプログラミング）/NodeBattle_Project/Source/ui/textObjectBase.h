#pragma once
#include "gameObject2D.h"
#include "moji.h"

// 文字列・書式・色・表示効果を管理し、共通の描画を行う
class TextObjectBase : public GameObject2D
{
  protected:
	std::wstring m_Text = L"初期化";
	FontData m_Font = {};

	float m_Zoom = 1.0f;

	// 文字列と書式を使い、色と影の指定に従って描画する
	void DrawText(const D2D1_COLOR_F &color, bool shadow);

	D2D1_COLOR_F m_Color = D2D1::ColorF(D2D1::ColorF::Black);

  private:
	enum class TextMoveState
	{
		None,	  // 動かない
		Float,	  // 浮遊表示
		Vibration // 震える
	};

	TextMoveState m_TextMoveState = TextMoveState::None;

	D2D1_POINT_2F TextMove(TextMoveState state);

  public:
	void Init() override;
	void Draw() override;

	void ClearMove()
	{
		m_TextMoveState = TextMoveState::None;
	}
	// テキスト系
	void SetText(const wchar_t *text)
	{
		m_Text = text;
	}

	const wchar_t *GetText()
	{
		return m_Text.c_str();
	}
	void SetFont(const FontData &font)
	{
		m_Font = font;
		m_Color = font.Color;
	}

	const FontData &GetFontDate() const
	{
		return m_Font;
	}

	void SetColor(float r = 1.0f, float g = 1.0f, float b = 1.0f, float a = 1.0f)
	{
		m_Color = D2D1::ColorF(r, g, b, a);
	}

	// 文字の表示倍率を設定する
	void SetFontSize(const float zoom)
	{
		m_Zoom = zoom;
	}

	// 文字を浮遊表示へ切り替える
	void SetStateFloat()
	{
		m_TextMoveState = TextMoveState::Float;
	}
	// 文字を振動表示へ切り替える
	void SetStateVibration()
	{
		m_TextMoveState = TextMoveState::Vibration;
	}
};