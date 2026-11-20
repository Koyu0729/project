#pragma once

#include "gameObject3D.h"

// カメラの場所替えのモード
enum CameraPlaceMode
{
	// デフォルト
	Place_Defalut = -1,

	// 上空
	Place_Front,
	// 上空（位置反転）
	Place_Back,

	// 上空（デフォルト）
	Place_Right,
	// 上空（位置反転）
	Place_Left,

	// プレイヤーの三人称視点
	Place_TPS,
	// プレイヤーの一人称視点
	Place_FPS,

	Place_Free,

	Place_Max
};

class Camera : public GameObject3D
{
  private:
	// 選んだカメラ（1は追従、2は横、3は上から）
	CameraPlaceMode m_CameraMode = Place_Defalut;

	int m_ViewNumber = 3;

	// 画面に映すための行列と、カメラが見る場所
	XMMATRIX m_Projection;

	XMMATRIX m_View;

	Vector3 m_Target = {0.0f, 0.0f, 0.0f};

	float m_ScrollOffset = 0.0f;

  public:
	// 初期化処理を行う
	void Init() override;

	// 終了処理を行う
	void Uninit() override;

	// 毎フレームの処理
	void Update() override;

	// 画面に描く処理
	void Draw() override;

	XMMATRIX GetViewMatrix()
	{
		return m_View;
	}

	// カメラの配置に合わせて、追従・横・俯瞰の表示を選ぶ
	void SetCameraMode(CameraPlaceMode mode)
	{
		m_CameraMode = mode;

		if (mode == Place_TPS || mode == Place_Back)
		{
			m_ViewNumber = 1;
		}
		else if (mode == Place_Right || mode == Place_Left)
		{
			m_ViewNumber = 2;
		}
		else
		{
			m_ViewNumber = 3;
		}
	}

	// カメラ1・2・3を切り替える
	void SetViewNumber(int number)
	{
		if (number >= 1 && number <= 3)
		{
			m_ViewNumber = number;
		}
	}

	int GetViewNumber() const
	{
		return m_ViewNumber;
	}

	Vector3 GetTargetPoint() const
	{
		return m_Target;
	}

	// 今選んでいるカメラの名前を返す
	const wchar_t *GetViewName() const
	{
		if (m_ViewNumber == 1)
		{
			return L"カメラ1：後方追従";
		}
		else if (m_ViewNumber == 2)
		{
			return L"カメラ2：横から";
		}
		else
		{
			return L"カメラ3：俯瞰";
		}
	}
};
