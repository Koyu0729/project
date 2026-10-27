#pragma once

#include "scene.h"
#include "gameTime.h"
#include "gameConfigData.h"

#include "kosu_s_cri.h"

class Polygon3D;

class Game : public Scene
{
private:
	Polygon3D* kickUI = nullptr;

	GameLoadData m_LoadData;
	bool m_HasLoadData = false;

	int m_GoalCameraFinishWaitFrame = 0;
	bool m_HasChangedToResult = false;

	// BGM—p
	CriAtomExPlayerHn m_BgmPlayer = nullptr;
	CriAtomExPlaybackId m_BgmPlaybackId = -1;

private:
	void LoadDataIfNeeded();
	void BuildScene();

public:
	void SetLoadData(const GameLoadData& loadData);

	void Init() override;
	void Uninit() override;

	void Update() override;
	void Draw() override;
};