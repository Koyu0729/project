#include "main.h"
#include "GameData.h"

#include "renderer.h"
#include "camera.h"
#include "player.h"
#include "manager.h"
#include "scene.h"

#include "mouseInput.h"

#include "stageBlockManager.h"
#include "stageBlock.h"

#include <cmath>

void Camera::Init()
{
	m_Position = Vector3(0.0f, 1.0f, -5.0f);
	m_Target = Vector3(0.0f, 1.0f, 0.0f);

	m_CurrentEyeHeight = m_Parameter.eyeHeight;

	m_Yaw = 0.0f;
	m_Pitch = 0.25f;

	m_BaseFOV = m_Parameter.normalFOV;
	m_FOV = m_Parameter.normalFOV;
	m_FOVImpact = 0.0f;

	m_Roll = 0.0f;
	m_LastWallSlideRollSide = 0.0f;
	m_WallSlideRollKeepFrame = 0;

	m_ShakeOffset = Vector3(0.0f, 0.0f, 0.0f);

	m_RunShakeTime = 0.0f;
	m_WallSlideShakeTime = 0.0f;

	m_WallSlidePositionOffset = 0.0f;
	m_WallSlidePositionOffsetPower = -2.0f;
	m_WallSlidePositionOffsetLerpRate = 0.15f;

	m_ImpactShakePower = 0.0f;

	m_FallShakeTimer = 0.0f;
	m_FallShakePower = 0.0f;

	m_WallKickPitchImpact = 0.0f;
	m_WallKickRollImpact = 0.0f;

	m_CameraFOV = CameraFOV::Normal;

	m_GoalCamera = GoalCameraData();
	m_IsGoalCameraFinished = false;

	SyncGameObjectRotation(m_Pitch);
}

void Camera::Uninit()
{
	m_GoalCamera.isActive = false;
	m_GoalCamera.mode = GoalCameraMode::Default;
	m_IsGoalCameraFinished = false;
}

