#include "jumpPad.h"

#include "renderer.h"
#include "modelRenderer.h"

#include "player.h"
#include "stageBlockManager.h"

#include "manager.h"
#include "scene.h"

#include "goalBuilding.h"

#include <cmath>

void JumpPad::Init()
{
	m_ModelRenderer = new ModelRenderer();

	ResetForReuse();
	LoadModel();

	SetActive(false);
	SetVisible(false);
}

void JumpPad::Uninit()
{
	if (m_ModelRenderer != nullptr)
	{
		delete m_ModelRenderer;
		m_ModelRenderer = nullptr;
	}
}

void JumpPad::ResetForReuse()
{
	m_HitCenter = Vector3(25.0f, 1.6f, 20.0f);
	m_HitSize = Vector3(2.5f, 0.5f, 2.5f);
	m_RotationY = 0.0f;

	m_ModelPath = "asset\\model\\jumpPad.obj";

	m_JumpVelocity = Vector3(0.0f, 1.6f, 5.0f);
	m_FloatPowerRate = 5.0f;

	m_CooldownFrame = 20;
	m_CurrentCooldownFrame = 0;

	m_UseGoalLaunch = false;
	m_IsGoalLaunchStarted = false;

	m_GoalTargetPosition = Vector3(25.0f, 180.0f, 5400.0f);
	m_GoalLaunchFrameMax = 120;
	m_GoalLaunchHeight = 60.0f;

	UpdateDrawScale();

	SetActive(false);
	SetVisible(false);
}

// 再利用までクールダウンと打ち上げ状態を止める
void JumpPad::SleepForPool()
{
	m_CurrentCooldownFrame = 0;

	m_UseGoalLaunch = false;
	m_IsGoalLaunchStarted = false;

	SetActive(false);
	SetVisible(false);
}

void JumpPad::LoadModel()
{
	if (m_ModelRenderer == nullptr)
	{
		return;
	}

	m_ModelRenderer->Load(m_ModelPath.c_str());
}

void JumpPad::Create(
	Scene* scene,
	Vector3 center,
	Vector3 size
)
{
	(void)scene;

	m_HitCenter = center;
	m_HitSize = size;

	if (m_HitSize.x <= 0.0f)
	{
		m_HitSize.x = 2.5f;
	}

	if (m_HitSize.y <= 0.0f)
	{
		m_HitSize.y = 0.5f;
	}

	if (m_HitSize.z <= 0.0f)
	{
		m_HitSize.z = 2.5f;
	}

	m_CurrentCooldownFrame = 0;
	m_IsGoalLaunchStarted = false;

	UpdateDrawScale();

	SetActive(true);
	SetVisible(true);
}

void JumpPad::Update()
{
	if (!Active())
	{
		return;
	}

	if (m_CurrentCooldownFrame > 0)
	{
		m_CurrentCooldownFrame--;
		return;
	}

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

	if (!IsHitPlayer(player))
	{
		return;
	}

	if (m_UseGoalLaunch)
	{
		StartGoalLaunch(player);

		m_CurrentCooldownFrame = m_CooldownFrame;
		return;
	}

	PlayerActionEffect effect = CreateActionEffect();

	player->AddActionEffect(effect);

	m_CurrentCooldownFrame = m_CooldownFrame;
}

void JumpPad::Draw()
{
	if (!Active())
	{
		return;
	}

	if (m_ModelRenderer == nullptr)
	{
		return;
	}

	XMMATRIX scale = XMMatrixScaling(m_DrawScale.x, m_DrawScale.y, m_DrawScale.z);
	XMMATRIX rotation = XMMatrixRotationRollPitchYaw(0.0f, m_RotationY, 0.0f);
	XMMATRIX translation = XMMatrixTranslation(m_HitCenter.x, m_HitCenter.y, m_HitCenter.z);
	XMMATRIX world = scale * rotation * translation;

	Renderer::SetWorldMatrix(world);

	m_ModelRenderer->Draw();
}

void JumpPad::UpdateDrawScale()
{
	m_DrawScale = Vector3(m_HitSize.x * 2.0f, m_HitSize.y * 2.0f, m_HitSize.z * 2.0f);
}

void JumpPad::AddMoveZ(float move)
{
	m_HitCenter.z -= move;
}

bool JumpPad::IsHitPlayer(Player* player)
{
	if (player == nullptr)
	{
		return false;
	}

	if (!Active())
	{
		return false;
	}

	return CheckOBB(
		m_HitCenter,
		m_HitSize,
		m_RotationY,
		player->GetPosition(),
		player->GetHitSize()
	);
}

