#pragma once

#include "scene.h"

class UI3DButton;

class Title : public Scene
{
private:
	UI3DButton* m_StartButton = nullptr;
public:
	void Init() override;
	void Update() override;
	void Uninit() override;
	void Draw() override;
};