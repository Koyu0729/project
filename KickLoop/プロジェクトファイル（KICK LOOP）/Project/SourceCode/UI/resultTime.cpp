#include "main.h"
#include "resultTime.h"

#include "renderer.h"
#include "gameData.h"

#include <cstring>
#include <cstdio>
#include <cmath>

void ResultTime::Init()
{
	CreateQuad();

	SetTexture("asset\\texture\\configTime.png");

	SetSpriteSheet(
		SPRITE_DIVIDE_X,
		SPRITE_DIVIDE_Y
	);

	m_PlayedStartSE = false;

	m_RotateWaveTimer = 0.0f;
	m_RotateSpeed = 8.0f;
	m_NextStartAngle = 1.5708f;

	m_IsRotateWait = false;
	m_RotateWaitTimer = 0.0f;
	m_RotateWaitTime = 3.0f;
}

void ResultTime::Uninit()
{
	UISpriteSheetBase::Uninit();
}

void ResultTime::Update()
{
	char buffer[16]{};
	sprintf_s(buffer, "%.1fm", GameData::ResultDistance);

	int scoreLen = SCORE_TEXT_COUNT;
	int distanceLen = (int)strlen(buffer);

	int totalSpriteCount = scoreLen + distanceLen;

	if (totalSpriteCount <= 0)
	{
		return;
	}

	float dt = 1.0f / 60.0f;

	float oneRotationAngle = 6.2831853f;
	float oneRotationTime = oneRotationAngle / m_RotateSpeed;

	float nextStartDelay = m_NextStartAngle / m_RotateSpeed;

	float totalRotateTime =
		(totalSpriteCount - 1) *
		nextStartDelay +
		oneRotationTime;

	if (m_IsRotateWait)
	{
		m_RotateWaitTimer += dt;

		if (m_RotateWaitTimer >= m_RotateWaitTime)
		{
			m_RotateWaitTimer = 0.0f;
			m_RotateWaveTimer = 0.0f;
			m_IsRotateWait = false;
		}

		return;
	}

	m_RotateWaveTimer += dt;

	if (m_RotateWaveTimer >= totalRotateTime)
	{
		m_RotateWaveTimer = totalRotateTime;
		m_IsRotateWait = true;
		m_RotateWaitTimer = 0.0f;
	}
}

void ResultTime::Draw()
{
	SetDraw3D();

	XMMATRIX world = XMMatrixIdentity();
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

	float digitSize = 1.5f;
	float digitSpace = 1.1f;

	Vector3 basePosition = Vector3(25.0f, 4.0f, 35.0f);

	int spriteIndex = 0;

	float oneRotationAngle = 6.2831853f;
	float oneRotationTime = oneRotationAngle / m_RotateSpeed;
	float nextStartDelay = m_NextStartAngle / m_RotateSpeed;

	{
		int scoreIndex[SCORE_TEXT_COUNT] =
		{
			INDEX_S,
			INDEX_C,
			INDEX_O,
			INDEX_R,
			INDEX_E,
			INDEX_COLON
		};

		float totalWidth = SCORE_TEXT_COUNT * digitSpace;
		float startX = basePosition.x - totalWidth * 0.5f;

		for (int i = 0; i < SCORE_TEXT_COUNT; i++)
		{
			Vector3 drawPosition = Vector3(
				startX + i * digitSpace,
				basePosition.y + 2.0f,
				basePosition.z
			);

			float angleY = 0.0f;

			if (!m_IsRotateWait)
			{
				float startTime = spriteIndex * nextStartDelay;
				float rotateTime = m_RotateWaveTimer - startTime;

				if (rotateTime > 0.0f &&
					rotateTime < oneRotationTime)
				{
					angleY = rotateTime * m_RotateSpeed;
				}
			}

			DrawDigitWorld(
				scoreIndex[i],
				drawPosition,
				digitSize,
				angleY
			);

			spriteIndex++;
		}
	}

	{
		char buffer[16]{};
		sprintf_s(buffer, "%.1fm", GameData::ResultDistance);

		int len = (int)strlen(buffer);

		float totalWidth = len * digitSpace;
		float startX = basePosition.x - totalWidth * 0.5f;

		for (int i = 0; i < len; i++)
		{
			int index = GetSpriteIndex(buffer[i]);

			if (index < 0)
			{
				continue;
			}

			Vector3 drawPosition = Vector3(
				startX + i * digitSpace,
				basePosition.y,
				basePosition.z
			);

			float angleY = 0.0f;

			if (!m_IsRotateWait)
			{
				float startTime = spriteIndex * nextStartDelay;
				float rotateTime = m_RotateWaveTimer - startTime;

				if (rotateTime > 0.0f &&
					rotateTime < oneRotationTime)
				{
					angleY = rotateTime * m_RotateSpeed;
				}
			}

			DrawDigitWorld(
				index,
				drawPosition,
				digitSize,
				angleY
			);

			spriteIndex++;
		}
	}

	Renderer::GetDeviceContext()->RSSetState(nullptr);

	if (rasterizerState)
	{
		rasterizerState->Release();
		rasterizerState = nullptr;
	}
}

void ResultTime::DrawDigitWorld(
	int index,
	Vector3 position,
	float size,
	float angleY
)
{
	if (index < 0)
	{
		return;
	}

	float tw = 1.0f;
	float th = 1.0f / (float)SPRITE_DIVIDE_Y;

	float tx = 0.0f;
	float ty = (float)index * th;

	float halfW = size * 0.5f;
	float halfH = size * 0.5f;

	float centerX = position.x + halfW;
	float centerY = position.y + halfH;
	float centerZ = position.z;

	float cosY = cosf(angleY);
	float sinY = sinf(angleY);

	float localX[4] =
	{
		-halfW,
		 halfW,
		-halfW,
		 halfW
	};

	float localY[4] =
	{
		 halfH,
		 halfH,
		-halfH,
		-halfH
	};

	VERTEX_3D vertex[4]{};

	for (int i = 0; i < 4; i++)
	{
		vertex[i].Position = XMFLOAT3(
			centerX + localX[i] * cosY,
			centerY + localY[i],
			centerZ - localX[i] * sinY
		);

		vertex[i].Normal = XMFLOAT3(0.0f, 0.0f, -1.0f);
		vertex[i].Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	}

	vertex[0].TexCoord = XMFLOAT2(tx, ty);
	vertex[1].TexCoord = XMFLOAT2(tx + tw, ty);
	vertex[2].TexCoord = XMFLOAT2(tx, ty + th);
	vertex[3].TexCoord = XMFLOAT2(tx + tw, ty + th);

	UpdateQuad(vertex);
	DrawQuad();
}

int ResultTime::GetSpriteIndex(char c) const
{
	if (c == '1') return INDEX_1;
	if (c == '2') return INDEX_2;
	if (c == '3') return INDEX_3;
	if (c == '4') return INDEX_4;
	if (c == '5') return INDEX_5;
	if (c == '6') return INDEX_6;
	if (c == '7') return INDEX_7;
	if (c == '8') return INDEX_8;
	if (c == '9') return INDEX_9;
	if (c == '0') return INDEX_0;
	if (c == '.') return INDEX_DOT;
	if (c == 'm') return INDEX_M;

	return -1;
}