bool JumpPad::CheckOBB(
	Vector3 obbCenter,
	Vector3 obbHalfSize,
	float obbRotationY,
	Vector3 playerCenter,
	Vector3 playerHalfSize
)
{
	Vector3 localPlayerCenter =
		WorldToLocalByYaw(
			playerCenter,
			obbCenter,
			obbRotationY
		);

	float cosY = cosf(obbRotationY);
	float sinY = sinf(obbRotationY);

	if (cosY < 0.0f)
	{
		cosY = -cosY;
	}

	if (sinY < 0.0f)
	{
		sinY = -sinY;
	}

	Vector3 localPlayerHalfSize;
	localPlayerHalfSize.x = playerHalfSize.x * cosY + playerHalfSize.z * sinY;
	localPlayerHalfSize.y = playerHalfSize.y;
	localPlayerHalfSize.z = playerHalfSize.x * sinY + playerHalfSize.z * cosY;

	if (localPlayerCenter.x + localPlayerHalfSize.x <= -obbHalfSize.x)
	{
		return false;
	}

	if (localPlayerCenter.x - localPlayerHalfSize.x >= obbHalfSize.x)
	{
		return false;
	}

	if (localPlayerCenter.y + localPlayerHalfSize.y <= -obbHalfSize.y)
	{
		return false;
	}

	if (localPlayerCenter.y - localPlayerHalfSize.y >= obbHalfSize.y)
	{
		return false;
	}

	if (localPlayerCenter.z + localPlayerHalfSize.z <= -obbHalfSize.z)
	{
		return false;
	}

	if (localPlayerCenter.z - localPlayerHalfSize.z >= obbHalfSize.z)
	{
		return false;
	}

	return true;
}

Vector3 JumpPad::WorldToLocalByYaw(
	Vector3 worldPosition,
	Vector3 center,
	float rotationY
)
{
	Vector3 relative;
	relative.x = worldPosition.x - center.x;
	relative.y = worldPosition.y - center.y;
	relative.z = worldPosition.z - center.z;

	float cosY = cosf(-rotationY);
	float sinY = sinf(-rotationY);

	Vector3 local;
	local.x = relative.x * cosY - relative.z * sinY;
	local.y = relative.y;
	local.z = relative.x * sinY + relative.z * cosY;

	return local;
}

Vector3 JumpPad::RotateVectorByYaw(
	Vector3 vector,
	float rotationY
)
{
	float cosY = cosf(rotationY);
	float sinY = sinf(rotationY);

	Vector3 result;
	result.x = vector.x * cosY + vector.z * sinY;
	result.y = vector.y;
	result.z = -vector.x * sinY + vector.z * cosY;

	return result;
}

// ジャンプ台の効果をプレイヤーへ渡す形にする
PlayerActionEffect JumpPad::CreateActionEffect()
{
	PlayerActionEffect effect;

	effect.effectType = PlayerActionEffectType::JumpPad;
	effect.effectKey = "";

	Vector3 worldVelocity = RotateVectorByYaw(m_JumpVelocity, m_RotationY);

	effect.moveVelocity = Vector3(worldVelocity.x, 0.0f, worldVelocity.z);
	effect.moveMode = PlayerActionMoveMode::Decay;
	effect.moveDecay = CalculateMoveDecay(worldVelocity);
	effect.moveMin = 0.01f;
	effect.moveFrame = 0;

	effect.useStartVerticalVelocity = true;
	effect.startVerticalVelocity = worldVelocity.y;
	effect.hasAppliedStartVerticalVelocity = false;

	ApplyFloatByVelocity(
		effect,
		worldVelocity
	);

	effect.requestBoostEffect = true;
	effect.stopOnGround = true;
	effect.wasGround = false;
	effect.isActive = true;

	return effect;
}

// 射出速度に合わせて滞空時間と重力を調整する
void JumpPad::ApplyFloatByVelocity(
	PlayerActionEffect& effect,
	Vector3 velocity
)
{
	float absX = fabsf(velocity.x);
	float absY = fabsf(velocity.y);
	float absZ = fabsf(velocity.z);

	float power = sqrtf(absX * absX + absY * absY + absZ * absZ);

	if (power <= 0.001f)
	{
		effect.floatFrame = 0;
		effect.useGravityRate = false;
		effect.gravityRate = 1.0f;
		effect.useMaxFallSpeed = false;
		effect.maxFallSpeed = -999.0f;

		return;
	}

	float upRate = absY / power;
	float forwardRate = absZ / power;
	float sideRate = absX / power;

	float floatFrameValue = 12.0f + power * m_FloatPowerRate * (1.0f + upRate * 1.4f + forwardRate * 0.4f + sideRate * 0.2f);

	if (floatFrameValue < 8.0f)
	{
		floatFrameValue = 8.0f;
	}

	if (floatFrameValue > 90.0f)
	{
		floatFrameValue = 90.0f;
	}

	effect.floatFrame = (int)floatFrameValue;
	effect.useGravityRate = true;
	effect.gravityRate = 0.80f - upRate * 0.48f - forwardRate * 0.16f;

	if (effect.gravityRate < 0.18f)
	{
		effect.gravityRate = 0.18f;
	}

	if (effect.gravityRate > 0.85f)
	{
		effect.gravityRate = 0.85f;
	}

	effect.useMaxFallSpeed = true;
	effect.maxFallSpeed = -0.55f + upRate * 0.38f + forwardRate * 0.08f;

	if (effect.maxFallSpeed > -0.10f)
	{
		effect.maxFallSpeed = -0.10f;
	}

	if (effect.maxFallSpeed < -0.60f)
	{
		effect.maxFallSpeed = -0.60f;
	}
}

