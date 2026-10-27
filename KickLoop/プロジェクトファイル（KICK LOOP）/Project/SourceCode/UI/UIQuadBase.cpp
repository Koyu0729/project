#include "UIQuadBase.h"
#include "texture.h"

#include <cstring>

void UIQuadBase::CreateQuad()
{
	if (m_VertexBuffer != nullptr)
	{
		return;
	}

	VERTEX_3D vertex[4]{};

	vertex[0].Position = XMFLOAT3(0, 0, 0);
	vertex[0].Normal = XMFLOAT3(0, 0, 0);
	vertex[0].Diffuse = XMFLOAT4(1, 1, 1, 1);
	vertex[0].TexCoord = XMFLOAT2(0, 0);

	vertex[1].Position = XMFLOAT3(1, 0, 0);
	vertex[1].Normal = XMFLOAT3(0, 0, 0);
	vertex[1].Diffuse = XMFLOAT4(1, 1, 1, 1);
	vertex[1].TexCoord = XMFLOAT2(1, 0);

	vertex[2].Position = XMFLOAT3(0, 1, 0);
	vertex[2].Normal = XMFLOAT3(0, 0, 0);
	vertex[2].Diffuse = XMFLOAT4(1, 1, 1, 1);
	vertex[2].TexCoord = XMFLOAT2(0, 1);

	vertex[3].Position = XMFLOAT3(1, 1, 0);
	vertex[3].Normal = XMFLOAT3(0, 0, 0);
	vertex[3].Diffuse = XMFLOAT4(1, 1, 1, 1);
	vertex[3].TexCoord = XMFLOAT2(1, 1);

	D3D11_BUFFER_DESC bd{};
	bd.Usage = D3D11_USAGE_DYNAMIC;
	bd.ByteWidth = sizeof(VERTEX_3D) * 4;
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	D3D11_SUBRESOURCE_DATA sd{};
	sd.pSysMem = vertex;

	Renderer::GetDevice()->CreateBuffer(&bd, &sd, &m_VertexBuffer);

	m_Texture = Texture::Load("asset\\texture\\kuro.png");

	Renderer::CreateVertexShader(
		&m_VertexShader,
		&m_VertexLayout,
		"shader\\unlitTextureVS.cso"
	);

	Renderer::CreatePixelShader(
		&m_PixelShader,
		"shader\\unlitTexturePS.cso"
	);
}

void UIQuadBase::Uninit()
{
	m_Texture = nullptr;

	if (m_VertexBuffer)
	{
		m_VertexBuffer->Release();
		m_VertexBuffer = nullptr;
	}

	if (m_VertexLayout)
	{
		m_VertexLayout->Release();
		m_VertexLayout = nullptr;
	}

	if (m_VertexShader)
	{
		m_VertexShader->Release();
		m_VertexShader = nullptr;
	}

	if (m_PixelShader)
	{
		m_PixelShader->Release();
		m_PixelShader = nullptr;
	}
}

void UIQuadBase::SetTexture(const char* path)
{
	if (path == nullptr)
	{
		return;
	}

	m_Texture = Texture::Load(path);
}

void UIQuadBase::SetColor(float r, float g, float b, float a)
{
	m_Color = XMFLOAT4(r, g, b, a);
}

void UIQuadBase::SetAlpha(float alpha)
{
	m_Color.w = alpha;
}

void UIQuadBase::UpdateQuad(const VERTEX_3D vertex[4])
{
	if (m_VertexBuffer == nullptr)
	{
		return;
	}

	D3D11_MAPPED_SUBRESOURCE msr{};

	if (FAILED(Renderer::GetDeviceContext()->Map(
		m_VertexBuffer,
		0,
		D3D11_MAP_WRITE_DISCARD,
		0,
		&msr
	)))
	{
		return;
	}

	memcpy(msr.pData, vertex, sizeof(VERTEX_3D) * 4);

	Renderer::GetDeviceContext()->Unmap(m_VertexBuffer, 0);
}

void UIQuadBase::SetDraw2D()
{
	Renderer::GetDeviceContext()->IASetInputLayout(m_VertexLayout);
	Renderer::GetDeviceContext()->VSSetShader(m_VertexShader, nullptr, 0);
	Renderer::GetDeviceContext()->PSSetShader(m_PixelShader, nullptr, 0);

	Renderer::SetWorldViewProjection2D();

	MATERIAL material{};
	material.Diffuse = m_Color;
	material.TextureEnable = true;
	Renderer::SetMaterial(material);

	UINT stride = sizeof(VERTEX_3D);
	UINT offset = 0;

	Renderer::GetDeviceContext()->IASetVertexBuffers(
		0,
		1,
		&m_VertexBuffer,
		&stride,
		&offset
	);

	if (m_Texture != nullptr)
	{
		Renderer::GetDeviceContext()->PSSetShaderResources(
			0,
			1,
			&m_Texture
		);
	}

	Renderer::GetDeviceContext()->IASetPrimitiveTopology(
		D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP
	);
}

void UIQuadBase::SetDraw3D()
{
	Renderer::GetDeviceContext()->IASetInputLayout(m_VertexLayout);
	Renderer::GetDeviceContext()->VSSetShader(m_VertexShader, nullptr, 0);
	Renderer::GetDeviceContext()->PSSetShader(m_PixelShader, nullptr, 0);

	MATERIAL material{};
	material.Diffuse = m_Color;
	material.TextureEnable = true;
	Renderer::SetMaterial(material);

	UINT stride = sizeof(VERTEX_3D);
	UINT offset = 0;

	Renderer::GetDeviceContext()->IASetVertexBuffers(
		0,
		1,
		&m_VertexBuffer,
		&stride,
		&offset
	);

	if (m_Texture != nullptr)
	{
		Renderer::GetDeviceContext()->PSSetShaderResources(
			0,
			1,
			&m_Texture
		);
	}

	Renderer::GetDeviceContext()->IASetPrimitiveTopology(
		D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP
	);
}

void UIQuadBase::DrawQuad()
{
	Renderer::GetDeviceContext()->Draw(4, 0);
}