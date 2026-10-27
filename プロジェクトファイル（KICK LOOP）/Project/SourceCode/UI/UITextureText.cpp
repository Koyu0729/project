#include "UITextureText.h"

void UITextureText::Init()
{
	CreateQuad();

	m_X = 0.0f;
	m_Y = 0.0f;
	m_Width = 100.0f;
	m_Height = 30.0f;

	m_U0 = 0.0f;
	m_V0 = 0.0f;
	m_U1 = 1.0f;
	m_V1 = 1.0f;

	m_R = 1.0f;
	m_G = 1.0f;
	m_B = 1.0f;
	m_A = 1.0f;

	m_TexturePath = nullptr;

	SetActive(true);
	SetVisible(true);

	UpdateVertex();
}

void UITextureText::Uninit()
{
	UIQuadBase::Uninit();
}

void UITextureText::Update()
{
}

void UITextureText::Draw()
{
	if (m_TexturePath == nullptr)
	{
		return;
	}

	SetDraw2D();

	SetTexture(m_TexturePath);

	UIQuadBase::SetColor(m_R, m_G, m_B, m_A);

	UpdateVertex();

	DrawQuad();
}

void UITextureText::SetTexturePath(const char* texturePath)
{
	m_TexturePath = texturePath;
}

void UITextureText::SetRect(float x, float y, float width, float height)
{
	m_X = x;
	m_Y = y;
	m_Width = width;
	m_Height = height;

	UpdateVertex();
}

void UITextureText::SetUV(float u0, float v0, float u1, float v1)
{
	m_U0 = u0;
	m_V0 = v0;
	m_U1 = u1;
	m_V1 = v1;

	UpdateVertex();
}

void UITextureText::SetUVPixel(float textureWidth, float textureHeight, float left, float top, float right, float bottom)
{
	m_U0 = left / textureWidth;
	m_V0 = top / textureHeight;
	m_U1 = right / textureWidth;
	m_V1 = bottom / textureHeight;

	UpdateVertex();
}

void UITextureText::SetColor(float r, float g, float b, float a)
{
	m_R = r;
	m_G = g;
	m_B = b;
	m_A = a;
}

void UITextureText::UpdateVertex()
{
	VERTEX_3D vertex[4]{};

	vertex[0].Position = XMFLOAT3(m_X, m_Y, 0.0f);
	vertex[0].Normal = XMFLOAT3(0.0f, 0.0f, 0.0f);
	vertex[0].Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	vertex[0].TexCoord = XMFLOAT2(m_U0, m_V0);

	vertex[1].Position = XMFLOAT3(m_X + m_Width, m_Y, 0.0f);
	vertex[1].Normal = XMFLOAT3(0.0f, 0.0f, 0.0f);
	vertex[1].Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	vertex[1].TexCoord = XMFLOAT2(m_U1, m_V0);

	vertex[2].Position = XMFLOAT3(m_X, m_Y + m_Height, 0.0f);
	vertex[2].Normal = XMFLOAT3(0.0f, 0.0f, 0.0f);
	vertex[2].Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	vertex[2].TexCoord = XMFLOAT2(m_U0, m_V1);

	vertex[3].Position = XMFLOAT3(m_X + m_Width, m_Y + m_Height, 0.0f);
	vertex[3].Normal = XMFLOAT3(0.0f, 0.0f, 0.0f);
	vertex[3].Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	vertex[3].TexCoord = XMFLOAT2(m_U1, m_V1);

	UpdateQuad(vertex);
}