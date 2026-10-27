#include "goalBuilding.h"

#include "manager.h"
#include "scene.h"
#include "player.h"

#include "stageBlock.h"
#include "stageBlockManager.h"

#include "renderer.h"
#include "modelRenderer.h"

void GoalBuilding::Init()
{
	BackgroundObject::Init();

	if (m_ModelRenderer != nullptr)
	{
		m_ModelRenderer->SetUVFlip(
			false,
			false
		);
	}

	LoadModel("asset\\model\\7Velocity.obj");

	m_BasePosition = Vector3(25.0f, 0.0f, 500.0f);

	m_PositionOffsetRate = 1.0f;

	m_StartScale = 8.0f;
	m_ScalePerDistance = 0.01f;
	m_MaxScale = 32.0f;

	m_IsLocked = false;

	m_ModelTopLocalY = 12.0f * 0.945f;

	m_Position = m_BasePosition;
	m_Scale = Vector3(m_StartScale, m_StartScale, m_StartScale);
	m_Rotation = Vector3(0.0f, 0.0f, 0.0f);

	SetActive(true);
	SetVisible(true);
}

void GoalBuilding::Update()
{
	SetActive(true);
	SetVisible(true);

	if (m_IsLocked)
	{
		return;
	}

	UpdateByPlayerProgress();
}

// 進行距離に合わせてゴール建物を配置する
void GoalBuilding::UpdateByPlayerProgress()
{
	Scene* scene = Manager::GetScene();

	if (scene == nullptr)
	{
		return;
	}

	Player* player = scene->GetGameObject<Player>();

	if (player == nullptr)
	{
		return;
	}

	float progress = player->GetProgressDistance();

	// ゴールが自動で進むのではなく、プレイヤー進行距離をZオフセットとして使う。
	m_Position = m_BasePosition;
	m_Position.z += progress * m_PositionOffsetRate;

	float scale = m_StartScale + progress * m_ScalePerDistance;

	if (scale > m_MaxScale)
	{
		scale = m_MaxScale;
	}

	if (scale < 0.01f)
	{
		scale = 0.01f;
	}

	m_Scale = Vector3(scale, scale, scale);
}

void GoalBuilding::CreateTopStageBlock()
{
	if (m_TopStageBlock != nullptr)
	{
		return;
	}

	Scene* scene = Manager::GetScene();

	if (scene == nullptr)
	{
		return;
	}

	Vector3 topPosition = GetTopPosition();
	Vector3 blockPosition = Vector3(topPosition.x, topPosition.y, topPosition.z);

	m_TopStageBlock = scene->AddGameObject<StageBlock>(3);

	if (m_TopStageBlock == nullptr)
	{
		return;
	}

	m_TopStageBlock->SetGoalTopBlock(true);

	m_TopStageBlock->SetPosition(blockPosition);
	m_TopStageBlock->SetSize(m_TopStageBlockSize);

	// 頂上床はゴール判定用なので、表示せず当たり判定だけ有効にする。
	m_TopStageBlock->SetActive(true);
	m_TopStageBlock->SetVisible(false);

	// ゴール床では壁アクションを発生させない。
	m_TopStageBlock->SetCanWallKick(false);
	m_TopStageBlock->SetCanWallSlide(false);
	m_TopStageBlock->SetCanClimb(false);
	m_TopStageBlock->SetCanAddWallKickGauge(false);

	StageBlockManager* stageBlockManager = scene->GetGameObject<StageBlockManager>();

	if (stageBlockManager != nullptr)
	{
		stageBlockManager->AddStageBlock(m_TopStageBlock);
	}
}

void GoalBuilding::SetBasePosition(Vector3 position)
{
	m_BasePosition = position;

	if (!m_IsLocked)
	{
		m_Position = m_BasePosition;
	}
}

void GoalBuilding::SetPositionOffsetRate(float rate)
{
	m_PositionOffsetRate = rate;
}

void GoalBuilding::SetStartScale(float scale)
{
	m_StartScale = scale;

	if (m_StartScale < 0.01f)
	{
		m_StartScale = 0.01f;
	}

	if (!m_IsLocked)
	{
		m_Scale = Vector3(m_StartScale, m_StartScale, m_StartScale);
	}
}

void GoalBuilding::SetScalePerDistance(float scalePerDistance)
{
	m_ScalePerDistance = scalePerDistance;
}

void GoalBuilding::SetMaxScale(float maxScale)
{
	m_MaxScale = maxScale;

	if (m_MaxScale < 0.01f)
	{
		m_MaxScale = 0.01f;
	}

	if (m_Scale.x > m_MaxScale)
	{
		m_Scale = Vector3(m_MaxScale, m_MaxScale, m_MaxScale);
	}
}

void GoalBuilding::SetModelTopLocalY(float localY)
{
	m_ModelTopLocalY = localY;

	if (m_ModelTopLocalY < 0.0f)
	{
		m_ModelTopLocalY = 0.0f;
	}
}

// ゴール演出中の建物位置を固定する
void GoalBuilding::LockCurrentPosition()
{
	m_IsLocked = true;

	SetActive(true);
	SetVisible(true);

	CreateTopStageBlock();
}

bool GoalBuilding::IsLocked() const
{
	return m_IsLocked;
}

Vector3 GoalBuilding::GetTopPosition() const
{
	return Vector3(
		m_Position.x,
		m_Position.y + m_ModelTopLocalY * m_Scale.y,
		m_Position.z
	);
}
