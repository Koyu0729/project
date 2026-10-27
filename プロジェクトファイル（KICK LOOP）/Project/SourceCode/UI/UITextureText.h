#pragma once

#include "UIQuadBase.h"

class UITextureText : public UIQuadBase
{
private:
	float m_X = 0.0f;
	float m_Y = 0.0f;
	float m_Width = 100.0f;
	float m_Height = 30.0f;

	float m_U0 = 0.0f;
	float m_V0 = 0.0f;
	float m_U1 = 1.0f;
	float m_V1 = 1.0f;

	float m_R = 1.0f;
	float m_G = 1.0f;
	float m_B = 1.0f;
	float m_A = 1.0f;

	const char* m_TexturePath = nullptr;

private:
	void UpdateVertex();

public:
	void Init() override;
	void Uninit() override;
	void Update() override;
	void Draw() override;

	void SetTexturePath(const char* texturePath);
	void SetRect(float x, float y, float width, float height);
	void SetUV(float u0, float v0, float u1, float v1);
	void SetUVPixel(float textureWidth, float textureHeight, float left, float top, float right, float bottom);
	void SetColor(float r, float g, float b, float a);
};