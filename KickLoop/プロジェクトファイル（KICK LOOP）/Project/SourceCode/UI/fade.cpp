#include "main.h"
#include "fade.h"

void Fade::Init()
{
	m_Polygon.Init();
	m_Polygon.SetTexture("asset\\texture\\kuro.png");
	m_Polygon.SetRect(0.0f, 0.0f, SCREEN_WIDTH, SCREEN_HEIGHT);
	m_Polygon.SetColor(1.0f, 1.0f, 1.0f, 0.0f);

	m_Alpha = 0.0f;
	m_State = None;
	m_Finished = true;
	m_Time = 0.0f;
	m_Timer = 0.0f;
}

void Fade::Uninit()
{
	m_Polygon.Uninit();
}

void Fade::Update()
{
	if (m_State == None || m_Finished)
	{
		return;
	}

	m_Timer += 1.0f / 60.0f;

	float t = (m_Time > 0.0f) ? (m_Timer / m_Time) : 1.0f;

	if (t >= 1.0f)
	{
		t = 1.0f;
		m_Finished = true;
		m_State = None;
	}

	if (m_State == FadeIn)
	{
		m_Alpha = 1.0f - t;
	}

	if (m_State == FadeOut)
	{
		m_Alpha = t;
	}
}

void Fade::Draw()
{
	if (m_Alpha <= 0.0f)
	{
		return;
	}

	m_Polygon.SetAlpha(m_Alpha);
	m_Polygon.Draw();
}

void Fade::StartFadeIn(float time)
{
	if (time <= 0.0f)
	{
		m_State = None;
		m_Time = 0.0f;
		m_Timer = 0.0f;
		m_Finished = true;
		m_Alpha = 0.0f;
		return;
	}

	m_State = FadeIn;
	m_Time = time;
	m_Timer = 0.0f;
	m_Finished = false;
	m_Alpha = 1.0f;
}

void Fade::StartFadeOut(float time)
{
	if (time <= 0.0f)
	{
		m_State = None;
		m_Time = 0.0f;
		m_Timer = 0.0f;
		m_Finished = true;
		m_Alpha = 1.0f;
		return;
	}

	m_State = FadeOut;
	m_Time = time;
	m_Timer = 0.0f;
	m_Finished = false;
	m_Alpha = 0.0f;
}