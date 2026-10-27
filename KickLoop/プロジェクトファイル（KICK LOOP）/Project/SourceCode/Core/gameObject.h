#pragma once
#include "texture.h"

class GameObject
{
protected:
	bool m_Destroy = false;

	bool m_Active = true;

	bool m_Visible = true;

	// 座標、回転、スケール
	Vector3 m_Position = { 0.0f, 0.0f, 0.0f };
	Vector3 m_Rotation = { 0.0f, 0.0f, 0.0f };
	Vector3 m_Scale = { 1.0f, 1.0f, 1.0f };

public:
	virtual ~GameObject() {}

	void SetDestroy() { m_Destroy = true; }

	void SetActive(bool active)
	{
		m_Active = active;
	}

	bool Active() const
	{
		return m_Active;
	}

	bool Destroy() const
	{
		return m_Destroy;
	}

	void SetVisible(bool visible)
	{
		m_Visible = visible;
	}

	bool Visible() const
	{
		return m_Visible;
	}

	virtual void Init() {}
	virtual void Uninit() {}
	virtual void Update() {}
	virtual void Draw() {}

	// 座標、回転、スケールのセッターとゲッター
	void SetPosition(Vector3 position) { m_Position = position; }
	void SetRotation(Vector3 rotation) { m_Rotation = rotation; }
	void SetScale(Vector3 scale) { m_Scale = scale; }

	void SetRotationDegree(Vector3 rotation)
	{
		m_Rotation.x = XMConvertToRadians(rotation.x);
		m_Rotation.y = XMConvertToRadians(rotation.y);
		m_Rotation.z = XMConvertToRadians(rotation.z);
	}

	Vector3 GetPosition() const { return m_Position; }
	Vector3 GetRotation() const { return m_Rotation; }
	Vector3 GetScale() const { return m_Scale; }
	Vector3 GetRotationDegree() const
	{
		return Vector3(
			XMConvertToDegrees(m_Rotation.x),
			XMConvertToDegrees(m_Rotation.y),
			XMConvertToDegrees(m_Rotation.z)
		);
	}

	// ワールド行列から右ベクトルを取得する
	Vector3 GetRight() 
	{
		XMMATRIX matrix = XMMatrixRotationRollPitchYaw(
			m_Rotation.x,
			m_Rotation.y,
			m_Rotation.z
		);

		Vector3 right;
		XMStoreFloat3((XMFLOAT3*)&right, matrix.r[0]);

		return right;
	}

	// ワールド行列から前方ベクトルを取得する
	Vector3 GetForward()
	{
		XMMATRIX matrix = XMMatrixRotationRollPitchYaw(
			m_Rotation.x,
			m_Rotation.y,
			m_Rotation.z
		);

		Vector3 forward;
		XMStoreFloat3((XMFLOAT3*)&forward, matrix.r[2]);

		return forward;
	}

	// 指定した位置からの距離を取得する関数
	float GetDistance(Vector3 position)
	{
		return (m_Position - position).length();
	}

	// 指定した位置から見た前方方向への距離を取得する関数
	float GetZ(Vector3 position, Vector3 forward)
	{
		Vector3 direction = m_Position - position;
		return Vector3::dot(direction, forward);
	}
};