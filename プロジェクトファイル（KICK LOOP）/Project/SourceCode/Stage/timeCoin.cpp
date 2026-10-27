#include "timeCoin.h"

#include "manager.h"
#include "scene.h"
#include "renderer.h"
#include "modelRenderer.h"

#include "player.h"
#include "gameTime.h"
#include "stageBlockManager.h"

#include <cmath>

void TimeCoin::Init()
{
	if (m_ModelRenderer != nullptr)
	{
		delete m_ModelRenderer;
		m_ModelRenderer = nullptr;
	}

	m_ModelRenderer = new ModelRenderer();

	Renderer::CreateVertexShader(
		&m_VertexShader,
		&m_VertexLayout,
		"shader\\unlitTextureVS.cso"
	);

	Renderer::CreatePixelShader(
		&m_PixelShader,
		"shader\\unlitTexturePS.cso"
	);

	m_ModelPath = "asset\\model\\TimeCoin.obj";

	LoadModel();

	m_HitCenter = Vector3(0.0f, 0.0f, 0.0f);
	m_HitSize = Vector3(0.7f, 0.7f, 0.7f);

	m_Position = m_HitCenter;
	m_Scale = Vector3(m_HitSize.x, m_HitSize.y, m_HitSize.z);
	m_Rotation = Vector3(0.0f, 0.0f, 0.0f);

	m_AddTime = 5.0f;
	m_RotationSpeedY = 0.04f;

	// StageBlockManager側のCreate()で有効化するため、初期状態ではPool待機にする。
	SetActive(false);
	SetVisible(false);
}

void TimeCoin::Uninit()
{
	if (m_ModelRenderer != nullptr)
	{
		delete m_ModelRenderer;
		m_ModelRenderer = nullptr;
	}

	if (m_VertexLayout != nullptr)
	{
		m_VertexLayout->Release();
		m_VertexLayout = nullptr;
	}

	if (m_VertexShader != nullptr)
	{
		m_VertexShader->Release();
		m_VertexShader = nullptr;
	}

	if (m_PixelShader != nullptr)
	{
		m_PixelShader->Release();
		m_PixelShader = nullptr;
	}

	m_ModelPath.clear();
}

void TimeCoin::Update()
{
	m_Rotation.y += m_RotationSpeedY;

	UpdateHit();
}

void TimeCoin::Draw()
{
	if (m_ModelRenderer == nullptr)
	{
		return;
	}

	if (m_ModelPath.empty())
	{
		return;
	}

	Renderer::GetDeviceContext()->IASetInputLayout(m_VertexLayout);

	Renderer::GetDeviceContext()->VSSetShader(
		m_VertexShader,
		nullptr,
		0
	);

	Renderer::GetDeviceContext()->PSSetShader(
		m_PixelShader,
		nullptr,
		0
	);

	XMMATRIX scale = XMMatrixScaling(m_Scale.x, m_Scale.y, m_Scale.z);
	XMMATRIX rotation = XMMatrixRotationRollPitchYaw(m_Rotation.x, m_Rotation.y, m_Rotation.z);
	XMMATRIX translation = XMMatrixTranslation(m_Position.x, m_Position.y, m_Position.z);
	XMMATRIX world = scale * rotation * translation;

	Renderer::SetWorldMatrix(world);

	m_ModelRenderer->Draw();
}

// 再利用するコインを指定位置に配置する
void TimeCoin::Create(
	Vector3 position,
	Vector3 size
)
{
	m_HitCenter = position;
	m_HitSize = size;

	NormalizeHitSize();

	m_Position = m_HitCenter;
	m_Scale = Vector3(m_HitSize.x, m_HitSize.y, m_HitSize.z);
	m_Rotation = Vector3(0.0f, 0.0f, 0.0f);

	SetActive(true);
	SetVisible(true);
}

void TimeCoin::SetAddTime(float addTime)
{
	m_AddTime = addTime;

	if (m_AddTime < 0.0f)
	{
		m_AddTime = 0.0f;
	}
}

Vector3 TimeCoin::GetHitCenter() const
{
	return m_HitCenter;
}

Vector3 TimeCoin::GetHitSize() const
{
	return m_HitSize;
}

void TimeCoin::LoadModel()
{
	if (m_ModelRenderer == nullptr)
	{
		return;
	}

	if (m_ModelPath.empty())
	{
		return;
	}

	m_ModelRenderer->Load(m_ModelPath.c_str());
}

// 取得時に時間を加算してコインを回収する
void TimeCoin::UpdateHit()
{
	if (!IsHitPlayer())
	{
		return;
	}

	Scene* scene = Manager::GetScene();

	if (scene == nullptr)
	{
		return;
	}

	Player* player = scene->GetGameObject<Player>();

	if (player != nullptr)
	{
		player->GetSoundController().PlayCoinGet();
	}

	Time* time = scene->GetGameObject<Time>();

	if (time != nullptr)
	{
		time->AddLimitTime(m_AddTime);
	}

	RecordCollectedTimeCoin();

	// Destroyせず非アクティブに戻し、StageBlockManager側のPoolで再利用する。
	SetActive(false);
	SetVisible(false);
}

bool TimeCoin::IsHitPlayer() const
{
	if (!Active())
	{
		return false;
	}

	Scene* scene = Manager::GetScene();

	if (scene == nullptr)
	{
		return false;
	}

	Player* player = scene->GetGameObject<Player>();

	if (player == nullptr)
	{
		return false;
	}

	Vector3 playerPosition = player->GetPosition();
	Vector3 playerHitSize = player->GetHitSize();

	float dx = fabsf(playerPosition.x - m_HitCenter.x);
	float dy = fabsf(playerPosition.y - m_HitCenter.y);
	float dz = fabsf(playerPosition.z - m_HitCenter.z);

	if (dx > playerHitSize.x + m_HitSize.x)
	{
		return false;
	}

	if (dy > playerHitSize.y + m_HitSize.y)
	{
		return false;
	}

	if (dz > playerHitSize.z + m_HitSize.z)
	{
		return false;
	}

	return true;
}

void TimeCoin::NormalizeHitSize()
{
	if (m_HitSize.x <= 0.0f)
	{
		m_HitSize.x = 0.7f;
	}

	if (m_HitSize.y <= 0.0f)
	{
		m_HitSize.y = 0.7f;
	}

	if (m_HitSize.z <= 0.0f)
	{
		m_HitSize.z = 0.7f;
	}
}

// 再構築時に復元する取得情報を記録する
void TimeCoin::RecordCollectedTimeCoin()
{
	Scene* scene = Manager::GetScene();

	if (scene == nullptr)
	{
		return;
	}

	StageBlockManager* stageBlockManager = scene->GetGameObject<StageBlockManager>();

	if (stageBlockManager == nullptr)
	{
		return;
	}

	stageBlockManager->AddCollectedTimeCoinRecord(
		m_HitCenter,
		m_HitSize,
		m_AddTime
	);
}
