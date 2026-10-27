#include "main.h"
#include "polygon2D.h"

void Polygon2D::Init()
{
	CreateQuad();

	m_Position = Vector3(0.0f, 0.0f, 0.0f);
	m_Size = XMFLOAT2(100.0f, 100.0f);

	SetTexture("asset\\texture\\kuro.png");

	SetRect(
		m_Position.x,
		m_Position.y,
		m_Size.x,
		m_Size.y
	);
}

void Polygon2D::Uninit()
{
	UIQuadBase::Uninit();
}

void Polygon2D::Update()
{
}

void Polygon2D::Draw()
{
	SetDraw2D();
	DrawQuad();
}

void Polygon2D::SetRect(float x, float y, float w, float h)
{
	m_Position = Vector3(x, y, 0.0f);
	m_Size = XMFLOAT2(w, h);

	VERTEX_3D vertex[4]{};

	vertex[0].Position = XMFLOAT3(x, y, 0.0f);
	vertex[1].Position = XMFLOAT3(x + w, y, 0.0f);
	vertex[2].Position = XMFLOAT3(x, y + h, 0.0f);
	vertex[3].Position = XMFLOAT3(x + w, y + h, 0.0f);

	vertex[0].TexCoord = XMFLOAT2(0.0f, 0.0f);
	vertex[1].TexCoord = XMFLOAT2(1.0f, 0.0f);
	vertex[2].TexCoord = XMFLOAT2(0.0f, 1.0f);
	vertex[3].TexCoord = XMFLOAT2(1.0f, 1.0f);

	for (int i = 0; i < 4; i++)
	{
		vertex[i].Normal = XMFLOAT3(0.0f, 0.0f, 0.0f);
		vertex[i].Diffuse = m_Color;
	}

	UpdateQuad(vertex);
}

void Polygon2D::SetColor(float r, float g, float b, float a)
{
	UIQuadBase::SetColor(r, g, b, a);

	SetRect(
		m_Position.x,
		m_Position.y,
		m_Size.x,
		m_Size.y
	);
}

void Polygon2D::SetAlpha(float alpha)
{
	UIQuadBase::SetAlpha(alpha);

	SetRect(
		m_Position.x,
		m_Position.y,
		m_Size.x,
		m_Size.y
	);
}