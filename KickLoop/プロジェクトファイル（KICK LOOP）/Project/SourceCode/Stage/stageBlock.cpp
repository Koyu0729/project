#include "main.h"
#include "renderer.h"
#include "texture.h"
#include "stageBlock.h"

#include <cassert>
#include <cmath>

constexpr int NUM_VERTICES = 24;

// 六面分の頂点を作る
void StageBlock::BuildVertexData(
	VERTEX_3D* vertex
)
{
	vertex[0].Position = XMFLOAT3(-1.0f, 1.0f, -1.0f);
	vertex[0].Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	vertex[0].Normal = XMFLOAT3(0.0f, 1.0f, 0.0f);

	vertex[1].Position = XMFLOAT3(1.0f, 1.0f, -1.0f);
	vertex[1].Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	vertex[1].Normal = XMFLOAT3(0.0f, 1.0f, 0.0f);

	vertex[2].Position = XMFLOAT3(-1.0f, 1.0f, 1.0f);
	vertex[2].Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	vertex[2].Normal = XMFLOAT3(0.0f, 1.0f, 0.0f);

	vertex[3].Position = XMFLOAT3(1.0f, 1.0f, 1.0f);
	vertex[3].Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	vertex[3].Normal = XMFLOAT3(0.0f, 1.0f, 0.0f);

	if (!m_IsUVFlipZ)
	{
		vertex[0].TexCoord = XMFLOAT2(0.0f, 0.0f);
		vertex[1].TexCoord = XMFLOAT2(1.0f, 0.0f);
		vertex[2].TexCoord = XMFLOAT2(0.0f, 1.0f);
		vertex[3].TexCoord = XMFLOAT2(1.0f, 1.0f);
	}
	else
	{
		vertex[0].TexCoord = XMFLOAT2(0.0f, 1.0f);
		vertex[1].TexCoord = XMFLOAT2(1.0f, 1.0f);
		vertex[2].TexCoord = XMFLOAT2(0.0f, 0.0f);
		vertex[3].TexCoord = XMFLOAT2(1.0f, 0.0f);
	}

	vertex[4].Position = XMFLOAT3(-1.0f, -1.0f, 1.0f);
	vertex[4].Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	vertex[4].TexCoord = XMFLOAT2(0.0f, 0.0f);
	vertex[4].Normal = XMFLOAT3(0.0f, -1.0f, 0.0f);

	vertex[5].Position = XMFLOAT3(1.0f, -1.0f, 1.0f);
	vertex[5].Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	vertex[5].TexCoord = XMFLOAT2(1.0f, 0.0f);
	vertex[5].Normal = XMFLOAT3(0.0f, -1.0f, 0.0f);

	vertex[6].Position = XMFLOAT3(-1.0f, -1.0f, -1.0f);
	vertex[6].Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	vertex[6].TexCoord = XMFLOAT2(0.0f, 1.0f);
	vertex[6].Normal = XMFLOAT3(0.0f, -1.0f, 0.0f);

	vertex[7].Position = XMFLOAT3(1.0f, -1.0f, -1.0f);
	vertex[7].Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	vertex[7].TexCoord = XMFLOAT2(1.0f, 1.0f);
	vertex[7].Normal = XMFLOAT3(0.0f, -1.0f, 0.0f);

	vertex[8].Position = XMFLOAT3(-1.0f, 1.0f, -1.0f);
	vertex[8].Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	vertex[8].TexCoord = XMFLOAT2(0.0f, 0.0f);
	vertex[8].Normal = XMFLOAT3(0.0f, 0.0f, -1.0f);

	vertex[9].Position = XMFLOAT3(1.0f, 1.0f, -1.0f);
	vertex[9].Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	vertex[9].TexCoord = XMFLOAT2(1.0f, 0.0f);
	vertex[9].Normal = XMFLOAT3(0.0f, 0.0f, -1.0f);

	vertex[10].Position = XMFLOAT3(-1.0f, -1.0f, -1.0f);
	vertex[10].Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	vertex[10].TexCoord = XMFLOAT2(0.0f, 1.0f);
	vertex[10].Normal = XMFLOAT3(0.0f, 0.0f, -1.0f);

	vertex[11].Position = XMFLOAT3(1.0f, -1.0f, -1.0f);
	vertex[11].Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	vertex[11].TexCoord = XMFLOAT2(1.0f, 1.0f);
	vertex[11].Normal = XMFLOAT3(0.0f, 0.0f, -1.0f);

	vertex[12].Position = XMFLOAT3(1.0f, 1.0f, 1.0f);
	vertex[12].Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	vertex[12].TexCoord = XMFLOAT2(0.0f, 0.0f);
	vertex[12].Normal = XMFLOAT3(0.0f, 0.0f, 1.0f);

	vertex[13].Position = XMFLOAT3(-1.0f, 1.0f, 1.0f);
	vertex[13].Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	vertex[13].TexCoord = XMFLOAT2(1.0f, 0.0f);
	vertex[13].Normal = XMFLOAT3(0.0f, 0.0f, 1.0f);

	vertex[14].Position = XMFLOAT3(1.0f, -1.0f, 1.0f);
	vertex[14].Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	vertex[14].TexCoord = XMFLOAT2(0.0f, 1.0f);
	vertex[14].Normal = XMFLOAT3(0.0f, 0.0f, 1.0f);

	vertex[15].Position = XMFLOAT3(-1.0f, -1.0f, 1.0f);
	vertex[15].Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	vertex[15].TexCoord = XMFLOAT2(1.0f, 1.0f);
	vertex[15].Normal = XMFLOAT3(0.0f, 0.0f, 1.0f);

	vertex[16].Position = XMFLOAT3(1.0f, 1.0f, -1.0f);
	vertex[16].Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	vertex[16].TexCoord = XMFLOAT2(0.0f, 0.0f);
	vertex[16].Normal = XMFLOAT3(1.0f, 0.0f, 0.0f);

	vertex[17].Position = XMFLOAT3(1.0f, 1.0f, 1.0f);
	vertex[17].Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	vertex[17].TexCoord = XMFLOAT2(1.0f, 0.0f);
	vertex[17].Normal = XMFLOAT3(1.0f, 0.0f, 0.0f);

	vertex[18].Position = XMFLOAT3(1.0f, -1.0f, -1.0f);
	vertex[18].Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	vertex[18].TexCoord = XMFLOAT2(0.0f, 1.0f);
	vertex[18].Normal = XMFLOAT3(1.0f, 0.0f, 0.0f);

	vertex[19].Position = XMFLOAT3(1.0f, -1.0f, 1.0f);
	vertex[19].Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	vertex[19].TexCoord = XMFLOAT2(1.0f, 1.0f);
	vertex[19].Normal = XMFLOAT3(1.0f, 0.0f, 0.0f);

	vertex[20].Position = XMFLOAT3(-1.0f, 1.0f, 1.0f);
	vertex[20].Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	vertex[20].TexCoord = XMFLOAT2(0.0f, 0.0f);
	vertex[20].Normal = XMFLOAT3(-1.0f, 0.0f, 0.0f);

	vertex[21].Position = XMFLOAT3(-1.0f, 1.0f, -1.0f);
	vertex[21].Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	vertex[21].TexCoord = XMFLOAT2(1.0f, 0.0f);
	vertex[21].Normal = XMFLOAT3(-1.0f, 0.0f, 0.0f);

	vertex[22].Position = XMFLOAT3(-1.0f, -1.0f, 1.0f);
	vertex[22].Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	vertex[22].TexCoord = XMFLOAT2(0.0f, 1.0f);
	vertex[22].Normal = XMFLOAT3(-1.0f, 0.0f, 0.0f);

	vertex[23].Position = XMFLOAT3(-1.0f, -1.0f, -1.0f);
	vertex[23].Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	vertex[23].TexCoord = XMFLOAT2(1.0f, 1.0f);
	vertex[23].Normal = XMFLOAT3(-1.0f, 0.0f, 0.0f);

	for (int i = 0; i < NUM_VERTICES; i++)
	{
		vertex[i].TexCoord.x =
			vertex[i].TexCoord.x *
			m_UVRepeat.x +
			m_UVScroll.x;

		vertex[i].TexCoord.y =
			vertex[i].TexCoord.y *
			m_UVRepeat.y +
			m_UVScroll.y;
	}
}

