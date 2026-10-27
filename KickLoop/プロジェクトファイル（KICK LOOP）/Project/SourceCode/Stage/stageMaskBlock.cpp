#include "main.h"
#include "renderer.h"
#include "texture.h"
#include "stageMaskBlock.h"

#include <cassert>

// マスク用のピクセルシェーダーに切り替える
void StageMaskBlock::Init()
{
	StageBlock::Init();

	// t1に渡すマスクテクスチャ。
	m_MaskTexture = Texture::Load("asset\\texture\\cyberWallMask.png");

	assert(m_MaskTexture);

	if (m_PixelShader)
	{
		m_PixelShader->Release();
		m_PixelShader = nullptr;
	}

	// StageBlock標準のPixelShaderではなく、
	// 通常テクスチャとマスクテクスチャを合成する専用PixelShaderを使う。
	Renderer::CreatePixelShader(
		&m_PixelShader,
		"shader\\emissiveMaskPS.cso"
	);

	if (m_EmissiveBuffer)
	{
		m_EmissiveBuffer->Release();
		m_EmissiveBuffer = nullptr;
	}

	{
		D3D11_BUFFER_DESC bd{};
		bd.Usage = D3D11_USAGE_DYNAMIC;
		bd.ByteWidth = sizeof(StageMaskBlockEmissiveBuffer);
		bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

		// PixelShaderへ渡すマスク強調用の定数バッファ。
		Renderer::GetDevice()->CreateBuffer(
			&bd,
			nullptr,
			&m_EmissiveBuffer
		);
	}
}

void StageMaskBlock::Uninit()
{
	m_MaskTexture = nullptr;

	if (m_EmissiveBuffer)
	{
		m_EmissiveBuffer->Release();
		m_EmissiveBuffer = nullptr;
	}

	StageBlock::Uninit();
}

void StageMaskBlock::Draw()
{
	if (!Active())
	{
		return;
	}

	if (m_Alpha <= 0.0f)
	{
		return;
	}

	Renderer::GetDeviceContext()->IASetInputLayout(m_VertexLayout);

	Renderer::GetDeviceContext()->VSSetShader(
		m_VertexShader,
		NULL,
		0
	);

	Renderer::GetDeviceContext()->PSSetShader(
		m_PixelShader,
		NULL,
		0
	);

	MATERIAL material{};

	material.Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, m_Alpha);
	material.TextureEnable = true;

	Renderer::SetMaterial(material);

	if (m_EmissiveBuffer)
	{
		D3D11_MAPPED_SUBRESOURCE msr{};

		// マスク強調色をPixelShaderへ渡すため、定数バッファを書き換える。
		Renderer::GetDeviceContext()->Map(
			m_EmissiveBuffer,
			0,
			D3D11_MAP_WRITE_DISCARD,
			0,
			&msr
		);

		StageMaskBlockEmissiveBuffer* buffer = (StageMaskBlockEmissiveBuffer*)msr.pData;

		if (m_IsRainbow)
		{
			float time = (float)GetTickCount64() * 0.001f * m_RainbowSpeed;
			XMFLOAT3 rainbowColor = GetRainbowColor(time);

			// 虹色設定時は、時間で変化する色をマスク部分の強調色として渡す。
			buffer->emissiveColor = XMFLOAT4(rainbowColor.x, rainbowColor.y, rainbowColor.z, m_Alpha);
		}
		else
		{
			// 通常時はSetColor / SetEmissiveColorで設定した色を使う。
			buffer->emissiveColor = XMFLOAT4(m_EmissiveColor.x, m_EmissiveColor.y, m_EmissiveColor.z, m_Alpha);
		}

		// x: マスク部分を設定色へ寄せる強さ。
		// y,z,w: 現在のShaderでは未使用。
		buffer->emissiveParam = XMFLOAT4(m_EmissivePower, m_MaskColorStart, m_MaskColorEnd, 0.0f);

		// 現在のShaderでは未使用。
		// マスク範囲をC++側から調整する場合に使う。
		buffer->maskParam = XMFLOAT4(m_MaskCoreStart, m_MaskCoreEnd, 0.0f, 0.0f);

		Renderer::GetDeviceContext()->Unmap(
			m_EmissiveBuffer,
			0
		);

		// b1にマスク表示用パラメータを渡す。
		Renderer::GetDeviceContext()->PSSetConstantBuffers(
			1,
			1,
			&m_EmissiveBuffer
		);
	}

	if (m_Texture)
	{
		// t0: 通常の壁テクスチャ。
		Renderer::GetDeviceContext()->PSSetShaderResources(
			0,
			1,
			&m_Texture
		);
	}

	if (m_MaskTexture)
	{
		// t1: 色を変えたい部分を指定するマスクテクスチャ。
		Renderer::GetDeviceContext()->PSSetShaderResources(
			1,
			1,
			&m_MaskTexture
		);
	}

	UINT stride = sizeof(VERTEX_3D);
	UINT offset = 0;

	Renderer::GetDeviceContext()->IASetVertexBuffers(
		0,
		1,
		&m_VertexBuffer,
		&stride,
		&offset
	);

	Renderer::GetDeviceContext()->IASetIndexBuffer(
		m_IndexBuffer,
		DXGI_FORMAT_R32_UINT,
		0
	);

	Renderer::GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	XMMATRIX scale = XMMatrixScaling(m_Scale.x * m_Size.x, m_Scale.y * m_Size.y, m_Scale.z * m_Size.z);
	XMMATRIX rotation = XMMatrixRotationRollPitchYaw(m_Rotation.x, m_Rotation.y, m_Rotation.z);
	XMMATRIX translation = XMMatrixTranslation(m_Position.x, m_Position.y, m_Position.z);
	XMMATRIX world = scale * rotation * translation;

	Renderer::SetWorldMatrix(world);

	D3D11_RASTERIZER_DESC rasterizerDesc{};
	rasterizerDesc.FillMode = D3D11_FILL_SOLID;
	rasterizerDesc.CullMode = D3D11_CULL_NONE;
	rasterizerDesc.FrontCounterClockwise = false;
	rasterizerDesc.DepthClipEnable = true;

	ID3D11RasterizerState* rasterizerState = nullptr;

	Renderer::GetDevice()->CreateRasterizerState(
		&rasterizerDesc,
		&rasterizerState
	);

	Renderer::GetDeviceContext()->RSSetState(rasterizerState);

	if (rasterizerState)
	{
		rasterizerState->Release();
		rasterizerState = nullptr;
	}

	Renderer::GetDeviceContext()->DrawIndexed(
		36,
		0,
		0
	);
}

