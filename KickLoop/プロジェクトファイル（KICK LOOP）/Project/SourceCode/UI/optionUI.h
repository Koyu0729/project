#pragma once

#include <array>

#include "UIQuadBase.h"
#include "UISlider.h"
#include "UITextureText.h"

class OptionUI : public UIQuadBase
{
private:
	struct SliderRow
	{
		UISlider slider;
		UITextureText label;
	};

	static constexpr int SliderCount = 5;
	static constexpr int ButtonCount = 3;

	std::array<SliderRow, SliderCount> m_Sliders;
	std::array<UITextureText, ButtonCount> m_ButtonLabels;

	bool m_IsOpen = false;
	int m_SelectedIndex = 0;
	int m_DraggingSlider = -1;
	bool m_PrevLeftMouseDown = false;

	void CreateParts();
	void LoadSliderValues();
	void ApplySliderValue(int index);
	void UpdateKeyboard();
	void UpdateMouse();
	void ExecuteButton(int index);

public:
	void Init() override;
	void Uninit() override;
	void Update() override;
	void Draw() override;

	void Open();
	void Close();
	void Toggle();

	bool IsOpen() const;
};
