#include "main.h"
#include "drawLine.h"
#include "renderer.h"

void DrawLine::Init()
{
	VERTEX_3D vertex[53] = {};

	// 頂点バッファ生成
	D3D11_BUFFER_DESC bd{};
	bd.Usage = D3D11_USAGE_DYNAMIC;
	bd.ByteWidth = sizeof(VERTEX_3D) * 53;
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	D3D11_SUBRESOURCE_DATA sd{};
	sd.pSysMem = vertex;

	Renderer::GetDevice()->CreateBuffer(&bd, &sd, &m_VertexBuffer);

	Renderer::CreateVertexShader(&m_VertexShader, &m_VertexLayout, "shader\\unlitTextureVS.cso");
	Renderer::CreatePixelShader(&m_PixelShader, "shader\\unlitTexturePS.cso");
}

void DrawLine::Uninit()
{
	m_VertexBuffer->Release();

	m_VertexLayout->Release();
	m_VertexShader->Release();
	m_PixelShader->Release();
}

void DrawLine::Update()
{
	if (!m_VertexBuffer) return;
	int loop = 0;
	Vector2 pos;

	D3D11_MAPPED_SUBRESOURCE msr;
	Renderer::GetDeviceContext()->Map(m_VertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &msr);

	VERTEX_3D* vertex = (VERTEX_3D*)msr.pData;

	for (float i = 0; i < 50; i++)
	{
		float t = i / float(50 - 1);

		pos.x = (1.0f - t) * (1.0f - t) * (1.0f - t) * m_StartPoint.x
			+ 3.0f * (1.0f - t) * (1.0f - t) * t * m_StartControl.x
			+ 3.0f * (1.0f - t) * t * t * m_EndControl.x
			+ t * t * t * m_EndPoint.x;

		pos.y = (1.0f - t) * (1.0f - t) * (1.0f - t) * m_StartPoint.y
			+ 3.0f * (1.0f - t) * (1.0f - t) * t * m_StartControl.y
			+ 3.0f * (1.0f - t) * t * t * m_EndControl.y
			+ t * t * t * m_EndPoint.y;

		vertex[loop].Position = XMFLOAT3(pos.x, pos.y, 0.0f);
		vertex[loop].Diffuse = XMFLOAT4(1,1,1,1);
		loop++;
	}

    const float tipX = m_EndPoint.x - 8.0f, tipY = m_EndPoint.y;
    vertex[50].Position = XMFLOAT3(tipX,tipY,0);
    vertex[51].Position = XMFLOAT3(tipX-9,tipY-4,0);
    vertex[52].Position = XMFLOAT3(tipX-9,tipY+4,0);
    for (int i=50; i<53; ++i) vertex[i].Diffuse = XMFLOAT4(1,1,1,1);
	Renderer::GetDeviceContext()->Unmap(m_VertexBuffer, 0);
}

void DrawLine::Draw()
{
    Update();
	// 入力レイアウト設定
	Renderer::GetDeviceContext()->IASetInputLayout(m_VertexLayout);
	// シェーダ設定
	Renderer::GetDeviceContext()->VSSetShader(m_VertexShader, NULL, 0);
	Renderer::GetDeviceContext()->PSSetShader(m_PixelShader, NULL, 0);
	// マトリクス設定
	Renderer::SetWorldViewProjection2D();

	// マテリアル設定
	MATERIAL material{};
	material.Diffuse = m_Color;
	material.TextureEnable = false;
	Renderer::SetMaterial(material);

	// 頂点バッファ設定
	UINT stride = sizeof(VERTEX_3D);
	UINT offset = 0;
	Renderer::GetDeviceContext()->IASetVertexBuffers(0, 1, &m_VertexBuffer, &stride, &offset);

	// プリミティブトポロジ設定
	Renderer::GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP);

	// ポリゴン描画
	Renderer::GetDeviceContext()->Draw(50, 0);
    Renderer::GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    Renderer::GetDeviceContext()->Draw(3, 50);
}

void DrawLine::SetPoints(const Vector2& start, const Vector2& end)
{
    m_StartPoint = start;
    const float controlOffset = (std::max)(40.0f, fabsf(end.x - start.x) * 0.5f);
    m_StartControl = Vector2(start.x + controlOffset, start.y);
    m_EndControl = Vector2(end.x - controlOffset, end.y);
    m_EndPoint = end;
}

void DrawLine::SetColor(const XMFLOAT4& color)
{
    m_Color = color;
}

