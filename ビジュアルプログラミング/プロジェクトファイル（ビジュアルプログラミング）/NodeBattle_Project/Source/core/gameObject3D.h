// 3Dオブジェクトの位置・向き・大きさを管理する
#pragma once
#include "texture.h"

#include "vector3.h"
#include "vector2.h"

#include "uiHitBox.h"

#include <DirectXMath.h>

using namespace DirectX;

#include "gameObject.h"

class GameObject3D : public GameObject
{
  protected:
	Vector3 m_Position = {0.0f, 0.0f, 0.0f};

	Vector3 m_Rotation = {0.0f, 0.0f, 0.0f};

	Vector3 m_Scale = {1.0f, 1.0f, 1.0f};

  public:

	bool Destroy() const
	{
		return m_Destroy;
	}
	// オブジェクトを消す予約をする

	void SetDestroy()
	{
		m_Destroy = true;
	};

	void SetPosition(Vector3 position)
	{
		m_Position = position;
	}

	void SetRotation(Vector3 rotation)
	{
		m_Rotation = rotation;
	}

	void SetScale(Vector3 scale)
	{
		m_Scale = scale;
	}

	Vector3 GetPosition()
	{
		return m_Position;
	}

	Vector3 GetRotation()
	{
		return m_Rotation;
	}

	Vector3 GetScale()
	{
		return m_Scale;
	}

	Vector3 GetRight()
	{
		XMMATRIX matrix;

		matrix = XMMatrixRotationRollPitchYaw(m_Rotation.x, m_Rotation.y, m_Rotation.z);

		Vector3 right;

		XMStoreFloat3((XMFLOAT3 *)&right, matrix.r[0]);

		return right;
	}

	Vector3 GetForward()
	{
		XMMATRIX matrix;

		matrix = XMMatrixRotationRollPitchYaw(m_Rotation.x, m_Rotation.y, m_Rotation.z);

		Vector3 forward;

		XMStoreFloat3((XMFLOAT3 *)&forward, matrix.r[2]);

		return forward;
	}

	float GetDistance(Vector3 position)
	{
		return (m_Position - position).length();
	}

	virtual float GetZ(Vector3 Position, Vector3 Forward)
	{
		Vector3 direction = m_Position - Position;

		return Vector3::dot(direction, Forward);
	}
};