// 壁の操作可否と描画設定を初期状態に戻す
void StageMaskBlock::ResetForReuse()
{
	SetActive(false);
	SetVisible(false);

	SetSize(Vector3(1.0f, 1.0f, 1.0f));
	SetBlockRotation(Vector3(0.0f, 0.0f, 0.0f));

	SetUseOBBHit(false);

	SetWallKickGroupId(-1);
	SetCanAddWallKickGauge(true);

	SetCanWallKick(true);
	SetCanWallSlide(true);
	SetCanClimb(true);

	SetUVScroll(false);
	SetUVFlipZ(false);
	SetUVRepeat(XMFLOAT2(1.0f, 1.0f));

	SetRainbow(false);
	SetRainbowSpeed(1.0f);

	SetColor(
		0.0f,
		0.85f,
		1.0f,
		1.0f
	);

	SetAlpha(1.0f);
	SetEmissivePower(1.8f);

	SetMaskColorRange(
		0.08f,
		0.60f
	);

	SetMaskCoreRange(
		0.15f,
		0.65f
	);

	SetMaskTexture("asset\\texture\\cyberWallMask.png");
}

// 色を変える範囲のマスクを読み込む
void StageMaskBlock::SetMaskTexture(const char* path)
{
	if (path == nullptr)
	{
		return;
	}

	m_MaskTexture = Texture::Load(path);
}

void StageMaskBlock::SetEmissiveColor(
	float r,
	float g,
	float b,
	float a
)
{
	m_EmissiveColor = XMFLOAT4(r, g, b, Clamp01(a));
}

void StageMaskBlock::SetEmissivePower(float power)
{
	// マスク部分を設定色へ寄せる強さ。
	// PixelShader内の色合成倍率として使う。
	m_EmissivePower = power;

	if (m_EmissivePower < 0.0f)
	{
		m_EmissivePower = 0.0f;
	}
}

void StageMaskBlock::SetMaskColorRange(
	float start,
	float end
)
{
	m_MaskColorStart = start;
	m_MaskColorEnd = end;

	NormalizeMaskRange(
		m_MaskColorStart,
		m_MaskColorEnd
	);
}

void StageMaskBlock::SetMaskCoreRange(
	float start,
	float end
)
{
	m_MaskCoreStart = start;
	m_MaskCoreEnd = end;

	NormalizeMaskRange(
		m_MaskCoreStart,
		m_MaskCoreEnd
	);
}

void StageMaskBlock::SetColor(
	float r,
	float g,
	float b,
	float a
)
{
	float alpha = Clamp01(a);

	m_Color = XMFLOAT4(r, g, b, alpha);

	// StageBlockとしての色と、マスク部分の強調色を同じ色にする。
	m_EmissiveColor = XMFLOAT4(r, g, b, alpha);

	m_Alpha = alpha;
}

float StageMaskBlock::Clamp01(float value) const
{
	if (value < 0.0f)
	{
		return 0.0f;
	}

	if (value > 1.0f)
	{
		return 1.0f;
	}

	return value;
}

void StageMaskBlock::NormalizeMaskRange(
	float& start,
	float& end
) const
{
	start = Clamp01(start);
	end = Clamp01(end);

	if (start > end)
	{
		float temp = start;
		start = end;
		end = temp;
	}
}