#pragma once

#include "main.h"
#include "renderer.h"
#include "texture.h"
#include "camera.h"
#include <algorithm>

// 弾と着弾エフェクトで共通に使う、カメラに正対する画像の描画
class BillboardSprite
{
	// 画像・頂点と、透明部分が床を隠さない奥行き設定
	ID3D11Buffer *m_Buffer = nullptr;

	ID3D11InputLayout *m_Layout = nullptr;

	ID3D11VertexShader *m_VertexShader = nullptr;

	ID3D11PixelShader *m_PixelShader = nullptr;

	ID3D11ShaderResourceView *m_Texture = nullptr;

	ID3D11DepthStencilState *m_DepthRead = nullptr;

  public:
	// 画像を読み込み、書き換え可能な頂点と描画設定を用意する
	void Init(const char *path)
	{
		m_Texture = Texture::Load(path);

		D3D11_BUFFER_DESC buffer{};

		buffer.Usage = D3D11_USAGE_DYNAMIC;
		buffer.ByteWidth = sizeof(VERTEX_3D) * 12;
		buffer.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		buffer.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		Renderer::GetDevice()->CreateBuffer(&buffer, nullptr, &m_Buffer);
		Renderer::CreateVertexShader(&m_VertexShader, &m_Layout, "shader\\unlitTextureVS.cso");
		Renderer::CreatePixelShader(&m_PixelShader, "shader\\unlitTexturePS.cso");

		D3D11_DEPTH_STENCIL_DESC depth{};

		depth.DepthEnable = TRUE;
		depth.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
		depth.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
		Renderer::GetDevice()->CreateDepthStencilState(&depth, &m_DepthRead);
	}

	// 共有の画像はTexture側で管理し、このオブジェクトの描画リソースだけ解放する
	void Uninit()
	{
		m_Buffer->Release();
		m_Layout->Release();
		m_VertexShader->Release();
		m_PixelShader->Release();
		m_DepthRead->Release();
	}

	// カメラの回転だけを使い、黒い背景を加算合成で透明として描く
	void Draw(Camera *camera, const Vector3 &position, float radius, const XMFLOAT4 &color, int frame = 0, int columns = 1, int rows = 1)
	{
		if (!camera)
		{
			return;
		}

		auto *context = Renderer::GetDeviceContext();

		D3D11_MAPPED_SUBRESOURCE mapped{};

		if (FAILED(context->Map(m_Buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped)))
		{
			return;
		}

		frame = (std::max)(0, (std::min)(columns * rows - 1, frame));

		const float width = 1.0f / columns, height = 1.0f / rows;

		const float left = (frame % columns) * width, top = (frame / columns) * height;

		const XMFLOAT3 positions[] = {{-1, 1, 0}, {1, 1, 0}, {-1, -1, 0}, {1, -1, 0}};

		const XMFLOAT2 uv[] = {{left + 0.0005f, top + 0.0005f}, {left + width - 0.0005f, top + 0.0005f}, {left + 0.0005f, top + height - 0.0005f}, {left + width - 0.0005f, top + height - 0.0005f}};

		const int indices[] = {0, 1, 2, 2, 1, 3, 1, 0, 2, 1, 2, 3};

		auto *vertices = static_cast<VERTEX_3D *>(mapped.pData);

		for (int i = 0; i < 12; ++i)
		{
			vertices[i] = {};
			vertices[i].Position = positions[indices[i]];
			vertices[i].Normal = XMFLOAT3(0, 0, -1);
			vertices[i].Diffuse = color;
			vertices[i].TexCoord = uv[indices[i]];
		}

		context->Unmap(m_Buffer, 0);

		XMMATRIX facing = XMMatrixInverse(nullptr, camera->GetViewMatrix());

		facing.r[3] = XMVectorSet(0, 0, 0, 1);
		Renderer::SetWorldMatrix(XMMatrixScaling(radius, radius, radius) * facing * XMMatrixTranslation(position.x, position.y, position.z));
		context->IASetInputLayout(m_Layout);
		context->VSSetShader(m_VertexShader, nullptr, 0);
		context->PSSetShader(m_PixelShader, nullptr, 0);

		MATERIAL material{};

		material.Diffuse = {1, 1, 1, 1};
		material.TextureEnable = true;
		Renderer::SetMaterial(material);

		// 他の3Dやボタンへ設定が漏れないよう、元の合成と奥行きを戻す
		ID3D11DepthStencilState *previousDepth = nullptr;

		ID3D11BlendState *previousBlend = nullptr;

		UINT stencil = 0, mask = 0;

		FLOAT factors[4]{};

		context->OMGetDepthStencilState(&previousDepth, &stencil);
		context->OMGetBlendState(&previousBlend, factors, &mask);
		context->OMSetDepthStencilState(m_DepthRead, 0);
		Renderer::SetAddBlendEnable(true);

		UINT stride = sizeof(VERTEX_3D), offset = 0;

		context->IASetVertexBuffers(0, 1, &m_Buffer, &stride, &offset);
		context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		context->PSSetShaderResources(0, 1, &m_Texture);
		context->Draw(12, 0);
		context->OMSetDepthStencilState(previousDepth, stencil);
		context->OMSetBlendState(previousBlend, factors, mask);

		if (previousDepth)
		{
			previousDepth->Release();
		}

		if (previousBlend)
		{
			previousBlend->Release();
		}
	}
};
