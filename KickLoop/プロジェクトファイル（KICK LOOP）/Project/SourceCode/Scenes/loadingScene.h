#pragma once

#include <thread>
#include <mutex>

#include "scene.h"
#include "gameConfigData.h"

class Polygon2D;

class LoadingScene : public Scene
{
private:
	std::thread m_LoadThread;
	std::mutex m_LoadMutex;

	bool m_IsThreadStarted = false;
	bool m_LoadFinished = false;
	bool m_LoadSuccess = false;
	bool m_NextSceneRequested = false;

	GameLoadData m_LoadData;

	Polygon2D* m_Background = nullptr;
	Polygon2D* m_LoadingImage = nullptr;

private:
	void StartLoadThread();

public:
	void Init() override;
	void Uninit() override;
	void Update() override;
	void Draw() override;
};