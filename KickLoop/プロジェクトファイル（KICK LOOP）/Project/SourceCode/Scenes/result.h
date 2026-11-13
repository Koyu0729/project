#pragma once

#include "scene.h"

class UI3DButton;

class Result : public Scene
{
private:
	UI3DButton* m_ResultButton = nullptr;
public:
	void Init() override;
	void Update() override;
	void Uninit() override;
	void Draw() override;
};