#include "GameData.h"

#include "kosu_s_cri.h"

float GameData::ResultDistance = 0.0f;

// Audio Option
float GameData::MasterVolume = 1.0f;
float GameData::BgmVolume = 0.8f;
float GameData::SeVolume = 0.8f;
float GameData::VoiceVolume = 0.8f;

// Control Option
float GameData::MouseSensitivity = 1.0f;

void GameData::InitOption()
{
	MasterVolume = 1.0f;
	BgmVolume = 0.8f;
	SeVolume = 0.8f;
	VoiceVolume = 0.8f;

	MouseSensitivity = 1.0f;

	ApplyAudioOption();
}

void GameData::SetMasterVolume(float volume)
{
	MasterVolume =
		ClampVolume(
			volume
		);

	ApplyAudioOption();
}

void GameData::SetBgmVolume(float volume)
{
	BgmVolume =
		ClampVolume(
			volume
		);

	ApplyAudioOption();
}

void GameData::SetSeVolume(float volume)
{
	SeVolume =
		ClampVolume(
			volume
		);

	ApplyAudioOption();
}

void GameData::SetVoiceVolume(float volume)
{
	VoiceVolume =
		ClampVolume(
			volume
		);

	ApplyAudioOption();
}

void GameData::SetMouseSensitivity(float sensitivity)
{
	MouseSensitivity =
		ClampMouseSensitivity(
			sensitivity
		);
}

float GameData::ClampVolume(float volume)
{
	if (volume < 0.0f)
	{
		return 0.0f;
	}

	if (volume > 10.0f)
	{
		return 10.0f;
	}

	return volume;
}

float GameData::ClampMouseSensitivity(float sensitivity)
{
	if (sensitivity < 0.1f)
	{
		return 0.1f;
	}

	if (sensitivity > 10.0f)
	{
		return 10.0f;
	}

	return sensitivity;
}

void GameData::ApplyAudioOption()
{
	AudioManager::SetMasterVolume(
		MasterVolume
	);

	AudioManager::SetBgmVolume(
		BgmVolume
	);

	AudioManager::SetSeVolume(
		SeVolume
	);

	AudioManager::SetVoiceVolume(
		VoiceVolume
	);
}