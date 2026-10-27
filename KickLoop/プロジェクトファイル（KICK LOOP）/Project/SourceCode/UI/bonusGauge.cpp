#include "bonusGauge.h"

#include "manager.h"
#include "scene.h"
#include "player.h"

void BonusGaugeUI::Init()
{
	CreateQuad();

	SetTexture("asset\\texture\\frameBoxR.png");

	m_MaxGauge = 3.0f;
	m_CurrentGauge = 0.0f;
	m_DisplayGauge = 0.0f;

	m_BaseX = 80.0f;
	m_BottomY = 520.0f;
	m_Width = 48.0f;
	m_Height = 270.0f;

	m_DisplayLerpRate = 0.20f;

	SetVisible(true);
}

void BonusGaugeUI::Uninit()
{
	UIQuadBase::Uninit();
}

void BonusGaugeUI::Update()
{
	Player* player = Manager::GetScene()->GetGameObject<Player>();

	if (player == nullptr)
	{
		SetGauge(0.0f);
		return;
	}

	SetGauge(player->GetBonusGauge());

	m_DisplayGauge +=
		(m_CurrentGauge - m_DisplayGauge) *
		m_DisplayLerpRate;

	if (m_DisplayGauge < 0.001f)
	{
		m_DisplayGauge = 0.0f;
	}
}

void BonusGaugeUI::Draw()
{
	SetDraw2D();

	DrawGauge();
}

void BonusGaugeUI::SetGauge(float gauge)
{
	m_CurrentGauge = ClampGauge(gauge);
}

void BonusGaugeUI::AddGauge(float gauge)
{
	m_CurrentGauge = ClampGauge(
		m_CurrentGauge + gauge
	);
}

bool BonusGaugeUI::ConsumeGauge(float gauge)
{
	if (m_CurrentGauge < gauge)
	{
		return false;
	}

	m_CurrentGauge = ClampGauge(
		m_CurrentGauge - gauge
	);

	return true;
}

float BonusGaugeUI::GetGauge() const
{
	return m_CurrentGauge;
}

float BonusGaugeUI::GetMaxGauge() const
{
	return m_MaxGauge;
}

bool BonusGaugeUI::IsFull() const
{
	return m_CurrentGauge >= m_MaxGauge;
}

void BonusGaugeUI::SetBasePosition(float x, float bottomY)
{
	m_BaseX = x;
	m_BottomY = bottomY;
}

void BonusGaugeUI::SetSize(float width, float height)
{
	m_Width = width;
	m_Height = height;
}

void BonusGaugeUI::DrawGauge()
{
	float rate = 0.0f;

	if (m_MaxGauge > 0.0f)
	{
		rate = m_DisplayGauge / m_MaxGauge;
	}

	if (rate < 0.0f)
	{
		rate = 0.0f;
	}

	if (rate > 1.0f)
	{
		rate = 1.0f;
	}

	float x = m_BaseX;
	float topY = m_BottomY - m_Height;

	float fillHeight = m_Height * rate;
	float fillY = m_BottomY - fillHeight;

	// äOòg
	DrawRect(
		x - 3.0f,
		topY - 3.0f,
		m_Width + 6.0f,
		m_Height + 6.0f,
		XMFLOAT4(1.0f, 1.0f, 1.0f, 0.35f)
	);

	// îwåi
	DrawRect(
		x,
		topY,
		m_Width,
		m_Height,
		XMFLOAT4(0.02f, 0.02f, 0.02f, 0.70f)
	);

	// íÜêg â∫Ç©ÇÁè„
	if (fillHeight > 0.0f)
	{
		DrawRect(
			x,
			fillY,
			m_Width,
			fillHeight,
			XMFLOAT4(0.2f, 0.8f, 1.0f, 0.95f)
		);
	}

	float oneHeight = m_Height / 3.0f;

	// ãÊêÿÇËê¸1
	DrawRect(
		x,
		m_BottomY - oneHeight,
		m_Width,
		2.0f,
		XMFLOAT4(1.0f, 1.0f, 1.0f, 0.45f)
	);

	// ãÊêÿÇËê¸2
	DrawRect(
		x,
		m_BottomY - oneHeight * 2.0f,
		m_Width,
		2.0f,
		XMFLOAT4(1.0f, 1.0f, 1.0f, 0.45f)
	);
}

void BonusGaugeUI::DrawRect(
	float x,
	float y,
	float width,
	float height,
	XMFLOAT4 color
)
{
	MATERIAL material{};
	material.Diffuse = color;
	material.TextureEnable = true;
	Renderer::SetMaterial(material);

	VERTEX_3D vertex[4]{};

	vertex[0].Position = XMFLOAT3(x, y, 0.0f);
	vertex[1].Position = XMFLOAT3(x + width, y, 0.0f);
	vertex[2].Position = XMFLOAT3(x, y + height, 0.0f);
	vertex[3].Position = XMFLOAT3(x + width, y + height, 0.0f);

	vertex[0].TexCoord = XMFLOAT2(0.0f, 0.0f);
	vertex[1].TexCoord = XMFLOAT2(1.0f, 0.0f);
	vertex[2].TexCoord = XMFLOAT2(0.0f, 1.0f);
	vertex[3].TexCoord = XMFLOAT2(1.0f, 1.0f);

	for (int i = 0; i < 4; i++)
	{
		vertex[i].Normal = XMFLOAT3(0.0f, 1.0f, 0.0f);
		vertex[i].Diffuse = color;
	}

	UpdateQuad(vertex);
	DrawQuad();
}

float BonusGaugeUI::ClampGauge(float gauge)
{
	if (gauge < 0.0f)
	{
		return 0.0f;
	}

	if (gauge > m_MaxGauge)
	{
		return m_MaxGauge;
	}

	return gauge;
}