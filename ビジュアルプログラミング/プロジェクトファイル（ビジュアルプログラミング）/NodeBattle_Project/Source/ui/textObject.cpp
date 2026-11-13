#include "main.h"
#include "textObject.h"

// 一覧の標準書式を初期化する
void TextObject::Init()
{
	TextObjectBase::Init();
	m_Color = D2D1::ColorF(D2D1::ColorF::White);
}

// 一覧の文字は専用のGPUリソースを所有しない
void TextObject::Uninit()
{
}

// ノード項目のクリックを選択状態へ反映する
void TextObject::Update()
{
	if (!m_Hit || !m_Visible)
	{
		return;
	}

	if (m_Hit->Clicked() && m_Type != NodeType::Type_Null)
	{
		m_Select = true;
	}
}

// マウスが重なった項目を強調し、基底の描画処理を使う
void TextObject::Draw()
{
	if (!m_Visible || !m_Hit)
	{
		return;
	}

	D2D1_COLOR_F color = m_Color;

	if (m_Hit->OverlapAABB() && m_Type != NodeType::Type_Null)
	{
		color = D2D1::ColorF(D2D1::ColorF::LightGoldenrodYellow);
	}

	DrawText(color, true);
}
