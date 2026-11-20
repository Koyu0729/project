#pragma once

#include "gameObject3D.h"
#include "cube.h"

// 床と四方の壁を配置する
class Field : public GameObject3D
{
	// 0番は床、1～4番は四方の壁。このFieldが管理する
	Cube m_Cubes[5];

	// 編集画面では隠し、テストと対戦で表示する
	bool m_Visible = true;

  public:
	void Init() override;

	void Uninit() override;

	void Update() override;

	void Draw() override;

	void SetVisible(bool visible)
	{
		m_Visible = visible;
	}

	bool GetVisible() const
	{
		return m_Visible;
	}
};
