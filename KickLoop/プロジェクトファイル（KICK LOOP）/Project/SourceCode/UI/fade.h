#pragma once

#include "polygon2D.h"

class Fade
{
public:
	enum State
	{
		None,
		FadeIn,
		FadeOut
	};

private:
	State m_State = None;

	float m_Time = 0.0f;
	float m_Timer = 0.0f;
	float m_Alpha = 0.0f;

	bool m_Finished = true;

	Polygon2D m_Polygon;

public:
	void Init();
	void Uninit();
	void Update();
	void Draw();

	void StartFadeIn(float time);
	void StartFadeOut(float time);

	bool IsFinished() const { return m_Finished; }
	State GetState() const { return m_State; }
	bool IsFade() const { return m_State != None; }
};