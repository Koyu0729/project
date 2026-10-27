#include "main.h"
#include "textObjectBase.h"
#include "moji.h"
#include "renderer.h"
void TextObjectBase::Init()
{
	m_Font.font = L"ZeroGothic";
	m_Font.fontSize = 16.0f;
	m_Font.Color = D2D1::ColorF(D2D1::ColorF::Black);
}

// 通常の文字表示は影なしで描く
void TextObjectBase::Draw()
{
	if (!m_Visible)
	{
		return;
	}

	DrawText(m_Color, false);
}

D2D1_POINT_2F TextObjectBase::TextMove(TextMoveState state)
{
	D2D1_POINT_2F offset = {0.0f, 0.0f};

	switch (state)
	{
	case TextMoveState::None:

		break;

	case TextMoveState::Float:
	{
		static float phase = 0.0f;
		phase += 0.05f;

		float amplitude = 5.0f;
		offset.y = sinf(phase) * amplitude;
		break;
	}

	case TextMoveState::Vibration:
	{
		// 警告や強調に使う振動表示
		float range = 2.0f;

		offset.x = ((rand() % 100) / 100.0f - 0.5f) * range;
		offset.y = ((rand() % 100) / 100.0f - 0.5f) * range;
		break;
	}
	}

	return offset;
}

// 基底の文字列と書式を使って描画する
void TextObjectBase::DrawText(const D2D1_COLOR_F &color, bool shadow)
{
	m_Font.Color = color;
	// フォント設定
	Moji::Instance().SetFont(m_Font);

	D2D1_POINT_2F basePos = D2D1::Point2F(m_Position.x, m_Position.y);
	D2D1_POINT_2F offset = TextMove(m_TextMoveState);
	D2D1_POINT_2F drawPos =
		{
			basePos.x + offset.x,
			basePos.y + offset.y};

	// 描画
	Moji::Instance().Draw(
		m_Text.c_str(),
		drawPos,
		m_Zoom,
		D2D1_DRAW_TEXT_OPTIONS_NONE,
		shadow);
}
