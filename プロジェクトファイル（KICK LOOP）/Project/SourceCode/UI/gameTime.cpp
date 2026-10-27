#include "main.h"
#include "gameTime.h"

#include "manager.h"
#include "scene.h"

#include <cmath>
#include <cstdio>
#include <cstring>

void Time::Init()
{
	CreateQuad();

	SetTexture("asset\\texture\\configTime.png");

	SetSpriteSheet(
		SPRITE_DIVIDE_X,
		SPRITE_DIVIDE_Y
	);

	SetVisible(true);
	SetColor(1.0f, 1.0f, 1.0f, 1.0f);

	m_Time = 3.0f;
	m_LimitTime = 60.0f;

	m_Mode = TimeMode::CountdownStart;

	m_PlayedStartSE = false;
	m_IsTimeUp = false;

	m_Player = AudioManager::CreatePlayer();

	m_Params.cueName = "start";
	m_Params.categoryId = SE;
	m_Params.is3D = false;
	m_Params.loopCount = 0;
	m_Params.volume = 0.0f;

	m_Source = criAtomEx3dSource_Create(NULL, NULL, 0);
}

void Time::Uninit()
{
	if (m_Player)
	{
		criAtomExPlayer_Stop(m_Player);
		criAtomExPlayer_UpdateAll(m_Player);
	}

	if (m_Source)
	{
		criAtomEx3dSource_Destroy(m_Source);
		m_Source = nullptr;
	}

	if (m_Player)
	{
		AudioManager::DestroyPlayer(m_Player);
		m_Player = nullptr;
	}

	UISpriteSheetBase::Uninit();
}

void Time::Update()
{
	if (m_Player != nullptr)
	{
		AudioManager::ApplyVolumeToPlayer(
			m_Player,
			VOICE,
			0.15f
		);
	}
	// クリア演出中は時間を進めない。
	if (m_Mode == TimeMode::TimeStop)
	{
		return;
	}

	float dt =
		1.0f / 60.0f;

	if (m_Mode == TimeMode::CountdownStart)
	{
		UpdateCountdown(
			dt
		);

		return;
	}

	if (m_Mode == TimeMode::CountdownTimer)
	{
		UpdateTimer(
			dt
		);

		return;
	}

	if (m_Mode == TimeMode::TimeUp)
	{
		return;
	}
}

void Time::Draw()
{
	SetDraw2D();

	if (m_Mode == TimeMode::CountdownStart)
	{
		DrawCountdown();
		return;
	}

	if (m_Mode == TimeMode::CountdownTimer)
	{
		DrawTimer();
		return;
	}

	if (m_Mode == TimeMode::TimeStop)
	{
		DrawTimer();
		return;
	}

	if (m_Mode == TimeMode::TimeUp)
	{
		DrawTimer();
		return;
	}
}

void Time::UpdateCountdown(float dt)
{
	m_Time -=
		dt;

	if (!m_PlayedStartSE)
	{
		PlayStartSE();

		m_PlayedStartSE =
			true;
	}

	if (m_Time <= 0.0f)
	{
		m_Time =
			0.0f;

		m_Mode =
			TimeMode::CountdownTimer;
	}
}

void Time::UpdateTimer(float dt)
{
	m_LimitTime -=
		dt;

	if (m_LimitTime <= 0.0f)
	{
		m_LimitTime =
			0.0f;

		m_Mode =
			TimeMode::TimeUp;

		m_IsTimeUp =
			true;
	}
}

void Time::DrawCountdown()
{
	int count =
		(int)ceilf(m_Time);

	if (count <= 0)
	{
		return;
	}

	char number =
		(char)('0' + count);

	int index =
		GetSpriteIndex(
			number
		);

	if (index < 0)
	{
		return;
	}

	float x =
		((float)SCREEN_WIDTH - COUNTDOWN_WIDTH) *
		0.5f;

	DrawSpriteIndex(
		index,
		x,
		COUNTDOWN_Y,
		COUNTDOWN_WIDTH,
		COUNTDOWN_HEIGHT
	);
}

void Time::DrawTimer()
{
	int displayTime =
		(int)ceilf(m_LimitTime);

	if (displayTime < 0)
	{
		displayTime =
			0;
	}

	char text[16]{};

	sprintf_s(
		text,
		"%02d",
		displayTime
	);

	int length =
		(int)strlen(text);

	if (length <= 0)
	{
		return;
	}

	float totalWidth =
		TIMER_WIDTH +
		TIMER_SPACING * (float)(length - 1);

	float startX =
		((float)SCREEN_WIDTH - totalWidth) *
		0.5f;

	for (int i = 0; i < length; i++)
	{
		int index =
			GetSpriteIndex(
				text[i]
			);

		if (index < 0)
		{
			continue;
		}

		DrawSpriteIndex(
			index,
			startX + TIMER_SPACING * (float)i,
			TIMER_Y,
			TIMER_WIDTH,
			TIMER_HEIGHT
		);
	}
}

int Time::GetSpriteIndex(char c) const
{
	if (c == '1') return 0;
	if (c == '2') return 1;
	if (c == '3') return 2;
	if (c == '4') return 3;
	if (c == '5') return 4;
	if (c == '6') return 5;
	if (c == '7') return 6;
	if (c == '8') return 7;
	if (c == '9') return 8;
	if (c == '0') return 9;
	if (c == '.') return 10;
	if (c == 'm') return 17;

	return -1;
}

void Time::AddLimitTime(float addTime)
{
	if (addTime <= 0.0f)
	{
		return;
	}

	if (m_Mode != TimeMode::CountdownTimer)
	{
		return;
	}

	m_LimitTime +=
		addTime;

	if (m_LimitTime < 0.0f)
	{
		m_LimitTime =
			0.0f;
	}
}

void Time::ReduceLimitTime(float reduceTime)
{
	if (reduceTime <= 0.0f)
	{
		return;
	}

	if (m_Mode != TimeMode::CountdownTimer)
	{
		return;
	}

	m_LimitTime -=
		reduceTime;

	if (m_LimitTime <= 0.0f)
	{
		m_LimitTime =
			0.0f;

		m_Mode =
			TimeMode::TimeUp;

		m_IsTimeUp =
			true;
	}
}

void Time::SetLimitTime(float limitTime)
{
	m_LimitTime =
		limitTime;

	if (m_LimitTime < 0.0f)
	{
		m_LimitTime =
			0.0f;
	}
}

void Time::StopTime()
{
	if (m_Mode == TimeMode::TimeUp)
	{
		return;
	}

	m_Mode =
		TimeMode::TimeStop;
}

void Time::ResumeTime()
{
	if (m_Mode != TimeMode::TimeStop)
	{
		return;
	}

	m_Mode = TimeMode::CountdownTimer;
}

void Time::PlayStartSE()
{
	if (m_Player == nullptr)
	{
		return;
	}

	m_Params.cueName = "start";
	m_Params.categoryId = VOICE;
	m_Params.volume = 0.15f;

	AudioManager::PlaySeStart(
		m_Player,
		m_Params,
		nullptr
	);
}