#pragma once

#include "UISpriteSheetBase.h"
#include "kosu_s_cri.h"

enum class TimeMode
{
	CountdownStart,
	CountdownTimer,

	// ゲームオーバー
	TimeUp,

	// ゲームクリア
	TimeStop,
};

class Time : public UISpriteSheetBase
{
private:
	float m_Time = 0.0f;
	float m_LimitTime = 60.0f;

	TimeMode m_Mode = TimeMode::CountdownStart;

	CriAtomExPlayerHn m_Player = nullptr;
	CriAtomEx3dSourceHn m_Source = nullptr;
	SePlayParams m_Params{};

	bool m_PlayedStartSE = false;

	// 時間切れになったか。
	// Result遷移はGameクラスが見る。
	bool m_IsTimeUp = false;

private:
	static constexpr int SPRITE_DIVIDE_X = 1;
	static constexpr int SPRITE_DIVIDE_Y = 18;

	static constexpr float COUNTDOWN_WIDTH = 50.0f;
	static constexpr float COUNTDOWN_HEIGHT = 50.0f;
	static constexpr float COUNTDOWN_Y = 50.0f;

	static constexpr float TIMER_WIDTH = 30.0f;
	static constexpr float TIMER_HEIGHT = 50.0f;
	static constexpr float TIMER_SPACING = 30.0f;
	static constexpr float TIMER_Y = 50.0f;

public:
	void Init() override;
	void Uninit() override;
	void Update() override;
	void Draw() override;

	bool IsCountup() const { return m_Mode == TimeMode::CountdownStart; }
	bool IsPlaying() const { return m_Mode == TimeMode::CountdownTimer; }
	bool IsTimeUp() const { return m_IsTimeUp; }
	bool IsTimeStop() const { return m_Mode == TimeMode::TimeStop; }

	float GetLimitTime() const { return m_LimitTime; }

	void AddLimitTime(float addTime);
	void ReduceLimitTime(float reduceTime);
	void SetLimitTime(float limitTime);

	void StopTime();
	void ResumeTime();

private:
	void UpdateCountdown(float dt);
	void UpdateTimer(float dt);

	void DrawCountdown();
	void DrawTimer();

	int GetSpriteIndex(char c) const;

	void PlayStartSE();
};