#pragma once

#include "UISpriteSheetBase.h"

class ComboCounterUI : public UISpriteSheetBase
{
private:
	ID3D11ShaderResourceView* m_NumberTexture = nullptr;
	ID3D11ShaderResourceView* m_TextTexture = nullptr;

	int m_Combo = 0;
	int m_OldCombo = 0;

	float m_UIScale = 1.0f;
	float m_PopPower = 0.0f;

	float m_BaseX = 95.0f;
	float m_BaseY = 105.0f;

private:
	static constexpr int NUMBER_ROW_COUNT = 10;

	static constexpr int NUMBER_INDEX_0 = 0;
	static constexpr int NUMBER_INDEX_1 = 1;
	static constexpr int NUMBER_INDEX_2 = 2;
	static constexpr int NUMBER_INDEX_3 = 3;
	static constexpr int NUMBER_INDEX_4 = 4;
	static constexpr int NUMBER_INDEX_5 = 5;
	static constexpr int NUMBER_INDEX_6 = 6;
	static constexpr int NUMBER_INDEX_7 = 7;
	static constexpr int NUMBER_INDEX_8 = 8;
	static constexpr int NUMBER_INDEX_9 = 9;

	static constexpr int TEXT_ROW_COUNT = 6;

	static constexpr int TEXT_INDEX_X = 0;
	static constexpr int TEXT_INDEX_C = 1;
	static constexpr int TEXT_INDEX_O = 2;
	static constexpr int TEXT_INDEX_M = 3;
	static constexpr int TEXT_INDEX_B = 4;
	static constexpr int TEXT_INDEX_O_LAST = 5;

public:
	void Init() override;
	void Uninit() override;
	void Update() override;
	void Draw() override;

	void SetCombo(int combo);
	int GetCombo() const;

	void SetBasePosition(float x, float y);

private:
	void DrawComboNumber(float x, float y);
	void DrawComboText(float x, float y);

	void DrawNumberDigit(
		int index,
		float x,
		float y,
		float width,
		float height
	);

	void DrawTextDigit(
		int index,
		float x,
		float y,
		float width,
		float height
	);

	int GetNumberIndex(char number);
};