float JumpPad::CalculateMoveDecay(Vector3 velocity)
{
	float absX = fabsf(velocity.x);
	float absY = fabsf(velocity.y);
	float absZ = fabsf(velocity.z);

	float power = sqrtf(absX * absX + absY * absY + absZ * absZ);

	if (power <= 0.001f)
	{
		return 0.94f;
	}

	float forwardRate = absZ / power;
	float upRate = absY / power;
	float decay = 0.90f + forwardRate * 0.075f + upRate * 0.02f;

	if (decay > 0.992f)
	{
		decay = 0.992f;
	}

	if (decay < 0.88f)
	{
		decay = 0.88f;
	}

	return decay;
}

void JumpPad::StartGoalLaunch(Player* player)
{
	if (player == nullptr)
	{
		return;
	}

	if (m_IsGoalLaunchStarted)
	{
		return;
	}

	if (m_GoalLaunchFrameMax < 1)
	{
		m_GoalLaunchFrameMax = 1;
	}

	if (m_GoalLaunchHeight < 0.0f)
	{
		m_GoalLaunchHeight = 0.0f;
	}

	m_IsGoalLaunchStarted = true;

	Vector3 goalTargetPosition = m_GoalTargetPosition;
	Scene* scene = Manager::GetScene();

	if (scene != nullptr)
	{
		StageBlockManager* stageBlockManager = scene->GetGameObject<StageBlockManager>();

		if (stageBlockManager != nullptr)
		{
			stageBlockManager->ReleaseGoalZClamp();
		}

		GoalBuilding* goalBuilding = scene->GetGameObject<GoalBuilding>();

		if (goalBuilding != nullptr)
		{
			goalTargetPosition = goalBuilding->GetTopPosition();
		}
	}

	player->StartGoalLaunch(
		goalTargetPosition,
		m_GoalLaunchFrameMax,
		m_GoalLaunchHeight
	);
}

void JumpPad::SetHitOBB(
	Vector3 center,
	Vector3 size,
	float rotationY
)
{
	m_HitCenter = center;
	m_HitSize = size;
	m_RotationY = rotationY;

	if (m_HitSize.x <= 0.0f)
	{
		m_HitSize.x = 2.5f;
	}

	if (m_HitSize.y <= 0.0f)
	{
		m_HitSize.y = 0.5f;
	}

	if (m_HitSize.z <= 0.0f)
	{
		m_HitSize.z = 2.5f;
	}

	UpdateDrawScale();
}

void JumpPad::SetHitAABB(
	Vector3 center,
	Vector3 size
)
{
	SetHitOBB(
		center,
		size,
		0.0f
	);
}

Vector3 JumpPad::GetHitCenter()
{
	return m_HitCenter;
}

Vector3 JumpPad::GetHitSize()
{
	return m_HitSize;
}

float JumpPad::GetRotationY()
{
	return m_RotationY;
}

void JumpPad::SetModelPath(std::string modelPath)
{
	m_ModelPath = modelPath;

	LoadModel();
}

void JumpPad::SetDrawScale(Vector3 drawScale)
{
	m_DrawScale = drawScale;

	if (m_DrawScale.x <= 0.001f)
	{
		m_DrawScale.x = 0.001f;
	}

	if (m_DrawScale.y <= 0.001f)
	{
		m_DrawScale.y = 0.001f;
	}

	if (m_DrawScale.z <= 0.001f)
	{
		m_DrawScale.z = 0.001f;
	}
}

void JumpPad::SetJumpVelocity(Vector3 jumpVelocity)
{
	m_JumpVelocity = jumpVelocity;
}

Vector3 JumpPad::GetJumpVelocity()
{
	return m_JumpVelocity;
}

void JumpPad::SetFloatPowerRate(float floatPowerRate)
{
	m_FloatPowerRate = floatPowerRate;

	if (m_FloatPowerRate < 0.0f)
	{
		m_FloatPowerRate = 0.0f;
	}
}

float JumpPad::GetFloatPowerRate()
{
	return m_FloatPowerRate;
}

void JumpPad::SetCooldownFrame(int cooldownFrame)
{
	if (cooldownFrame < 1)
	{
		cooldownFrame = 1;
	}

	m_CooldownFrame = cooldownFrame;
}

void JumpPad::SetRotationY(float rotationY)
{
	m_RotationY = rotationY;
}

void JumpPad::SetGoalLaunch(
	bool useGoalLaunch,
	Vector3 targetPosition,
	int frameMax,
	float height
)
{
	m_UseGoalLaunch = useGoalLaunch;
	m_GoalTargetPosition = targetPosition;

	if (frameMax < 1)
	{
		frameMax = 1;
	}

	m_GoalLaunchFrameMax = frameMax;

	if (height < 0.0f)
	{
		height = 0.0f;
	}

	m_GoalLaunchHeight = height;
	m_IsGoalLaunchStarted = false;
}

void JumpPad::SetActive(bool isActive)
{
	GameObject::SetActive(isActive);
}

bool JumpPad::IsActive()
{
	return Active();
}
