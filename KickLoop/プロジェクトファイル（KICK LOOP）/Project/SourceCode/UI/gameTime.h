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
	float m_Time = 0.0f;  // スタート前のカウントダウンの残り秒数
	float m_LimitTime = 60.0f;  // プレイ中の残り秒数

	float m_AddedTime = 0.0f;  // 表示中に続けて取ったコインの合計秒数
	float m_AddedTimeDisplayTimer = 0.0f;  // 加算秒数を画面に出しておく残り時間（秒）
	ID3D11ShaderResourceView* m_TimeBonusTexture = nullptr;

	TimeMode m_Mode = TimeMode::CountdownStart;

	CriAtomExPlayerHn m_Player = nullptr;
	CriAtomEx3dSourceHn m_Source = nullptr;
	SePlayParams m_Params{};

	bool m_PlayedStartSE = false;

	// 時間切れになったか。
	// Result遷移はGameクラスが見る。
	bool m_IsTimeUp = false;

private:
	// 数字・記号画像の分割数
	static constexpr int SPRITE_DIVIDE_X = 1;   // X（横の列数）
	static constexpr int SPRITE_DIVIDE_Y = 18;  // Y（縦の行数）

	static constexpr float COUNTDOWN_WIDTH = 50.0f;
	static constexpr float COUNTDOWN_HEIGHT = 50.0f;
	static constexpr float COUNTDOWN_Y = 50.0f;

	static constexpr float TIMER_WIDTH = 30.0f;
	static constexpr float TIMER_HEIGHT = 50.0f;
	static constexpr float TIMER_SPACING = 30.0f;
	static constexpr float TIMER_Y = 50.0f;

	static constexpr float TIME_BONUS_DURATION = 1.2f;
	static constexpr float TIME_BONUS_FADE_DURATION = 0.3f;
	static constexpr float TIME_BONUS_WIDTH = 26.0f;
	static constexpr float TIME_BONUS_HEIGHT = 36.0f;
	static constexpr float TIME_BONUS_MARGIN = 8.0f;
	static constexpr int TIME_BONUS_GLYPH_COUNT = 13;

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
	void UpdateTimeBonus(float dt);

	void DrawCountdown();
	void DrawTimer();
	void DrawTimeBonus(float x);

	int GetSpriteIndex(char c) const;

	void PlayStartSE();
};