#include "main.h"
#include "cube.h"
#include "renderer.h"
#include "texture.h"

// 中心から各方向へ1の大きさのCubeを作る
void Cube::Init()
{
	const float repeatX = m_TextureRepeat.x;
	const float repeatY = m_TextureRepeat.y;
	const float repeatZ = m_TextureRepeat.z;

	// 各面を2枚の三角形で描くための36頂点
	VERTEX_3D vertices[36] =
	{
		// 上面
		{{-1, 1, 1}, {0, 1, 0}, {1, 1, 1, 1}, {0, 0}},
		{{1, 1, 1}, {0, 1, 0}, {1, 1, 1, 1}, {repeatX, 0}},
		{{-1, 1, -1}, {0, 1, 0}, {1, 1, 1, 1}, {0, repeatZ}},
		{{-1, 1, -1}, {0, 1, 0}, {1, 1, 1, 1}, {0, repeatZ}},
		{{1, 1, 1}, {0, 1, 0}, {1, 1, 1, 1}, {repeatX, 0}},
		{{1, 1, -1}, {0, 1, 0}, {1, 1, 1, 1}, {repeatX, repeatZ}},

		// 下面
		{{-1, -1, -1}, {0, -1, 0}, {1, 1, 1, 1}, {0, 0}},
		{{1, -1, -1}, {0, -1, 0}, {1, 1, 1, 1}, {repeatX, 0}},
		{{-1, -1, 1}, {0, -1, 0}, {1, 1, 1, 1}, {0, repeatZ}},
		{{-1, -1, 1}, {0, -1, 0}, {1, 1, 1, 1}, {0, repeatZ}},
		{{1, -1, -1}, {0, -1, 0}, {1, 1, 1, 1}, {repeatX, 0}},
		{{1, -1, 1}, {0, -1, 0}, {1, 1, 1, 1}, {repeatX, repeatZ}},

		// 手前
		{{-1, 1, -1}, {0, 0, -1}, {1, 1, 1, 1}, {0, 0}},
		{{1, 1, -1}, {0, 0, -1}, {1, 1, 1, 1}, {repeatX, 0}},
		{{-1, -1, -1}, {0, 0, -1}, {1, 1, 1, 1}, {0, repeatY}},
		{{-1, -1, -1}, {0, 0, -1}, {1, 1, 1, 1}, {0, repeatY}},
		{{1, 1, -1}, {0, 0, -1}, {1, 1, 1, 1}, {repeatX, 0}},
		{{1, -1, -1}, {0, 0, -1}, {1, 1, 1, 1}, {repeatX, repeatY}},

		// 奥
		{{1, 1, 1}, {0, 0, 1}, {1, 1, 1, 1}, {0, 0}},
		{{-1, 1, 1}, {0, 0, 1}, {1, 1, 1, 1}, {repeatX, 0}},
		{{1, -1, 1}, {0, 0, 1}, {1, 1, 1, 1}, {0, repeatY}},
		{{1, -1, 1}, {0, 0, 1}, {1, 1, 1, 1}, {0, repeatY}},
		{{-1, 1, 1}, {0, 0, 1}, {1, 1, 1, 1}, {repeatX, 0}},
		{{-1, -1, 1}, {0, 0, 1}, {1, 1, 1, 1}, {repeatX, repeatY}},

		// 左
		{{-1, 1, 1}, {-1, 0, 0}, {1, 1, 1, 1}, {0, 0}},
		{{-1, 1, -1}, {-1, 0, 0}, {1, 1, 1, 1}, {repeatZ, 0}},
		{{-1, -1, 1}, {-1, 0, 0}, {1, 1, 1, 1}, {0, repeatY}},
		{{-1, -1, 1}, {-1, 0, 0}, {1, 1, 1, 1}, {0, repeatY}},
		{{-1, 1, -1}, {-1, 0, 0}, {1, 1, 1, 1}, {repeatZ, 0}},
		{{-1, -1, -1}, {-1, 0, 0}, {1, 1, 1, 1}, {repeatZ, repeatY}},

		// 右
		{{1, 1, -1}, {1, 0, 0}, {1, 1, 1, 1}, {0, 0}},
		{{1, 1, 1}, {1, 0, 0}, {1, 1, 1, 1}, {repeatZ, 0}},
		{{1, -1, -1}, {1, 0, 0}, {1, 1, 1, 1}, {0, repeatY}},
		{{1, -1, -1}, {1, 0, 0}, {1, 1, 1, 1}, {0, repeatY}},
		{{1, 1, 1}, {1, 0, 0}, {1, 1, 1, 1}, {repeatZ, 0}},
		{{1, -1, 1}, {1, 0, 0}, {1, 1, 1, 1}, {repeatZ, repeatY}},
	};

	D3D11_BUFFER_DESC buffer{};
	buffer.Usage = D3D11_USAGE_DEFAULT;
	buffer.ByteWidth = sizeof(vertices);
	buffer.BindFlags = D3D11_BIND_VERTEX_BUFFER;

	D3D11_SUBRESOURCE_DATA source{};
	source.pSysMem = vertices;
	Renderer::GetDevice()->CreateBuffer(&buffer, &source, &m_VertexBuffer);

	if (!m_TexturePath.empty())
	{
		m_Texture = Texture::Load(m_TexturePath.c_str());
	}

	Renderer::CreateVertexShader(&m_VertexShader, &m_VertexLayout, "shader\\unlitTextureVS.cso");
	Renderer::CreatePixelShader(&m_PixelShader, "shader\\unlitTexturePS.cso");
}

// このCubeが所有する描画リソースを解放する
void Cube::Uninit()
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

// 配置したCubeはその位置に置いておく
void Cube::Update()
{
}

// 設定した位置・向き・大きさでCubeを描く
void Cube::Draw()
{
	if (!m_VertexBuffer || !m_VertexLayout || !m_VertexShader || !m_PixelShader)
	{
		return;
	}

	XMMATRIX scale = XMMatrixScaling(m_Scale.x, m_Scale.y, m_Scale.z);
	XMMATRIX rotation = XMMatrixRotationRollPitchYaw(m_Rotation.x, m_Rotation.y, m_Rotation.z);
	XMMATRIX translation = XMMatrixTranslation(m_Position.x, m_Position.y, m_Position.z);
	Renderer::SetWorldMatrix(scale * rotation * translation);

	ID3D11DeviceContext *context = Renderer::GetDeviceContext();
	context->IASetInputLayout(m_VertexLayout);
	context->VSSetShader(m_VertexShader, nullptr, 0);
	context->PSSetShader(m_PixelShader, nullptr, 0);

	MATERIAL material{};
	material.Diffuse = m_Color;
	material.TextureEnable = m_Texture != nullptr;
	Renderer::SetMaterial(material);
	context->PSSetShaderResources(0, 1, &m_Texture);

	UINT stride = sizeof(VERTEX_3D);
	UINT offset = 0;
	context->IASetVertexBuffers(0, 1, &m_VertexBuffer, &stride, &offset);
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	context->Draw(36, 0);
}

// 描画に使う画像の場所を設定する
void Cube::SetTexture(const std::string &path)
{
	m_TexturePath = path;
}

// X・Y・Z方向の画像の繰り返し回数を設定する
void Cube::SetTextureRepeat(Vector3 repeat)
{
	m_TextureRepeat = repeat;
}

// 全体の色を設定する
void Cube::SetColor(XMFLOAT4 color)
{
	m_Color = color;
}
