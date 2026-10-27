#include "UISpriteSheetBase.h"

void UISpriteSheetBase::SetSpriteSheet(int divideX, int divideY)
{
	if (divideX <= 0)
	{
		divideX = 1;
	}

	if (divideY <= 0)
	{
		divideY = 1;
	}

	m_DivideX = divideX;
	m_DivideY = divideY;
}

void UISpriteSheetBase::DrawSpriteIndex(
	int index,
	float x,
	float y,
	float w,
	float h
)
{
	DrawSpriteIndex(
		index,
		x,
		y,
		w,
		h,
		m_Color.w
	);
}

void UISpriteSheetBase::DrawSpriteIndex(
	int index,
	float x,
	float y,
	float w,
	float h,
	float alpha
)
{
	DrawSpriteIndex(
		nullptr,
		index,
		m_DivideX,
		m_DivideY,
		x,
		y,
		w,
		h,
		alpha
	);
}

void UISpriteSheetBase::DrawSpriteIndex(
	ID3D11ShaderResourceView* texture,
	int index,
	int divideX,
	int divideY,
	float x,
	float y,
	float w,
	float h
)
{
	DrawSpriteIndex(
		texture,
		index,
		divideX,
		divideY,
		x,
		y,
		w,
		h,
		m_Color.w
	);
}

void UISpriteSheetBase::DrawSpriteIndex(
	ID3D11ShaderResourceView* texture,
	int index,
	int divideX,
	int divideY,
	float x,
	float y,
	float w,
	float h,
	float alpha
)
{
	if (index < 0)
	{
		return;
	}

	if (divideX <= 0 || divideY <= 0)
	{
		return;
	}

	int maxIndex = divideX * divideY;

	if (index >= maxIndex)
	{
		return;
	}

	int ix = index % divideX;
	int iy = index / divideX;

	float tw = 1.0f / (float)divideX;
	float th = 1.0f / (float)divideY;

	float tx = (float)ix * tw;
	float ty = (float)iy * th;

	VERTEX_3D vertex[4]{};

	vertex[0].Position = XMFLOAT3(x, y, 0.0f);
	vertex[1].Position = XMFLOAT3(x + w, y, 0.0f);
	vertex[2].Position = XMFLOAT3(x, y + h, 0.0f);
	vertex[3].Position = XMFLOAT3(x + w, y + h, 0.0f);

	vertex[0].TexCoord = XMFLOAT2(tx, ty);
	vertex[1].TexCoord = XMFLOAT2(tx + tw, ty);
	vertex[2].TexCoord = XMFLOAT2(tx, ty + th);
	vertex[3].TexCoord = XMFLOAT2(tx + tw, ty + th);

	for (int i = 0; i < 4; i++)
	{
		vertex[i].Normal = XMFLOAT3(0.0f, 0.0f, 0.0f);
		vertex[i].Diffuse = XMFLOAT4(
			m_Color.x,
			m_Color.y,
			m_Color.z,
			alpha
		);
	}

	UpdateQuad(vertex);

	if (texture != nullptr)
	{
		Renderer::GetDeviceContext()->PSSetShaderResources(
			0,
			1,
			&texture
		);
	}

	DrawQuad();
}