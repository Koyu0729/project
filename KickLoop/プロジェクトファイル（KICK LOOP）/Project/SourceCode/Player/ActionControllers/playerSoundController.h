#pragma once

#include "main.h"
#include "kosu_s_cri.h"

// プレイヤー操作やアクション結果に合わせてSEを再生するクラス。
class PlayerSoundController
{
private:
	CriAtomExPlayerHn m_JumpPlayer = nullptr;
	CriAtomExPlayerHn m_LandingPlayer = nullptr;
	CriAtomExPlayerHn m_BlinkPlayer = nullptr;
	CriAtomExPlayerHn m_GrabPlayer = nullptr;
	CriAtomExPlayerHn m_SlidePlayer = nullptr;

	CriAtomExPlayerHn m_CoinPlayer = nullptr;
	CriAtomExPlayerHn m_GaugePlayer = nullptr;
	CriAtomExPlayerHn m_ComboPlayer = nullptr;
	CriAtomExPlayerHn m_ComboLastPlayer = nullptr;

private:
	void PlayOneShot(
		CriAtomExPlayerHn player,
		const char* cueName,
		Sounds categoryId,
		float volume,
		float pitch = 0.0f
	);

public:
	void Init();
	void Uninit();

	void UpdateListenerPosition(const Vector3& position);

	// プレイヤーの基本アクションに対応するSE。
	void PlayJump();
	void PlayLanding();
	void PlayBlink();
	void PlayGrab();

	void PlaySlide();
	void PlayWallSlide();
	void PlayWallKick();

	// コイン取得、ゲージ増加、コンボ成立時のSE。
	void PlayCoinGet();

	void PlayComboGauge();
	void PlayCombo(int combo);
	void PlayComboLast();
};
