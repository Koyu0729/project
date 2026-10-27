#pragma once

#include "UISpriteSheetBase.h"
#include "kosu_s_cri.h"

class ResultTime : public UISpriteSheetBase
{
private:
	bool m_PlayedStartSE = false;

	float m_RotateWaveTimer = 0.0f;
	float m_RotateSpeed = 8.0f;
	float m_NextStartAngle = 1.5708f;

	bool m_IsRotateWait = false;
	float m_RotateWaitTimer = 0.0f;
	float m_RotateWaitTime = 3.0f;

private:
	static constexpr int SPRITE_DIVIDE_X = 1;
	static constexpr int SPRITE_DIVIDE_Y = 18;

	static constexpr int SCORE_TEXT_COUNT = 6;

	static constexpr int INDEX_1 = 0;
	static constexpr int INDEX_2 = 1;
	static constexpr int INDEX_3 = 2;
	static constexpr int INDEX_4 = 3;
	static constexpr int INDEX_5 = 4;
	static constexpr int INDEX_6 = 5;
	static constexpr int INDEX_7 = 6;
	static constexpr int INDEX_8 = 7;
	static constexpr int INDEX_9 = 8;
	static constexpr int INDEX_0 = 9;
	static constexpr int INDEX_DOT = 10;

	static constexpr int INDEX_S = 11;
	static constexpr int INDEX_C = 12;
	static constexpr int INDEX_O = 13;
	static constexpr int INDEX_R = 14;
	static constexpr int INDEX_E = 15;
	static constexpr int INDEX_COLON = 16;
	static constexpr int INDEX_M = 17;

public:
	void Init() override;
	void Uninit() override;
	void Update() override;
	void Draw() override;

private:
	void DrawDigitWorld(
		int index,
		Vector3 position,
		float size,
		float angleY
	);

	int GetSpriteIndex(char c) const;
};