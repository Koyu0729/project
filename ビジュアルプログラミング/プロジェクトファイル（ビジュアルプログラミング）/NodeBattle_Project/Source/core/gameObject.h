// オブジェクト名と、初期化・更新・描画の共通インターフェース
#pragma once
#include "texture.h"

#include "vector3.h"
#include "vector2.h"

#include "uiHitBox.h"

#include <DirectXMath.h>

using namespace DirectX;

class GameObject
{
  protected:

	std::wstring m_ObjectName = L"NullName";

	bool m_Destroy = false;

  public:
	virtual ~GameObject() = default;
	

	// CPU側のデータを準備する
	virtual void InitCPU() {};

	// 描画に必要なデータを準備する
	virtual void InitGPU() {};

	// 初期化処理を行う
	virtual void Init() {};

	// 終了処理を行う
	virtual void Uninit()
	{
	}

	// 毎フレームの処理
	virtual void Update()
	{
	}

	// 画面に描く処理
	virtual void Draw()
	{
	}

	GameObject *SetName(const std::wstring &name)
	{
		m_ObjectName = name;

		return this;
	}

	const std::wstring &GetName() const
	{
		return m_ObjectName;
	}
};
