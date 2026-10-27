#include "main.h"
#include "loadPercent.h"

#include <cmath>

void LoadPercent::Init()
{
	CreateQuad();

	SetTexture("asset\\texture\\kuro.png");

	m_Angle = 0.0f;

	m_Width = 200.0f;
	m_Height = 200.0f;

	m_X = SCREEN_WIDTH * 0.9f;
	m_Y = SCREEN_HEIGHT * 0.85f;

	SetVisible(true);

	UpdateRotationQuad();
}

void LoadPercent::Uninit()
{
	UIQuadBase::Uninit();
}

void LoadPercent::Update()
{
	m_Angle += 2.0f;

	if (m_Angle >= 360.0f)
	{
		m_Angle -= 360.0f;
	}

	UpdateRotationQuad();
}

void LoadPercent::Draw()
{
	SetDraw2D();
	DrawQuad();
}

void LoadPercent::UpdateRotationQuad()
{
	float rad = m_Angle * (XM_PI / 180.0f);

	float halfW = m_Width * 0.5f;
	float halfH = m_Height * 0.5f;

	XMFLOAT2 local[4] =
	{
		XMFLOAT2(-halfW, -halfH),
		XMFLOAT2(halfW, -halfH),
		XMFLOAT2(-halfW,  halfH),
		XMFLOAT2(halfW,  halfH)
	};

	float c = cosf(rad);
	float s = sinf(rad);

	VERTEX_3D vertex[4]{};

	for (int i = 0; i < 4; i++)
	{
		float rx = local[i].x * c - local[i].y * s;
		float ry = local[i].x * s + local[i].y * c;

		vertex[i].Position = XMFLOAT3(
			m_X + rx,
			m_Y + ry,
			0.0f
		);

		vertex[i].Normal = XMFLOAT3(0.0f, 0.0f, 0.0f);
		vertex[i].Diffuse = m_Color;
	}

	vertex[0].TexCoord = XMFLOAT2(0.0f, 0.0f);
	vertex[1].TexCoord = XMFLOAT2(1.0f, 0.0f);
	vertex[2].TexCoord = XMFLOAT2(0.0f, 1.0f);
	vertex[3].TexCoord = XMFLOAT2(1.0f, 1.0f);

	UpdateQuad(vertex);
}