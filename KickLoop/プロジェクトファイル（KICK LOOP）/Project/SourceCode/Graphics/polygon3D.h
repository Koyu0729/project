#pragma once

#include "UIQuadBase.h"

class Polygon3D : public UIQuadBase
{
private:
	XMFLOAT2 m_Size = XMFLOAT2(1.0f, 1.0f);

	bool m_UseFloating = false;
	float m_FloatBaseY = 0.0f;
	float m_FloatTimer = 0.0f;
	float m_FloatAmplitude = 0.0f;
	float m_FloatSpeed = 0.0f;

	bool m_UseTiltElastic = false;
	float m_TiltAngle = 0.0f;
	float m_TiltVelocity = 0.0f;
	float m_TiltSpring = 0.0f;

	bool m_LookAtPlayer = false;

public:
	void Init() override;
	void Uninit() override;
	void Update() override;
	void Draw() override;

	void SetPosition3D(float x, float y, float z);

	void SetSize(float w, float h);

	void SetRect3D(
		float x,
		float y,
		float z,
		float w,
		float h
	);

	// 度数指定
	void SetRotation(float x, float y, float z);

	// ラジアン指定
	void SetRotationRadian(float x, float y, float z);

	void SetColor(float r, float g, float b, float a);
	void SetAlpha(float alpha);

	void SetFloating(float amplitude, float speed);
	void StopFloating();

	void StartTiltElastic(float power, float spring);
	void StopTiltElastic();

	void SetLookAtPlayer(bool enable);

	XMFLOAT2 GetSize() const { return m_Size; }
	float GetAlpha() const { return m_Color.w; }

private:
	void UpdateLookAtPlayer();
	void UpdateFloating();
	void UpdateTiltElastic();

	void UpdateVertexBuffer();

	float DegreeToRadian(float degree);
};