void Camera::Update()
{
	if (m_GoalCamera.isActive)
	{
		UpdateGoalCameraSequence();

		Scene* scene = Manager::GetScene();

		if (scene != nullptr)
		{
			StageBlockManager* stageBlockManager =
				scene->GetGameObject<StageBlockManager>();

			if (stageBlockManager != nullptr)
			{
				stageBlockManager->UpdateGoalCameraStage(
					m_Position.z
				);
			}
		}

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

	Vector3 playerPos = player->GetPosition();

	m_Yaw += Mouse::DeltaX() * m_Parameter.mouseSensitivity * GameData::MouseSensitivity;
	m_Pitch += Mouse::DeltaY() * m_Parameter.mouseSensitivity * GameData::MouseSensitivity;

	ClampPitch();

	UpdateFallCamera(player);

	ClampPitch();

	UpdateWallKickPitchImpact();

	UpdateFOV(player);

	UpdateCrouchEyeHeight(player);

	Vector3 playerEyePosition = playerPos + Vector3(0.0f, m_CurrentEyeHeight, 0.0f);
	Vector3 basePosition = playerEyePosition;

	float cameraPitch = m_Pitch + m_WallKickPitchImpact;
	cameraPitch = ClampFloat(cameraPitch, m_Parameter.pitchMin, m_Parameter.pitchMax);

	Vector3 forward = BuildForward(cameraPitch);

	SyncGameObjectRotation(cameraPitch);

	UpdateWallSlidePositionOffset(
		player,
		forward,
		basePosition
	);

	UpdateCameraShake(
		player,
		forward
	);

	Vector3 desiredPosition = basePosition + m_ShakeOffset;

	if (IsCameraHitBlockByPlayerHit(
		desiredPosition,
		player->GetHitSize()
	))
	{
		m_Position = playerEyePosition;
	}
	else
	{
		m_Position = desiredPosition;
	}

	m_Target = m_Position + forward;

	UpdateWallSlideRoll(
		player,
		forward
	);

	SyncGameObjectRotation(cameraPitch);

	if (player->IsWallKick())
	{
		PlayWallKickFOVImpact();
		player->SetWallKick(false);
	}
}

void Camera::Draw()
{
	m_Projection = XMMatrixPerspectiveFovLH(
		m_FOV,
		(float)SCREEN_WIDTH / (float)SCREEN_HEIGHT,
		m_Parameter.nearClip,
		m_Parameter.farClip
	);

	Renderer::SetProjectionMatrix(m_Projection);

	XMVECTOR eye = XMLoadFloat3((XMFLOAT3*)&m_Position);
	XMVECTOR target = XMLoadFloat3((XMFLOAT3*)&m_Target);

	XMVECTOR forward = XMVector3Normalize(target - eye);

	XMVECTOR baseUp = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
	XMMATRIX rollMatrix = XMMatrixRotationAxis(forward, m_Roll);
	XMVECTOR up = XMVector3TransformNormal(baseUp, rollMatrix);

	m_View = XMMatrixLookAtLH(
		eye,
		target,
		up
	);

	Renderer::SetViewMatrix(m_View);
}

// ゴール用のカメラ演出を開始する
void Camera::StartGoalCameraSequence(
	Vector3 playerStartPosition,
	Vector3 goalPosition,
	Vector3 goalBlockPosition
)
{
	m_GoalCamera = GoalCameraData();

	m_GoalCamera.startPosition = playerStartPosition;
	m_GoalCamera.startPosition.y = playerStartPosition.y + m_GoalCamera.heightOffset;

	m_GoalCamera.endPosition = goalPosition;
	m_GoalCamera.endPosition.y = m_GoalCamera.startPosition.y;

	m_GoalCamera.goalBlockPosition = goalBlockPosition;

	m_GoalCamera.moveDirection.x = m_GoalCamera.endPosition.x - m_GoalCamera.startPosition.x;
	m_GoalCamera.moveDirection.y = 0.0f;
	m_GoalCamera.moveDirection.z = m_GoalCamera.endPosition.z - m_GoalCamera.startPosition.z;

	float moveLength = sqrtf(
		m_GoalCamera.moveDirection.x * m_GoalCamera.moveDirection.x +
		m_GoalCamera.moveDirection.z * m_GoalCamera.moveDirection.z
	);

	if (moveLength < 0.001f)
	{
		m_GoalCamera.moveDirection = Vector3(0.0f, 0.0f, 1.0f);
	}
	else
	{
		m_GoalCamera.moveDirection.x /= moveLength;
		m_GoalCamera.moveDirection.z /= moveLength;
	}

	m_Position = m_GoalCamera.startPosition;

	m_Target.x = m_Position.x + m_GoalCamera.moveDirection.x * m_GoalCamera.lookAhead;
	m_Target.y = m_Position.y - m_GoalCamera.targetDownOffset;
	m_Target.z = m_Position.z + m_GoalCamera.moveDirection.z * m_GoalCamera.lookAhead;

	m_GoalCamera.isActive = true;
	m_GoalCamera.mode = GoalCameraMode::Default;
	m_GoalCamera.lastRate = 0.0f;

	m_Roll = 0.0f;
	m_ShakeOffset = Vector3(0.0f, 0.0f, 0.0f);

	m_FOV = m_Parameter.normalFOV;
	m_BaseFOV = m_Parameter.normalFOV;
	m_FOVImpact = 0.0f;

	m_WallKickPitchImpact = 0.0f;
	m_WallKickRollImpact = 0.0f;
	m_ImpactShakePower = 0.0f;
	m_FallShakePower = 0.0f;

	m_CameraFOV = CameraFOV::Normal;
	m_IsGoalCameraFinished = false;
}

// ゴール演出の段階に合わせてカメラを動かす
void Camera::UpdateGoalCameraSequence()
{
	if (m_GoalCamera.mode == GoalCameraMode::Default)
	{
		UpdateGoalCameraDefault();
		return;
	}

	if (m_GoalCamera.mode == GoalCameraMode::Last)
	{
		UpdateGoalCameraLast();
		return;
	}
}

void Camera::UpdateGoalCameraDefault()
{
	Vector3 toGoal;

	toGoal.x = m_GoalCamera.endPosition.x - m_Position.x;
	toGoal.y = 0.0f;
	toGoal.z = m_GoalCamera.endPosition.z - m_Position.z;

	float remainDistance = sqrtf(
		toGoal.x * toGoal.x +
		toGoal.z * toGoal.z
	);

	// ゴール付近まで来たら、上昇してゴール床を見るLast演出へ切り替える。
	if (remainDistance <= m_GoalCamera.lastStartDistance)
	{
		m_GoalCamera.mode = GoalCameraMode::Last;

		m_GoalCamera.lastStartPosition = m_Position;

		m_GoalCamera.lastEndPosition.x = m_GoalCamera.goalBlockPosition.x;
		m_GoalCamera.lastEndPosition.y = m_GoalCamera.goalBlockPosition.y + m_GoalCamera.lastTopOffset;
		m_GoalCamera.lastEndPosition.z = m_GoalCamera.goalBlockPosition.z - m_GoalCamera.lastBackDistance;

		m_GoalCamera.lastRate = 0.0f;

		return;
	}

	m_Position.x += m_GoalCamera.moveDirection.x * m_GoalCamera.moveSpeed;
	m_Position.y = m_GoalCamera.startPosition.y;
	m_Position.z += m_GoalCamera.moveDirection.z * m_GoalCamera.moveSpeed;

	m_Target.x = m_Position.x + m_GoalCamera.moveDirection.x * m_GoalCamera.lookAhead;
	m_Target.y = m_Position.y - m_GoalCamera.targetDownOffset;
	m_Target.z = m_Position.z + m_GoalCamera.moveDirection.z * m_GoalCamera.lookAhead;

	m_Roll = 0.0f;
}

void Camera::UpdateGoalCameraLast()
{
	m_GoalCamera.lastRate += m_GoalCamera.lastRateSpeed;

	if (m_GoalCamera.lastRate > 1.0f)
	{
		m_GoalCamera.lastRate = 1.0f;
	}

	float t = m_GoalCamera.lastRate;

	float cameraT = t;
	float targetT = t + 0.12f;

	if (targetT > 1.0f)
	{
		targetT = 1.0f;
	}

	float cameraSmoothXZ = cameraT * cameraT * (3.0f - 2.0f * cameraT);
	float cameraRiseY = cameraT * cameraT * cameraT;

	m_Position.x =
		m_GoalCamera.lastStartPosition.x +
		(m_GoalCamera.lastEndPosition.x - m_GoalCamera.lastStartPosition.x) *
		cameraSmoothXZ;

	m_Position.y =
		m_GoalCamera.lastStartPosition.y +
		(m_GoalCamera.lastEndPosition.y - m_GoalCamera.lastStartPosition.y) *
		cameraRiseY;

	m_Position.z =
		m_GoalCamera.lastStartPosition.z +
		(m_GoalCamera.lastEndPosition.z - m_GoalCamera.lastStartPosition.z) *
		cameraSmoothXZ;

	float targetSmoothXZ = targetT * targetT * (3.0f - 2.0f * targetT);
	float targetRiseY = targetT * targetT * targetT;

	Vector3 pathTarget;

	pathTarget.x =
		m_GoalCamera.lastStartPosition.x +
		(m_GoalCamera.lastEndPosition.x - m_GoalCamera.lastStartPosition.x) *
		targetSmoothXZ;

	pathTarget.y =
		m_GoalCamera.lastStartPosition.y +
		(m_GoalCamera.lastEndPosition.y - m_GoalCamera.lastStartPosition.y) *
		targetRiseY;

	pathTarget.z =
		m_GoalCamera.lastStartPosition.z +
		(m_GoalCamera.lastEndPosition.z - m_GoalCamera.lastStartPosition.z) *
		targetSmoothXZ;

	if (t < 0.85f)
	{
		m_Target = pathTarget;
	}
	else
	{
		float goalLookT = (t - 0.85f) / 0.15f;

		if (goalLookT > 1.0f)
		{
			goalLookT = 1.0f;
		}

		float smoothGoalLookT = goalLookT * goalLookT * (3.0f - 2.0f * goalLookT);

		// 最後だけゴール床へ視線を寄せ、到達地点を見せる。
		m_Target.x =
			pathTarget.x +
			(m_GoalCamera.goalBlockPosition.x - pathTarget.x) *
			smoothGoalLookT;

		m_Target.y =
			pathTarget.y +
			(m_GoalCamera.goalBlockPosition.y - pathTarget.y) *
			smoothGoalLookT;

		m_Target.z =
			pathTarget.z +
			(m_GoalCamera.goalBlockPosition.z - pathTarget.z) *
			smoothGoalLookT;
	}

	m_Roll = 0.0f;

	if (m_GoalCamera.lastRate >= 1.0f)
	{
		m_IsGoalCameraFinished = true;
	}
}

// しゃがみ状態に合わせて視点の高さを変える
void Camera::UpdateCrouchEyeHeight(Player* player)
{
	float targetEyeHeight = m_Parameter.eyeHeight;
	float lerpRate = m_Parameter.crouchEyeReturnLerpRate;

	if (player != nullptr && player->IsCrouchRiding())
	{
		targetEyeHeight = m_Parameter.crouchEyeHeight;
		lerpRate = m_Parameter.crouchEyeLerpRate;
	}

	m_CurrentEyeHeight = LerpFloat(
		m_CurrentEyeHeight,
		targetEyeHeight,
		lerpRate
	);
}

// 二つの値の間を補間する
float Camera::LerpFloat(
	float current,
	float target,
	float rate
)
{
	return current + (target - current) * rate;
}

float Camera::ClampFloat(
	float value,
	float minValue,
	float maxValue
)
{
	if (value < minValue)
	{
		return minValue;
	}

	if (value > maxValue)
	{
		return maxValue;
	}

	return value;
}

// 値の正負を返す
float Camera::SignFloat(float value)
{
	if (value > 0.0f)
	{
		return 1.0f;
	}

	if (value < 0.0f)
	{
		return -1.0f;
	}

	return 0.0f;
}

void Camera::ClampPitch()
{
	m_Pitch = ClampFloat(
		m_Pitch,
		m_Parameter.pitchMin,
		m_Parameter.pitchMax
	);
}

// 落下演出中のカメラを動かす
void Camera::UpdateFallCamera(Player* player)
{
	if (player == nullptr)
	{
		return;
	}

	if (player->IsFallCameraMode())
	{
		m_Pitch = LerpFloat(
			m_Pitch,
			m_Parameter.fallTargetPitch,
			m_Parameter.fallPitchLerpRate
		);
	}
}

void Camera::UpdateWallKickPitchImpact()
{
	m_WallKickPitchImpact *= m_Parameter.wallKickPitchImpactDecay;

	if (fabsf(m_WallKickPitchImpact) < 0.001f)
	{
		m_WallKickPitchImpact = 0.0f;
	}
}

void Camera::UpdateFOV(Player* player)
{
	if (player == nullptr)
	{
		return;
	}

	float targetBaseFOV = m_Parameter.normalFOV;

	if (player->IsFallCameraMode())
	{
		targetBaseFOV = m_Parameter.fallFOV;
	}

	m_BaseFOV = LerpFloat(
		m_BaseFOV,
		targetBaseFOV,
		m_Parameter.fovLerpRate
	);

	m_FOVImpact *= m_Parameter.fovImpactDecay;

	if (m_FOVImpact < 0.001f)
	{
		m_FOVImpact = 0.0f;
		m_CameraFOV = CameraFOV::Normal;
	}

	m_FOV = m_BaseFOV + m_FOVImpact;
}

// 視線の前方向を求める
Vector3 Camera::BuildForward(float pitch)
{
	Vector3 forward;

	forward.x = sinf(m_Yaw) * cosf(pitch);
	forward.y = -sinf(pitch);
	forward.z = cosf(m_Yaw) * cosf(pitch);

	return forward;
}

// 視点の角度をカメラ本体の回転に反映する
void Camera::SyncGameObjectRotation(float pitch)
{
	m_Rotation.x = pitch;
	m_Rotation.y = m_Yaw;
	m_Rotation.z = 0.0f;
}

// 視線の右方向を求める
Vector3 Camera::BuildCameraRight(Vector3 forward)
{
	Vector3 cameraRight;

	cameraRight.x = forward.z;
	cameraRight.y = 0.0f;
	cameraRight.z = -forward.x;

	float rightLength = sqrtf(cameraRight.x * cameraRight.x + cameraRight.z * cameraRight.z);

	if (rightLength > 0.0f)
	{
		cameraRight.x /= rightLength;
		cameraRight.z /= rightLength;
	}

	return cameraRight;
}

// 壁滑りに合わせて視点をずらす
void Camera::UpdateWallSlidePositionOffset(
	Player* player,
	Vector3 forward,
	Vector3& basePosition
)
{
	float targetOffset = 0.0f;

	if (player != nullptr && player->IsWallSliding())
	{
		Vector3 wallNormal = player->GetWallNormal();
		Vector3 cameraRight = BuildCameraRight(forward);

		float side = wallNormal.x * cameraRight.x + wallNormal.z * cameraRight.z;

		targetOffset = -side * m_WallSlidePositionOffsetPower;
	}

	m_WallSlidePositionOffset = LerpFloat(
		m_WallSlidePositionOffset,
		targetOffset,
		m_WallSlidePositionOffsetLerpRate
	);

	Vector3 cameraRight = BuildCameraRight(forward);

	basePosition.x += cameraRight.x * m_WallSlidePositionOffset;
	basePosition.z += cameraRight.z * m_WallSlidePositionOffset;
}

// 壁滑りに合わせてカメラを傾ける
void Camera::UpdateWallSlideRoll(
	Player* player,
	Vector3 forward
)
{
	float targetRoll = 0.0f;

	if (player != nullptr && player->IsWallSliding())
	{
		float maxRoll = m_Parameter.maxRollDegree * 3.1415926535f / 180.0f;

		Vector3 wallNormal = player->GetWallNormal();
		Vector3 cameraRight = BuildCameraRight(forward);

		float side = wallNormal.x * cameraRight.x + wallNormal.z * cameraRight.z;

		targetRoll = -side * maxRoll;

		float rollSide = SignFloat(targetRoll);

		if (rollSide != 0.0f)
		{
			m_LastWallSlideRollSide = rollSide;
		}

		m_WallSlideRollKeepFrame = m_Parameter.wallSlideRollKeepFrame;
	}
	else
	{
		if (m_WallSlideRollKeepFrame > 0)
		{
			m_WallSlideRollKeepFrame--;

			float maxRoll = m_Parameter.maxRollDegree * 3.1415926535f / 180.0f;

			targetRoll = m_LastWallSlideRollSide * maxRoll * 0.35f;
		}
	}

	m_WallKickRollImpact *= m_Parameter.wallKickRollImpactDecay;

	if (fabsf(m_WallKickRollImpact) < 0.001f)
	{
		m_WallKickRollImpact = 0.0f;
	}

	float finalTargetRoll = targetRoll + m_WallKickRollImpact;
	float rollRate = m_Parameter.rollLerpRate;

	if (player == nullptr || !player->IsWallSliding())
	{
		if (m_WallSlideRollKeepFrame > 0)
		{
			rollRate = m_Parameter.wallSlideRollReturnRate;
		}
	}

	m_Roll = LerpFloat(
		m_Roll,
		finalTargetRoll,
		rollRate
	);
}

// カメラの揺れを減衰させる
void Camera::UpdateCameraShake(
	Player* player,
	Vector3 forward
)
{
	m_ShakeOffset = Vector3(0.0f, 0.0f, 0.0f);

	if (player == nullptr)
	{
		return;
	}

	Vector3 cameraRight = BuildCameraRight(forward);

	if (player->IsGround())
	{
		m_RunShakeTime += m_Parameter.runShakeSpeed * 0.016f;

		float verticalShake = sinf(m_RunShakeTime) * m_Parameter.runShakePower;
		float sideShake = sinf(m_RunShakeTime * 0.5f) * m_Parameter.runShakePower * 0.35f;

		m_ShakeOffset.x += cameraRight.x * sideShake;
		m_ShakeOffset.z += cameraRight.z * sideShake;
		m_ShakeOffset.y += verticalShake;
	}
	else
	{
		m_RunShakeTime += m_Parameter.runShakeSpeed * m_Parameter.airRunShakeSpeedRate;
	}

	if (player->IsWallSliding())
	{
		m_WallSlideShakeTime += m_Parameter.wallSlideShakeSpeed * 0.016f;

		float slideSideShake = sinf(m_WallSlideShakeTime) * m_Parameter.wallSlideShakePower;
		float slideVerticalShake = cosf(m_WallSlideShakeTime * 1.7f) * m_Parameter.wallSlideShakePower * 0.45f;

		m_ShakeOffset.x += cameraRight.x * slideSideShake;
		m_ShakeOffset.z += cameraRight.z * slideSideShake;
		m_ShakeOffset.y += slideVerticalShake;
	}
	else
	{
		m_WallSlideShakeTime += m_Parameter.wallSlideShakeSpeed * m_Parameter.wallSlideIdleSpeedRate;
	}

	if (!player->IsGround() && !player->IsWallSliding())
	{
		Vector3 velocity = player->GetVelocity();

		float fallSpeed = -velocity.y;

		if (fallSpeed > m_Parameter.fallShakeStartSpeed)
		{
			float speedRate = ClampFloat(
				(fallSpeed - m_Parameter.fallShakeStartSpeed) /
				(m_Parameter.fallShakeMaxSpeed - m_Parameter.fallShakeStartSpeed),
				0.0f,
				1.0f
			);

			m_FallShakePower = m_Parameter.fallShakeMaxPower * speedRate;
		}
		else
		{
			m_FallShakePower = 0.0f;
		}
	}
	else
	{
		m_FallShakePower = 0.0f;
	}

	if (m_FallShakePower > 0.0f)
	{
		m_FallShakeTimer += 0.016f;

		float fallShakeTime = m_FallShakeTimer * m_Parameter.fallShakeSpeed;

		float fallSideShake = sinf(fallShakeTime) * m_FallShakePower;
		float fallVerticalShake = cosf(fallShakeTime * 1.3f) * m_FallShakePower * 0.6f;

		m_ShakeOffset.x += cameraRight.x * fallSideShake;
		m_ShakeOffset.z += cameraRight.z * fallSideShake;
		m_ShakeOffset.y += fallVerticalShake;
	}
	else
	{
		m_FallShakeTimer = 0.0f;
	}

	if (m_ImpactShakePower > 0.001f)
	{
		float impactX = sinf(m_RunShakeTime * 31.0f) * m_ImpactShakePower;
		float impactY = cosf(m_RunShakeTime * 27.0f) * m_ImpactShakePower * 0.6f;

		m_ShakeOffset.x += cameraRight.x * impactX;
		m_ShakeOffset.z += cameraRight.z * impactX;
		m_ShakeOffset.y += impactY;

		m_ImpactShakePower *= m_Parameter.impactShakeDecay;
	}
	else
	{
		m_ImpactShakePower = 0.0f;
	}
}

// プレイヤー周囲の判定でカメラの衝突を調べる
bool Camera::IsCameraHitBlockByPlayerHit(
	Vector3 position,
	Vector3 playerHitSize
)
{
	Scene* scene = Manager::GetScene();

	if (scene == nullptr)
	{
		return false;
	}

	StageBlockManager* stageBlockManager = scene->GetGameObject<StageBlockManager>();

	if (stageBlockManager == nullptr)
	{
		return false;
	}

	Vector3 cameraHitSize = Vector3(
		playerHitSize.x + m_Parameter.cameraCollisionXZMargin,
		m_Parameter.cameraCollisionThinY,
		playerHitSize.z + m_Parameter.cameraCollisionXZMargin
	);

	StageBlock* hitBlock = stageBlockManager->GetHitStageBlock(
		position,
		cameraHitSize
	);

	return hitBlock != nullptr;
}

// 壁キック時に視野角を一時的に変える
void Camera::PlayWallKickFOVImpact()
{
	m_FOVImpact = m_Parameter.wallKickFOVImpact;
	m_FOV = m_BaseFOV + m_FOVImpact;
	m_ImpactShakePower = m_Parameter.wallKickShakePower;
	m_WallKickPitchImpact = m_Parameter.wallKickPitchImpact;

	float rollImpact = m_Parameter.wallKickRollImpactDegree * 3.1415926535f / 180.0f;
	float rollSide = m_LastWallSlideRollSide;

	if (rollSide == 0.0f)
	{
		rollSide = 1.0f;
	}

	m_WallKickRollImpact = rollImpact * rollSide * m_Parameter.wallKickUseSlideRollRate;
	m_WallSlideRollKeepFrame = m_Parameter.wallSlideRollKeepFrame;
	m_CameraFOV = CameraFOV::Impact;
}

void Camera::SetYawPitch(
	float yaw,
	float pitch
)
{
	m_Yaw = yaw;
	m_Pitch = pitch;

	ClampPitch();

	SyncGameObjectRotation(m_Pitch);
}

void Camera::SetYaw(float yaw)
{
	m_Yaw = yaw;

	SyncGameObjectRotation(m_Pitch);
}

void Camera::SetPitch(float pitch)
{
	m_Pitch = pitch;

	ClampPitch();

	SyncGameObjectRotation(m_Pitch);
}
