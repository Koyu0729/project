#pragma once

#include "UISpriteSheetBase.h"

class DistanceCounter : public UISpriteSheetBase
{
private:
	float m_Distance = 0.0f;

private:
	static constexpr int SPRITE_DIVIDE_X = 1;
	static constexpr int SPRITE_DIVIDE_Y = 18;

	static constexpr float DIGIT_WIDTH = 30.0f;
	static constexpr float DIGIT_HEIGHT = 50.0f;
	static constexpr float DIGIT_SPACING = 30.0f;

	static constexpr float DRAW_Y = 50.0f;

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
	static constexpr int INDEX_M = 17;

public:
	void Init() override;
	void Uninit() override;
	void Update() override;
	void Draw() override;

	float GetDistance() const { return m_Distance; }

private:
	void DrawDistance();

	int GetSpriteIndex(char c) const;
};