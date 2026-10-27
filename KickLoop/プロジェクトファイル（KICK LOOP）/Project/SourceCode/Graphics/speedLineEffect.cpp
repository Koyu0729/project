#include "main.h"
#include "speedLineEffect.h"

void SpeedLineEffect::Init()
{
	CreateQuad();

	m_BaseWidth = (float)SCREEN_WIDTH;
	m_BaseHeight = (float)SCREEN_HEIGHT;

	SetTexture("asset\\texture\\front.png");

	SetRect(
		0.0f,
		0.0f,
		m_BaseWidth,
		m_BaseHeight
	);

	SetAlpha(0.0f);
	SetVisible(false);

	m_SpeedRate = 0.0f;

	m_BoostAlpha = 0.0f;
	m_CurrentAlpha = 0.0f;

	m_MaxAlpha = 0.25f;
	m_BoostMaxAlpha = 0.40f;
	m_AlphaLimit = 0.70f;

	m_AlphaLerpSpeed = 0.08f;
	m_BoostDecay = 0.06f;

	m_MinRunAlpha = 0.10f;
	m_AppearSpeedRate = 0.08f;

	m_IsActiveEffect = false;

	m_CurrentScale = 1.0f;
	m_TargetScale = 1.0f;
	m_ScaleLerpSpeed = 0.12f;
}

void SpeedLineEffect::Uninit()
{
	UIQuadBase::Uninit();
}

void SpeedLineEffect::Update()
{
	m_CurrentScale +=
		(m_TargetScale - m_CurrentScale) *
		m_ScaleLerpSpeed;

	if (m_CurrentScale < 1.001f &&
		m_TargetScale == 1.0f)
	{
		m_CurrentScale = 1.0f;
	}

	UpdateScaleRect();

	if (!m_IsActiveEffect)
	{
		m_SpeedRate = 0.0f;
		m_BoostAlpha = 0.0f;

		m_CurrentAlpha +=
			(0.0f - m_CurrentAlpha) *
			m_AlphaLerpSpeed;

		m_CurrentAlpha = ClampAlpha(m_CurrentAlpha);

		if (m_CurrentAlpha < 0.01f)
		{
			m_CurrentAlpha = 0.0f;

			SetAlpha(0.0f);
			SetVisible(false);

			m_CurrentScale = 1.0f;
			m_TargetScale = 1.0f;

			UpdateScaleRect();

			return;
		}

		SetVisible(true);
		SetAlpha(m_CurrentAlpha);

		return;
	}

	// 速度がしきい値未満なら消す
	// ただしブースト中だけは表示する
	if (m_SpeedRate <= m_AppearSpeedRate &&
		m_BoostAlpha <= 0.0f)
	{
		m_CurrentAlpha +=
			(0.0f - m_CurrentAlpha) *
			m_AlphaLerpSpeed;

		m_CurrentAlpha = ClampAlpha(m_CurrentAlpha);

		if (m_CurrentAlpha < 0.01f)
		{
			m_CurrentAlpha = 0.0f;

			SetAlpha(0.0f);
			SetVisible(false);

			m_CurrentScale = 1.0f;
			m_TargetScale = 1.0f;

			UpdateScaleRect();

			return;
		}

		SetVisible(true);
		SetAlpha(m_CurrentAlpha);

		return;
	}

	float targetAlpha =
		m_SpeedRate *
		m_MaxAlpha;

	if (targetAlpha < m_MinRunAlpha)
	{
		targetAlpha = m_MinRunAlpha;
	}

	if (m_BoostAlpha > 0.0f)
	{
		targetAlpha += m_BoostAlpha;

		m_BoostAlpha -= m_BoostDecay;

		if (m_BoostAlpha < 0.0f)
		{
			m_BoostAlpha = 0.0f;
		}
	}

	if (targetAlpha > m_BoostMaxAlpha)
	{
		targetAlpha = m_BoostMaxAlpha;
	}

	targetAlpha = ClampAlpha(targetAlpha);

	m_CurrentAlpha +=
		(targetAlpha - m_CurrentAlpha) *
		m_AlphaLerpSpeed;

	m_CurrentAlpha = ClampAlpha(m_CurrentAlpha);

	if (m_CurrentAlpha < 0.01f)
	{
		m_CurrentAlpha = 0.0f;

		SetAlpha(0.0f);
		SetVisible(false);

		return;
	}

	SetVisible(true);
	SetAlpha(m_CurrentAlpha);
}

void SpeedLineEffect::Draw()
{
	SetDraw2D();
	DrawQuad();
}

void SpeedLineEffect::SetSpeedRate(float rate)
{
	if (rate < 0.0f)
	{
		rate = 0.0f;
	}

	if (rate > 1.0f)
	{
		rate = 1.0f;
	}

	m_SpeedRate = rate;
}

void SpeedLineEffect::PlayBoost()
{
	m_BoostAlpha = 0.45f;

	m_CurrentScale = 1.2f;
	m_TargetScale = 1.0f;

	UpdateScaleRect();
}

void SpeedLineEffect::SetActiveEffect(bool active)
{
	m_IsActiveEffect = active;
}

void SpeedLineEffect::SetAlphaLimit(float alphaLimit)
{
	if (alphaLimit < 0.0f)
	{
		alphaLimit = 0.0f;
	}

	if (alphaLimit > 0.70f)
	{
		alphaLimit = 0.70f;
	}

	m_AlphaLimit = alphaLimit;
}

void SpeedLineEffect::SetWallSlideEffect(
	bool isWallSlide,
	float wallNormalX
)
{
	// 呼び出し互換用。ここでは速度を変更しない
}

void SpeedLineEffect::SetRect(
	float x,
	float y,
	float w,
	float h
)
{
	VERTEX_3D vertex[4]{};

	vertex[0].Position = XMFLOAT3(x, y, 0.0f);
	vertex[1].Position = XMFLOAT3(x + w, y, 0.0f);
	vertex[2].Position = XMFLOAT3(x, y + h, 0.0f);
	vertex[3].Position = XMFLOAT3(x + w, y + h, 0.0f);

	vertex[0].TexCoord = XMFLOAT2(0.0f, 0.0f);
	vertex[1].TexCoord = XMFLOAT2(1.0f, 0.0f);
	vertex[2].TexCoord = XMFLOAT2(0.0f, 1.0f);
	vertex[3].TexCoord = XMFLOAT2(1.0f, 1.0f);

	for (int i = 0; i < 4; i++)
	{
		vertex[i].Normal = XMFLOAT3(0.0f, 0.0f, 0.0f);
		vertex[i].Diffuse = m_Color;
	}

	UpdateQuad(vertex);
}

void SpeedLineEffect::UpdateScaleRect()
{
	float width =
		m_BaseWidth *
		m_CurrentScale;

	float height =
		m_BaseHeight *
		m_CurrentScale;

	float x =
		((float)SCREEN_WIDTH - width) *
		0.5f;

	float y =
		((float)SCREEN_HEIGHT - height) *
		0.5f;

	SetRect(
		x,
		y,
		width,
		height
	);
}

float SpeedLineEffect::ClampAlpha(float alpha)
{
	if (alpha < 0.0f)
	{
		return 0.0f;
	}

	if (alpha > m_AlphaLimit)
	{
		return m_AlphaLimit;
	}

	return alpha;
}