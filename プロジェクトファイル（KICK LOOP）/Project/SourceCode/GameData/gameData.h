#pragma once

class GameData
{
public:
	// ƒŠƒUƒ‹ƒg—p
	static float ResultDistance;

	// Audio Option
	static float MasterVolume;
	static float BgmVolume;
	static float SeVolume;
	static float VoiceVolume;

	// Control Option
	static float MouseSensitivity;

public:
	static void InitOption();

	static void SetMasterVolume(float volume);
	static void SetBgmVolume(float volume);
	static void SetSeVolume(float volume);
	static void SetVoiceVolume(float volume);

	static void SetMouseSensitivity(float sensitivity);

	static float ClampVolume(float volume);
	static float ClampMouseSensitivity(float sensitivity);

	static void ApplyAudioOption();
};