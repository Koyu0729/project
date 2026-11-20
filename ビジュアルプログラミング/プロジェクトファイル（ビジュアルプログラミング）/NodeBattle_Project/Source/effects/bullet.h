#pragma once

#include "gameObject3D.h"

class ModelRenderer;
class Player;

class Bullet : public GameObject3D 
{
  private:
	// カメラに正対する弾の画像
	class BillboardSprite *m_Sprite = nullptr;

	Vector3 m_Velocity = {};

	Vector3 m_StartPosition = {};
	Vector3 m_EndPosition = {};
	Vector3 m_StartVector = {};
	Vector3 m_EndVector = {};
	float m_Time = 0.0f;
	float m_Lifetime = 0.0f;

	Player *m_Owner = nullptr;

  public:
	void Init() override;
	void Uninit() override;
	void Update() override;
	void Draw() override;

	void Shot(Vector3 startposition, Vector3 EndPosition)
	{
		m_StartPosition = startposition;
		m_EndPosition = EndPosition;
		m_StartVector = Vector3(2.0f, 2.0f, 0.0f);
		m_EndVector = Vector3(2.0f, -2.0f, 0.0f);
		m_Time = 0.0f;
	}
	void SetVelocity(Vector3 velocity)
	{
		m_Velocity = velocity;
	}

	Vector3 GetVelocity()
	{
		return m_Velocity;
	}

	void SetOwner(Player *owner)
	{
		m_Owner = owner;
	}
};
