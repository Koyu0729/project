#include "playerSoundController.h"

// プレイヤー用の音声再生ハンドルを作る
void PlayerSoundController::Init()
{
	m_JumpPlayer = AudioManager::CreatePlayer();
	m_LandingPlayer = AudioManager::CreatePlayer();
	m_BlinkPlayer = AudioManager::CreatePlayer();
	m_GrabPlayer = AudioManager::CreatePlayer();
	m_SlidePlayer = AudioManager::CreatePlayer();

	m_CoinPlayer = AudioManager::CreatePlayer();
	m_GaugePlayer = AudioManager::CreatePlayer();
	m_ComboPlayer = AudioManager::CreatePlayer();
	m_ComboLastPlayer = AudioManager::CreatePlayer();
}

// 音声再生ハンドルを解放する
void PlayerSoundController::Uninit()
{
	if (m_JumpPlayer != nullptr)
	{
		AudioManager::DestroyPlayer(m_JumpPlayer);
		m_JumpPlayer = nullptr;
	}

	if (m_LandingPlayer != nullptr)
	{
		AudioManager::DestroyPlayer(m_LandingPlayer);
		m_LandingPlayer = nullptr;
	}

	if (m_BlinkPlayer != nullptr)
	{
		AudioManager::DestroyPlayer(m_BlinkPlayer);
		m_BlinkPlayer = nullptr;
	}

	if (m_GrabPlayer != nullptr)
	{
		AudioManager::DestroyPlayer(m_GrabPlayer);
		m_GrabPlayer = nullptr;
	}

	if (m_SlidePlayer != nullptr)
	{
		AudioManager::DestroyPlayer(m_SlidePlayer);
		m_SlidePlayer = nullptr;
	}

	if (m_CoinPlayer != nullptr)
	{
		AudioManager::DestroyPlayer(m_CoinPlayer);
		m_CoinPlayer = nullptr;
	}

	if (m_GaugePlayer != nullptr)
	{
		AudioManager::DestroyPlayer(m_GaugePlayer);
		m_GaugePlayer = nullptr;
	}

	if (m_ComboPlayer != nullptr)
	{
		AudioManager::DestroyPlayer(m_ComboPlayer);
		m_ComboPlayer = nullptr;
	}

	if (m_ComboLastPlayer != nullptr)
	{
		AudioManager::DestroyPlayer(m_ComboLastPlayer);
		m_ComboLastPlayer = nullptr;
	}
}

// 音の聞こえる位置をプレイヤーに合わせる
void PlayerSoundController::UpdateListenerPosition(const Vector3& position)
{
	AudioManager::Update_3DListener_Position(
		position.x,
		position.y,
		position.z
	);
}

// 共通の設定で効果音を一度鳴らす
void PlayerSoundController::PlayOneShot(
	CriAtomExPlayerHn player,
	const char* cueName,
	Sounds categoryId,
	float volume,
	float pitch
)
{
	if (player == nullptr)
	{
		return;
	}

	if (cueName == nullptr)
	{
		return;
	}

	SePlayParams params;

	params.cueName = cueName;
	params.categoryId = categoryId;
	params.volume = volume;
	params.pitch = pitch;
	params.loopCount = 0;
	params.startTimeMs = 0;
	params.priority = 0;
	params.fadeInMs = 0;
	params.fadeOutMs = 0;
	params.pan = 0.0f;
	params.is3D = false;
	params.pos = { 0.0f, 0.0f, 0.0f };

	AudioManager::PlaySeStart(
		player,
		params,
		nullptr
	);
}

// ジャンプ音を鳴らす
void PlayerSoundController::PlayJump()
{
	PlayOneShot(
		m_JumpPlayer,
		"jump",
		SE,
		0.5f
	);
}

void PlayerSoundController::PlayLanding()
{
	PlayOneShot(
		m_LandingPlayer,
		"landing",
		SE,
		0.5f
	);
}

void PlayerSoundController::PlayWallKick()
{
	// 着地音と相性がよかったため、壁キックにも同じSEを使用する。
	PlayOneShot(
		m_LandingPlayer,
		"landing",
		SE,
		0.5f
	);
}

void PlayerSoundController::PlayBlink()
{
	PlayOneShot(
		m_BlinkPlayer,
		"blink",
		SE,
		0.5f
	);
}

void PlayerSoundController::PlayGrab()
{
	PlayOneShot(
		m_GrabPlayer,
		"grab",
		SE,
		0.5f
	);
}

void PlayerSoundController::PlaySlide()
{
	PlayOneShot(
		m_SlidePlayer,
		"raiding",
		SE,
		0.75f
	);
}

// 壁滑りの効果音を再生する。
void PlayerSoundController::PlayWallSlide()
{
	PlayOneShot(
		m_SlidePlayer,
		"raiding",
		SE,
		0.0f
	);
}

// コイン取得音を鳴らす
void PlayerSoundController::PlayCoinGet()
{
	PlayOneShot(
		m_CoinPlayer,
		"coin",
		SE,
		0.5f
	);
}

void PlayerSoundController::PlayComboGauge()
{
	PlayOneShot(
		m_GaugePlayer,
		"gauge",
		SE,
		0.5f
	);
}

void PlayerSoundController::PlayCombo(int combo)
{
	if (combo < 1)
	{
		combo = 1;
	}

	float pitch = (float)(combo - 1) * 100.0f;

	PlayOneShot(
		m_ComboPlayer,
		"combo",
		SE,
		0.5f,
		pitch
	);
}

// コンボ終了時の効果音を再生する。
void PlayerSoundController::PlayComboLast()
{
	PlayOneShot(
		m_ComboLastPlayer,
		"comboLast",
		SE,
		0.1f
	);
}