void StageBlock::NormalizeSize()
{
	if (m_Size.x <= 0.001f)
	{
		m_Size.x = 0.001f;
	}

	if (m_Size.y <= 0.001f)
	{
		m_Size.y = 0.001f;
	}

	if (m_Size.z <= 0.001f)
	{
		m_Size.z = 0.001f;
	}
}

void StageBlock::Init()
{
	VERTEX_3D vertex[NUM_VERTICES];

	BuildVertexData(
		vertex
	);

	{
		D3D11_BUFFER_DESC bd{};
		bd.Usage = D3D11_USAGE_DYNAMIC;
		bd.ByteWidth = sizeof(VERTEX_3D) * NUM_VERTICES;
		bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

		D3D11_SUBRESOURCE_DATA sd{};
		sd.pSysMem = vertex;

		Renderer::GetDevice()->CreateBuffer(
			&bd,
			&sd,
			&m_VertexBuffer
		);
	}

	unsigned int index[36] =
	{
		0, 1, 2,
		1, 3, 2,

		4, 5, 6,
		5, 7, 6,

		8, 9, 10,
		9, 11, 10,

		12, 13, 14,
		13, 15, 14,

		16, 17, 18,
		17, 19, 18,

		20, 21, 22,
		21, 23, 22
	};

	{
		D3D11_BUFFER_DESC bd{};
		bd.Usage = D3D11_USAGE_DEFAULT;
		bd.ByteWidth = sizeof(unsigned int) * 36;
		bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
		bd.CPUAccessFlags = 0;

		D3D11_SUBRESOURCE_DATA sd{};
		sd.pSysMem = index;

		Renderer::GetDevice()->CreateBuffer(
			&bd,
			&sd,
			&m_IndexBuffer
		);
	}

	m_Texture = Texture::Load("asset\\texture\\frameBoxR.png");

	assert(m_Texture);

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

void StageBlock::Uninit()
{
	m_Texture = nullptr;

	if (m_VertexBuffer)
	{
		m_VertexBuffer->Release();
		m_VertexBuffer = nullptr;
	}

	if (m_IndexBuffer)
	{
		m_IndexBuffer->Release();
		m_IndexBuffer = nullptr;
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

void StageBlock::Update()
{
}

void StageBlock::Draw()
{
	if (!Active())
	{
		return;
	}

	if (m_Alpha <= 0.0f)
	{
		return;
	}

	Renderer::GetDeviceContext()->IASetInputLayout(
		m_VertexLayout
	);

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

	if (m_IsRainbow)
	{
		float time =
			(float)GetTickCount64() *
			0.001f *
			m_RainbowSpeed;

		XMFLOAT3 rainbowColor =
			GetRainbowColor(time);

		material.Diffuse =
			XMFLOAT4(
				rainbowColor.x,
				rainbowColor.y,
				rainbowColor.z,
				m_Alpha
			);
	}
	else
	{
		material.Diffuse =
			m_Color;

		material.Diffuse.w =
			m_Alpha;
	}

	material.TextureEnable = true;

	Renderer::SetMaterial(
		material
	);

	if (m_Texture)
	{
		Renderer::GetDeviceContext()->PSSetShaderResources(
			0,
			1,
			&m_Texture
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

	Renderer::GetDeviceContext()->IASetPrimitiveTopology(
		D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST
	);

	XMMATRIX scale =
		XMMatrixScaling(
			m_Scale.x * m_Size.x,
			m_Scale.y * m_Size.y,
			m_Scale.z * m_Size.z
		);

	XMMATRIX rotation =
		XMMatrixRotationRollPitchYaw(
			m_Rotation.x,
			m_Rotation.y,
			m_Rotation.z
		);

	XMMATRIX translation =
		XMMatrixTranslation(
			m_Position.x,
			m_Position.y,
			m_Position.z
		);

	XMMATRIX world =
		scale *
		rotation *
		translation;

	Renderer::SetWorldMatrix(
		world
	);

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

	Renderer::GetDeviceContext()->RSSetState(
		rasterizerState
	);

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

void StageBlock::SetTexture(const char* path)
{
	if (path == nullptr)
	{
		return;
	}

	m_Texture =
		Texture::Load(path);
}

void StageBlock::SetUVFlipZ(bool isFlip)
{
	m_IsUVFlipZ =
		isFlip;

	UpdateVertexUV();
}

void StageBlock::SetSize(Vector3 size)
{
	m_Size =
		size;

	NormalizeSize();
}

Vector3 StageBlock::GetSize() const
{
	return m_Size;
}

Vector3 StageBlock::GetMin() const
{
	Vector3 halfSize;

	halfSize.x =
		m_Scale.x *
		m_Size.x;

	halfSize.y =
		m_Scale.y *
		m_Size.y;

	halfSize.z =
		m_Scale.z *
		m_Size.z;

	return Vector3(
		m_Position.x - halfSize.x,
		m_Position.y - halfSize.y,
		m_Position.z - halfSize.z
	);
}

Vector3 StageBlock::GetMax() const
{
	Vector3 halfSize;

	halfSize.x =
		m_Scale.x *
		m_Size.x;

	halfSize.y =
		m_Scale.y *
		m_Size.y;

	halfSize.z =
		m_Scale.z *
		m_Size.z;

	return Vector3(
		m_Position.x + halfSize.x,
		m_Position.y + halfSize.y,
		m_Position.z + halfSize.z
	);
}

// ブロックと対象の接触を調べる
bool StageBlock::IsHitAABB(
	Vector3 position,
	Vector3 size
) const
{
	if (!Active())
	{
		return false;
	}

	Vector3 min =
		GetMin();

	Vector3 max =
		GetMax();

	Vector3 otherMin =
		Vector3(
			position.x - size.x,
			position.y - size.y,
			position.z - size.z
		);

	Vector3 otherMax =
		Vector3(
			position.x + size.x,
			position.y + size.y,
			position.z + size.z
		);

	if (max.x <= otherMin.x)
	{
		return false;
	}

	if (min.x >= otherMax.x)
	{
		return false;
	}

	if (max.y <= otherMin.y)
	{
		return false;
	}

	if (min.y >= otherMax.y)
	{
		return false;
	}

	if (max.z <= otherMin.z)
	{
		return false;
	}

	if (min.z >= otherMax.z)
	{
		return false;
	}

	return true;
}

bool StageBlock::IsHit(
	Vector3 position,
	Vector3 size
)
{
	if (m_UseOBBHit)
	{
		return IsHitOBB(
			position,
			size
		);
	}

	return IsHitAABB(
		position,
		size
	);
}

bool StageBlock::IsHitOBB(
	Vector3 position,
	Vector3 size
) const
{
	if (!Active())
	{
		return false;
	}

	Vector3 blockHalfSize;

	blockHalfSize.x =
		m_Scale.x *
		m_Size.x;

	blockHalfSize.y =
		m_Scale.y *
		m_Size.y;

	blockHalfSize.z =
		m_Scale.z *
		m_Size.z;

	XMMATRIX blockRotation =
		XMMatrixRotationRollPitchYaw(
			m_Rotation.x,
			m_Rotation.y,
			m_Rotation.z
		);

	XMVECTOR blockAxis[3];

	blockAxis[0] =
		XMVector3Normalize(
			XMVector3TransformNormal(
				XMVectorSet(
					1.0f,
					0.0f,
					0.0f,
					0.0f
				),
				blockRotation
			)
		);

	blockAxis[1] =
		XMVector3Normalize(
			XMVector3TransformNormal(
				XMVectorSet(
					0.0f,
					1.0f,
					0.0f,
					0.0f
				),
				blockRotation
			)
		);

	blockAxis[2] =
		XMVector3Normalize(
			XMVector3TransformNormal(
				XMVectorSet(
					0.0f,
					0.0f,
					1.0f,
					0.0f
				),
				blockRotation
			)
		);

	XMVECTOR otherAxis[3];

	otherAxis[0] =
		XMVectorSet(
			1.0f,
			0.0f,
			0.0f,
			0.0f
		);

	otherAxis[1] =
		XMVectorSet(
			0.0f,
			1.0f,
			0.0f,
			0.0f
		);

	otherAxis[2] =
		XMVectorSet(
			0.0f,
			0.0f,
			1.0f,
			0.0f
		);

	float blockHalf[3] =
	{
		blockHalfSize.x,
		blockHalfSize.y,
		blockHalfSize.z
	};

	float otherHalf[3] =
	{
		size.x,
		size.y,
		size.z
	};

	XMVECTOR blockCenter =
		XMVectorSet(
			m_Position.x,
			m_Position.y,
			m_Position.z,
			0.0f
		);

	XMVECTOR otherCenter =
		XMVectorSet(
			position.x,
			position.y,
			position.z,
			0.0f
		);

	XMVECTOR centerDiff =
		XMVectorSubtract(
			otherCenter,
			blockCenter
		);

	XMVECTOR checkAxis[15];

	checkAxis[0] = blockAxis[0];
	checkAxis[1] = blockAxis[1];
	checkAxis[2] = blockAxis[2];

	checkAxis[3] = otherAxis[0];
	checkAxis[4] = otherAxis[1];
	checkAxis[5] = otherAxis[2];

	checkAxis[6] = XMVector3Cross(blockAxis[0], otherAxis[0]);
	checkAxis[7] = XMVector3Cross(blockAxis[0], otherAxis[1]);
	checkAxis[8] = XMVector3Cross(blockAxis[0], otherAxis[2]);

	checkAxis[9] = XMVector3Cross(blockAxis[1], otherAxis[0]);
	checkAxis[10] = XMVector3Cross(blockAxis[1], otherAxis[1]);
	checkAxis[11] = XMVector3Cross(blockAxis[1], otherAxis[2]);

	checkAxis[12] = XMVector3Cross(blockAxis[2], otherAxis[0]);
	checkAxis[13] = XMVector3Cross(blockAxis[2], otherAxis[1]);
	checkAxis[14] = XMVector3Cross(blockAxis[2], otherAxis[2]);

	for (int i = 0; i < 15; i++)
	{
		float axisLengthSq =
			XMVectorGetX(
				XMVector3LengthSq(
					checkAxis[i]
				)
			);

		if (axisLengthSq <= 0.000001f)
		{
			continue;
		}

		XMVECTOR axis =
			XMVector3Normalize(
				checkAxis[i]
			);

		float distance =
			XMVectorGetX(
				XMVector3Dot(
					centerDiff,
					axis
				)
			);

		if (distance < 0.0f)
		{
			distance = -distance;
		}

		float blockRadius = 0.0f;

		for (int blockAxisIndex = 0; blockAxisIndex < 3; blockAxisIndex++)
		{
			float dot =
				XMVectorGetX(
					XMVector3Dot(
						blockAxis[blockAxisIndex],
						axis
					)
				);

			if (dot < 0.0f)
			{
				dot = -dot;
			}

			blockRadius +=
				blockHalf[blockAxisIndex] *
				dot;
		}

		float otherRadius = 0.0f;

		for (int otherAxisIndex = 0; otherAxisIndex < 3; otherAxisIndex++)
		{
			float dot =
				XMVectorGetX(
					XMVector3Dot(
						otherAxis[otherAxisIndex],
						axis
					)
				);

			if (dot < 0.0f)
			{
				dot = -dot;
			}

			otherRadius +=
				otherHalf[otherAxisIndex] *
				dot;
		}

		if (distance > blockRadius + otherRadius)
		{
			return false;
		}
	}

	return true;
}

// 斜面の高さと法線を求める
bool StageBlock::GetSlopeRideSurfaceY(
	Vector3 position,
	Vector3 size,
	float& outSurfaceY,
	Vector3& outNormal
)
{
	outSurfaceY = 0.0f;
	outNormal = Vector3(0.0f, 1.0f, 0.0f);

	XMMATRIX rotation =
		XMMatrixRotationRollPitchYaw(
			m_Rotation.x,
			m_Rotation.y,
			m_Rotation.z
		);

	XMMATRIX inverseRotation =
		XMMatrixInverse(
			nullptr,
			rotation
		);

	XMVECTOR localPoint =
		XMVector3TransformCoord(
			XMVectorSet(
				position.x - m_Position.x,
				position.y - m_Position.y,
				position.z - m_Position.z,
				1.0f
			),
			inverseRotation
		);

	float localX =
		XMVectorGetX(localPoint);

	float localZ =
		XMVectorGetZ(localPoint);

	Vector3 halfSize =
		Vector3(
			m_Scale.x * m_Size.x,
			m_Scale.y * m_Size.y,
			m_Scale.z * m_Size.z
		);

	if (localX < -halfSize.x || localX > halfSize.x)
	{
		return false;
	}

	if (localZ < -halfSize.z || localZ > halfSize.z)
	{
		return false;
	}

	XMVECTOR worldTopCenter =
		XMVector3TransformCoord(
			XMVectorSet(
				0.0f,
				halfSize.y,
				0.0f,
				1.0f
			),
			rotation
		);

	worldTopCenter =
		XMVectorAdd(
			worldTopCenter,
			XMVectorSet(
				m_Position.x,
				m_Position.y,
				m_Position.z,
				0.0f
			)
		);

	XMVECTOR worldNormalVector =
		XMVector3Normalize(
			XMVector3TransformNormal(
				XMVectorSet(
					0.0f,
					1.0f,
					0.0f,
					0.0f
				),
				rotation
			)
		);

	Vector3 normal =
		Vector3(
			XMVectorGetX(worldNormalVector),
			XMVectorGetY(worldNormalVector),
			XMVectorGetZ(worldNormalVector)
		);

	if (fabsf(normal.y) < 0.001f)
	{
		return false;
	}

	float topCenterX =
		XMVectorGetX(worldTopCenter);

	float topCenterY =
		XMVectorGetY(worldTopCenter);

	float topCenterZ =
		XMVectorGetZ(worldTopCenter);

	float surfaceY =
		topCenterY -
		normal.x * (position.x - topCenterX) / normal.y -
		normal.z * (position.z - topCenterZ) / normal.y;

	float playerBottomY =
		position.y -
		size.y;

	float distance =
		playerBottomY -
		surfaceY;

	if (distance < 0.0f)
	{
		distance = -distance;
	}

	if (distance > 1.20f)
	{
		return false;
	}

	outSurfaceY =
		surfaceY;

	outNormal =
		normal;

	return true;
}

bool StageBlock::GetSlopeRideNormal(
	Vector3 position,
	Vector3 size,
	Vector3& outNormal
)
{
	float surfaceY = 0.0f;

	return GetSlopeRideSurfaceY(
		position,
		size,
		surfaceY,
		outNormal
	);
}

bool StageBlock::GetHitNormal(
	Vector3 position,
	Vector3 size,
	Vector3& outNormal
)
{
	outNormal =
		Vector3(
			0.0f,
			0.0f,
			0.0f
		);

	if (!IsHit(
		position,
		size
	))
	{
		return false;
	}

	Vector3 halfSize =
		Vector3(
			m_Scale.x * m_Size.x,
			m_Scale.y * m_Size.y,
			m_Scale.z * m_Size.z
		);

	XMMATRIX rotation =
		XMMatrixRotationRollPitchYaw(
			m_Rotation.x,
			m_Rotation.y,
			m_Rotation.z
		);

	XMMATRIX inverseRotation =
		XMMatrixInverse(
			nullptr,
			rotation
		);

	XMVECTOR localPoint =
		XMVector3TransformCoord(
			XMVectorSet(
				position.x - m_Position.x,
				position.y - m_Position.y,
				position.z - m_Position.z,
				1.0f
			),
			inverseRotation
		);

	float localX =
		XMVectorGetX(localPoint);

	float localY =
		XMVectorGetY(localPoint);

	float localZ =
		XMVectorGetZ(localPoint);

	float penetrationX =
		halfSize.x +
		size.x -
		fabsf(localX);

	float penetrationY =
		halfSize.y +
		size.y -
		fabsf(localY);

	float penetrationZ =
		halfSize.z +
		size.z -
		fabsf(localZ);

	Vector3 localNormal =
		Vector3(
			0.0f,
			0.0f,
			0.0f
		);

	if (penetrationX <= penetrationY && penetrationX <= penetrationZ)
	{
		if (localX >= 0.0f)
		{
			localNormal.x = 1.0f;
		}
		else
		{
			localNormal.x = -1.0f;
		}
	}
	else if (penetrationY <= penetrationX && penetrationY <= penetrationZ)
	{
		if (localY >= 0.0f)
		{
			localNormal.y = 1.0f;
		}
		else
		{
			localNormal.y = -1.0f;
		}
	}
	else
	{
		if (localZ >= 0.0f)
		{
			localNormal.z = 1.0f;
		}
		else
		{
			localNormal.z = -1.0f;
		}
	}

	XMVECTOR worldNormalVector =
		XMVector3Normalize(
			XMVector3TransformNormal(
				XMVectorSet(
					localNormal.x,
					localNormal.y,
					localNormal.z,
					0.0f
				),
				rotation
			)
		);

	outNormal =
		Vector3(
			XMVectorGetX(worldNormalVector),
			XMVectorGetY(worldNormalVector),
			XMVectorGetZ(worldNormalVector)
		);

	return true;
}

bool StageBlock::IsHitTopFace(
	Vector3 position,
	Vector3 size,
	Vector3& outNormal
)
{
	outNormal =
		Vector3(
			0.0f,
			1.0f,
			0.0f
		);

	if (!GetHitNormal(
		position,
		size,
		outNormal
	))
	{
		return false;
	}

	if (outNormal.y >= 0.35f)
	{
		return true;
	}

	return false;
}

void StageBlock::SetUseOBBHit(bool useOBBHit)
{
	m_UseOBBHit =
		useOBBHit;
}

void StageBlock::SetBlockRotation(Vector3 rotation)
{
	m_Rotation =
		rotation;
}

void StageBlock::AddUVScroll(float scroll)
{
	if (!m_IsUVScroll)
	{
		return;
	}

	m_UVScroll.x +=
		scroll *
		m_UVScrollDirection.x;

	m_UVScroll.y +=
		scroll *
		m_UVScrollDirection.y;

	UpdateVertexUV();
}

void StageBlock::UpdateVertexUV()
{
	if (m_VertexBuffer == nullptr)
	{
		return;
	}

	VERTEX_3D vertex[NUM_VERTICES];

	BuildVertexData(
		vertex
	);

	D3D11_MAPPED_SUBRESOURCE msr{};

	Renderer::GetDeviceContext()->Map(
		m_VertexBuffer,
		0,
		D3D11_MAP_WRITE_DISCARD,
		0,
		&msr
	);

	VERTEX_3D* mappedVertex =
		(VERTEX_3D*)msr.pData;

	for (int i = 0; i < NUM_VERTICES; i++)
	{
		mappedVertex[i] =
			vertex[i];
	}

	Renderer::GetDeviceContext()->Unmap(
		m_VertexBuffer,
		0
	);
}

void StageBlock::SetUVScrollDirection(XMFLOAT2 direction)
{
	m_UVScrollDirection =
		direction;
}

void StageBlock::SetUVScroll(bool isScroll)
{
	m_IsUVScroll =
		isScroll;
}

void StageBlock::SetUVRepeat(XMFLOAT2 repeat)
{
	if (repeat.x <= 0.0f)
	{
		repeat.x = 1.0f;
	}

	if (repeat.y <= 0.0f)
	{
		repeat.y = 1.0f;
	}

	m_UVRepeat =
		repeat;

	UpdateVertexUV();
}

void StageBlock::SetRainbow(bool isRainbow)
{
	m_IsRainbow =
		isRainbow;
}

void StageBlock::SetRainbowSpeed(float speed)
{
	if (speed <= 0.0f)
	{
		speed = 1.0f;
	}

	m_RainbowSpeed =
		speed;
}

void StageBlock::SetAlpha(float alpha)
{
	if (alpha < 0.0f)
	{
		alpha = 0.0f;
	}

	if (alpha > 1.0f)
	{
		alpha = 1.0f;
	}

	m_Alpha =
		alpha;
}

XMFLOAT3 StageBlock::GetRainbowColor(float time) const
{
	float r =
		sinf(time + 0.0f) *
		0.5f +
		0.5f;

	float g =
		sinf(time + 2.094f) *
		0.5f +
		0.5f;

	float b =
		sinf(time + 4.188f) *
		0.5f +
		0.5f;

	return XMFLOAT3(
		r,
		g,
		b
	);
}

void StageBlock::SetColor(
	float r,
	float g,
	float b,
	float a
)
{
	if (a < 0.0f)
	{
		a = 0.0f;
	}

	if (a > 1.0f)
	{
		a = 1.0f;
	}

	m_Color =
		XMFLOAT4(
			r,
			g,
			b,
			a
		);

	m_Alpha = a;
}