#include "button.h"

#include "main.h"

#include "manager.h"

#include "scene.h"

#include "renderer.h"

#include "texture.h"

#include "textObjectBase.h"

// 初期化処理を行う
void Button::Init()

{

	VERTEX_3D vertex[4];

	vertex[0].Position = XMFLOAT3(0.0f, 0.0f, 0.0f);

	vertex[0].Normal = XMFLOAT3(0.0f, 0.0f, 0.0f);

	vertex[0].Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);

	vertex[0].TexCoord = XMFLOAT2(0.0f, 0.0f);

	vertex[1].Position = XMFLOAT3(1.0f, 0.0f, 0.0f);

	vertex[1].Normal = XMFLOAT3(0.0f, 0.0f, 0.0f);

	vertex[1].Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);

	vertex[1].TexCoord = XMFLOAT2(1.0f, 0.0f);

	vertex[2].Position = XMFLOAT3(0.0f, 1.0f, 0.0f);

	vertex[2].Normal = XMFLOAT3(0.0f, 0.0f, 0.0f);

	vertex[2].Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);

	vertex[2].TexCoord = XMFLOAT2(0.0f, 1.0f);

	vertex[3].Position = XMFLOAT3(1.0f, 1.0f, 0.0f);

	vertex[3].Normal = XMFLOAT3(0.0f, 0.0f, 0.0f);

	vertex[3].Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);

	vertex[3].TexCoord = XMFLOAT2(1.0f, 1.0f);

	// 頂点バッファ生成

	D3D11_BUFFER_DESC bd{};

	bd.Usage = D3D11_USAGE_DEFAULT;

	bd.ByteWidth = sizeof(VERTEX_3D) * 4;

	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;

	bd.CPUAccessFlags = 0;

	D3D11_SUBRESOURCE_DATA sd{};

	sd.pSysMem = vertex;

	Renderer::GetDevice()->CreateBuffer(&bd, &sd, &m_VertexBuffer);

	m_Texture = Texture::Load("asset\\texture\\test.png");

	// シェーダー読み込み

	Renderer::CreateVertexShader(&m_VertexShader, &m_VertexLayout, "shader\\unlitTextureVS.cso");

	Renderer::CreatePixelShader(&m_PixelShader, "shader\\unlitTexturePS.cso");

	m_Position.x = 0.0f;

	m_Position.y = 0.0f;

	m_Scale.x = 200.0f;

	m_Scale.y = 100.0f;

	// 文字初期設定

	m_Font.font = L"メイリオ";

	m_Font.fontSize = 32.0f;

	m_Font.Color = D2D1::ColorF(D2D1::ColorF::Black);

	m_Font.shadowOffset = D2D1::Point2F(2, 2);

	// 効果音

	// 当たり判定追加

	AddHitBox(true, true);

	m_Hit->SetBox(m_Position, m_Scale);
}

// 終了処理を行う
void Button::Uninit()

{

	m_VertexBuffer->Release();

	m_VertexLayout->Release();

	m_VertexShader->Release();

	m_PixelShader->Release();
}

// 毎フレームの処理
void Button::Update()

{

	// ボタンの位置に合わせて当たり判定を更新する

	m_Hit->SetBox(m_Position, m_Scale);

}

// 画面に描く処理
void Button::Draw()

{

	// 入力レイアウト設定

	Renderer::GetDeviceContext()->IASetInputLayout(m_VertexLayout);

	// シェーダ設定

	Renderer::GetDeviceContext()->VSSetShader(m_VertexShader, NULL, 0);

	Renderer::GetDeviceContext()->PSSetShader(m_PixelShader, NULL, 0);

	// マトリクス設定

	Renderer::SetWorldViewProjection2D();

	// マトリクスの追加設定

	XMMATRIX world, scale, rot, trans;

	scale = XMMatrixScaling(m_Scale.x, m_Scale.y, 1.0f);

	trans = XMMatrixTranslation(m_Position.x, m_Position.y, 0.0f);

	world = scale * trans;

	Renderer::SetWorldMatrix(world);

	// マテリアル設定

	MATERIAL material{};

	material.Diffuse = m_Color;

	material.TextureEnable = true;

	Renderer::SetMaterial(material);

	// 頂点バッファ設定

	UINT stride = sizeof(VERTEX_3D);

	UINT offset = 0;

	Renderer::GetDeviceContext()->IASetVertexBuffers(0, 1, &m_VertexBuffer, &stride, &offset);

	// テクスチャ設定

	Renderer::GetDeviceContext()->PSSetShaderResources(0, 1, &m_Texture);

	// プリミティブトポロジ設定

	Renderer::GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	// ポリゴン描画

	Renderer::GetDeviceContext()->Draw(4, 0);

	// 文字

	Vector2 buttonPos = GetPosition();

	Vector2 buttonScale = GetScale();

	D2D1_POINT_2F textPos = D2D1::Point2F(

		buttonPos.x + buttonScale.x * 0.2f, // ボタン左上から横方向へ配置

		buttonPos.y + buttonScale.y * 0.3f // ボタン左上から縦方向へ配置

	);

	float nameLength = (float)GetName().size();

	m_Font.fontSize = 32.0f * (1.0f - (nameLength * 0.05f));

	if (m_Font.fontSize < 24.0f)
	{
		m_Font.fontSize = 24.0f; // 最小サイズ制限
	}

	if (m_Font.fontSize > 48.0f)
	{
		m_Font.fontSize = 48.0f;
	}

	if (m_CaptionSize > 0)
	{
		m_Font.fontSize = m_CaptionSize;

		m_Font.Color = D2D1::ColorF(D2D1::ColorF::White);

		textPos = D2D1::Point2F(buttonPos.x + 12, buttonPos.y + 8);
	}

	Moji::Instance().SetFont(m_Font);

	Moji::Instance().Draw(

		GetName(),

		textPos,

		1.0f,

		D2D1_DRAW_TEXT_OPTIONS_NONE,

		false

	);
}

Button *Button::SetTexRule(

	Vector2 position,

	float rotation,

	Vector2 scale,

	const char *FileName)

{

	m_Position = position;

	m_Rotation = rotation;

	m_Scale = scale;

	m_Texture = Texture::Load(FileName);

	return this;
}
