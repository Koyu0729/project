#include "main.h"
#include "renderer.h"
#include "fade.h"
#include "texture.h"

void Fade::Init()
{
	const float w = SCREEN_WIDTH;
	const float h = SCREEN_HEIGHT;

	// 頂点バッファの情報を作る
	VERTEX_3D vertex[4];

	vertex[0].Position = XMFLOAT3(0.0f, 0.0f, 0.0f);
	vertex[0].TexCoord = XMFLOAT2(0.0f, 0.0f);
	vertex[0].Diffuse = XMFLOAT4(0, 0, 0, 1);

	vertex[1].Position = XMFLOAT3(w, 0.0f, 0.0f);
	vertex[1].TexCoord = XMFLOAT2(1.0f, 0.0f);
	vertex[1].Diffuse = XMFLOAT4(0, 0, 0, 1);

	vertex[2].Position = XMFLOAT3(0.0f, h, 0.0f);
	vertex[2].TexCoord = XMFLOAT2(0.0f, 1.0f);
	vertex[2].Diffuse = XMFLOAT4(0, 0, 0, 1);

	vertex[3].Position = XMFLOAT3(w, h, 0.0f);
	vertex[3].TexCoord = XMFLOAT2(1.0f, 1.0f);
	vertex[3].Diffuse = XMFLOAT4(0, 0, 0, 1);

	// 頂点バッファ生成
	D3D11_BUFFER_DESC bd{};
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(VERTEX_3D) * 4;
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = 0;

	D3D11_SUBRESOURCE_DATA sd{};
	sd.pSysMem = vertex;

	Renderer::GetDevice()->CreateBuffer(&bd, &sd, &m_VertexBuffer);

	// フェード用の画像も共有キャッシュが所有する
	m_Texture = Texture::Load("asset\\texture\\kuro.png");

	Renderer::CreateVertexShader(&m_VertexShader, &m_VertexLayout,
								 "shader\\unlitTextureVS.cso");
	Renderer::CreatePixelShader(&m_PixelShader,
								"shader\\unlitTexturePS.cso");

	m_Alpha = 0.0f;
	m_State = None;
	m_Finished = true;
}

void Fade::Uninit()
{
	m_Texture = nullptr;
	m_VertexBuffer->Release();
	m_VertexLayout->Release();
	m_VertexShader->Release();
	m_PixelShader->Release();
}

void Fade::Update()
{
	if (m_State == None || m_Finished)
	{
		return;
	}

	m_Timer += 1.0f / 60.0f;
	float t = m_Timer / m_Time;

	if (t >= 1.0f)
	{
		t = 1.0f;
	}

	if (m_State == FadeIn)
	{
		m_Alpha = 1.0f - t;
		if (t >= 1.0f)
		{
			m_Finished = true;
			m_State = None;
		}
	}
	else if (m_State == FadeOut)
	{
		m_Alpha = t;
		if (t >= 1.0f)
		{
			m_Finished = true;
		}
	}
}

void Fade::Draw()
{
	if (m_Alpha <= 0.0f)
	{
		return;
	}

	Renderer::GetDeviceContext()->IASetInputLayout(m_VertexLayout);
	Renderer::GetDeviceContext()->VSSetShader(m_VertexShader, NULL, 0);
	Renderer::GetDeviceContext()->PSSetShader(m_PixelShader, NULL, 0);

	Renderer::SetWorldViewProjection2D();

	MATERIAL material{};
	material.Diffuse = {0.0f, 0.0f, 0.0f, m_Alpha}; // フェードの透明度を反映する
	material.TextureEnable = m_Texture != nullptr;
	Renderer::SetMaterial(material);

	UINT stride = sizeof(VERTEX_3D);
	UINT offset = 0;
	Renderer::GetDeviceContext()->IASetVertexBuffers(0, 1, &m_VertexBuffer, &stride, &offset);

	Renderer::GetDeviceContext()->PSSetShaderResources(0, 1, &m_Texture);

	Renderer::GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	Renderer::GetDeviceContext()->Draw(4, 0);
}

void Fade::StartFadeIn(float time)
{
	m_State = FadeIn;
	m_Time = time;
	m_Timer = 0.0f;
	m_Finished = false;
	m_Alpha = 1.0f;
}

void Fade::StartFadeOut(float time)
{
	m_State = FadeOut;
	m_Time = time;
	m_Timer = 0.0f;
	m_Finished = false;
	m_Alpha = 0.0f;
}
