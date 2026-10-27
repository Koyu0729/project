#include "main.h"
#include "distanceCounter.h"

#include "manager.h"
#include "scene.h"
#include "player.h"

#include <cstdio>
#include <cstring>

void DistanceCounter::Init()
{
	CreateQuad();

	SetTexture("asset\\texture\\configTime.png");

	SetSpriteSheet(
		SPRITE_DIVIDE_X,
		SPRITE_DIVIDE_Y
	);

	m_Distance = 0.0f;

	SetVisible(true);
}

void DistanceCounter::Uninit()
{
	UISpriteSheetBase::Uninit();
}

void DistanceCounter::Update()
{
	Player* player =
		Manager::GetScene()->GetGameObject<Player>();

	if (player == nullptr)
	{
		m_Distance = 0.0f;
		return;
	}

	m_Distance =
		player->GetProgressDistance();

	if (m_Distance < 1.0f)
	{
		m_Distance = 0.0f;
	}
}

void DistanceCounter::Draw()
{
	DrawDistance();
}

void DistanceCounter::DrawDistance()
{
	char text[16]{};
	sprintf_s(text, "%.0fm", m_Distance);

	int length = (int)strlen(text);

	if (length <= 0)
	{
		return;
	}

	float totalWidth =
		DIGIT_WIDTH +
		DIGIT_SPACING * (float)(length - 1);

	float startX =
		((float)SCREEN_WIDTH - totalWidth) * 0.5f;

	for (int i = 0; i < length; i++)
	{
		int index = GetSpriteIndex(text[i]);

		if (index < 0)
		{
			continue;
		}

		DrawSpriteIndex(
			index,
			startX + DIGIT_SPACING * (float)i,
			DRAW_Y,
			DIGIT_WIDTH,
			DIGIT_HEIGHT
		);
	}
}

int DistanceCounter::GetSpriteIndex(char c) const
{
	if (c == '1') return INDEX_1;
	if (c == '2') return INDEX_2;
	if (c == '3') return INDEX_3;
	if (c == '4') return INDEX_4;
	if (c == '5') return INDEX_5;
	if (c == '6') return INDEX_6;
	if (c == '7') return INDEX_7;
	if (c == '8') return INDEX_8;
	if (c == '9') return INDEX_9;
	if (c == '0') return INDEX_0;
	if (c == '.') return INDEX_DOT;
	if (c == 'm') return INDEX_M;

	return -1;
}