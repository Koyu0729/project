#pragma once
#include "scene.h"

class TextObjectBase;

class LoadingScene : public Scene
{
  private:
	// 読み込み表示と、点のアニメーション用の時間
	TextObjectBase *m_LoadingText = nullptr;
	int m_Frame = 0;

  public:
	bool Init() override;
	void Uninit() override;
	void Update() override;
	void Draw() override;
};