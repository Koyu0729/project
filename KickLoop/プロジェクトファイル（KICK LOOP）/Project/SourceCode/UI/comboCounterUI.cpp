#include "main.h"
#include "comboCounterUI.h"
#include "texture.h"

#include "manager.h"
#include "scene.h"
#include "player.h"

#include <cstring>
#include <cstdio>

void ComboCounterUI::Init()
{
	CreateQuad();

	m_NumberTexture = Texture::Load("asset\\texture\\comboNumber.png");
	m_TextTexture = Texture::Load("asset\\texture\\comboText.png");

	m_Combo = 0;
	m_OldCombo = 0;

	SetVisible(false);

	m_UIScale = 1.0f;
	m_PopPower = 0.0f;

	m_BaseX = 95.0f;
	m_BaseY = 105.0f;
}

void ComboCounterUI::Uninit()
{
	m_NumberTexture = nullptr;
	m_TextTexture = nullptr;

	UISpriteSheetBase::Uninit();
}

void ComboCounterUI::Update()
{
	Player* player = Manager::GetScene()->GetGameObject<Player>();

	if (player == nullptr)
	{
		SetCombo(0);
		SetVisible(false);
		return;
	}

	SetCombo(player->GetCombo());

	if (m_Combo <= 0)
	{
		SetVisible(false);

		m_OldCombo = 0;
		m_PopPower = 0.0f;
		m_UIScale = 1.0f;

		return;
	}

	SetVisible(true);

	if (m_Combo != m_OldCombo)
	{
		m_PopPower = 1.0f;
		m_OldCombo = m_Combo;
	}

	if (m_PopPower > 0.0f)
	{
		m_PopPower -= 0.08f;

		if (m_PopPower < 0.0f)
		{
			m_PopPower = 0.0f;
		}
	}

	m_UIScale = 1.0f + m_PopPower * 0.35f;
}

void ComboCounterUI::Draw()
{
	if (m_NumberTexture == nullptr)
	{
		return;
	}

	if (m_TextTexture == nullptr)
	{
		return;
	}

	SetDraw2D();

	DrawComboNumber(m_BaseX, m_BaseY);
	DrawComboText(m_BaseX - 55.0f, m_BaseY + 72.0f);
}

void ComboCounterUI::SetCombo(int combo)
{
	if (combo < 0)
	{
		combo = 0;
	}

	m_Combo = combo;
}

int ComboCounterUI::GetCombo() const
{
	return m_Combo;
}

void ComboCounterUI::SetBasePosition(float x, float y)
{
	m_BaseX = x;
	m_BaseY = y;
}

void ComboCounterUI::DrawComboNumber(float x, float y)
{
	char buffer[16]{};
	sprintf_s(buffer, "%d", m_Combo);

	int length = (int)strlen(buffer);

	float digitWidth = 48.0f * m_UIScale;
	float digitHeight = 70.0f * m_UIScale;
	float digitInterval = 34.0f * m_UIScale;

	float totalWidth =
		digitWidth +
		digitInterval * (float)(length - 1);

	float startX =
		x -
		totalWidth * 0.5f;

	DrawTextDigit(
		TEXT_INDEX_X,
		startX - 42.0f * m_UIScale,
		y + 4.0f * m_UIScale,
		38.0f * m_UIScale,
		58.0f * m_UIScale
	);

	for (int i = 0; i < length; i++)
	{
		int index = GetNumberIndex(buffer[i]);

		DrawNumberDigit(
			index,
			startX + digitInterval * (float)i,
			y,
			digitWidth,
			digitHeight
		);
	}
}

void ComboCounterUI::DrawComboText(float x, float y)
{
	float width = 34.0f;
	float height = 50.0f;
	float interval = 27.0f;

	DrawTextDigit(TEXT_INDEX_C, x + interval * 0.0f, y, width, height);
	DrawTextDigit(TEXT_INDEX_O, x + interval * 1.0f, y, width, height);
	DrawTextDigit(TEXT_INDEX_M, x + interval * 2.0f, y, width, height);
	DrawTextDigit(TEXT_INDEX_B, x + interval * 3.0f, y, width, height);
	DrawTextDigit(TEXT_INDEX_O_LAST, x + interval * 4.0f, y, width, height);
}

void ComboCounterUI::DrawNumberDigit(
	int index,
	float x,
	float y,
	float width,
	float height
)
{
	DrawSpriteIndex(
		m_NumberTexture,
		index,
		1,
		NUMBER_ROW_COUNT,
		x,
		y,
		width,
		height
	);
}

void ComboCounterUI::DrawTextDigit(
	int index,
	float x,
	float y,
	float width,
	float height
)
{
	DrawSpriteIndex(
		m_TextTexture,
		index,
		1,
		TEXT_ROW_COUNT,
		x,
		y,
		width,
		height
	);
}

int ComboCounterUI::GetNumberIndex(char number)
{
	if (number == '0') return NUMBER_INDEX_0;
	if (number == '1') return NUMBER_INDEX_1;
	if (number == '2') return NUMBER_INDEX_2;
	if (number == '3') return NUMBER_INDEX_3;
	if (number == '4') return NUMBER_INDEX_4;
	if (number == '5') return NUMBER_INDEX_5;
	if (number == '6') return NUMBER_INDEX_6;
	if (number == '7') return NUMBER_INDEX_7;
	if (number == '8') return NUMBER_INDEX_8;
	if (number == '9') return NUMBER_INDEX_9;

	return NUMBER_INDEX_0;
}