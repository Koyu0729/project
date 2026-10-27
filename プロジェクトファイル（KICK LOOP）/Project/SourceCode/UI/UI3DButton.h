#pragma once

#include "UIQuadBase.h"

class Camera;

class UI3DButton : public UIQuadBase
{
private:
	Vector3 m_Position = Vector3(0.0f, 0.0f, 0.0f);
	Vector3 m_Rotation = Vector3(0.0f, 0.0f, 0.0f);
	Vector3 m_Scale = Vector3(1.0f, 1.0f, 1.0f);

	XMFLOAT2 m_Size = XMFLOAT2(1.0f, 1.0f);

	float m_HitDistance = 100.0f;

	bool m_IsHover = false;
	bool m_IsPressed = false;

	float m_HoverFlashTimer = 0.0f;
	float m_PressFlashTimer = 0.0f;

	float m_HighlightScale = 1.25f;

private:
	void UpdateHit();

	bool IsHitByCameraRay(
		Camera* camera
	);

	XMMATRIX BuildWorldMatrix() const;

	void UpdateVertexBuffer();
	void UpdateVertexBufferWithScale(float scaleRate);

	void DrawHighlight();
	void DrawMainButton();

	float DegreeToRadian(float degree);

public:
	void Init() override;
	void Uninit() override;
	void Update() override;
	void Draw() override;

	void SetPosition(const Vector3& position);

	void SetPosition3D(
		float x,
		float y,
		float z
	);

	void SetSize(
		float width,
		float height
	);

	void SetRect3D(
		float x,
		float y,
		float z,
		float width,
		float height
	);

	void SetRotation(
		float x,
		float y,
		float z
	);

	void SetRotationRadian(
		float x,
		float y,
		float z
	);

	void SetScale(
		float x,
		float y,
		float z
	);

	void SetHitDistance(float distance);

	bool IsHover() const;
	bool IsPressed() const;
};