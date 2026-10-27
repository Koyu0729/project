#include "playerMoveController.h"

#include "input.h"
#include "camera.h"
#include "manager.h"
#include "scene.h"

#include "playerRuntimeState.h"
#include "playerMoveParameter.h"
#include "playerActionRequest.h"
#include "playerEffectController.h"
#include "playerSoundController.h"

#include <cmath>

// 入力とカメラの向きから移動を要求する
void PlayerMoveController::BuildAction(
	Vector3 playerPosition,
	Vector3& rotation,
	PlayerRuntimeState& state,
	PlayerMoveParameter& moveParameter,
	PlayerEffectController& effectController,
	PlayerSoundController& soundController,
	PlayerActionRequest& action
)
{
	float inputX = 0.0f;
	float inputZ = 0.0f;
	float inputLength = 0.0f;

	GetMoveInput(inputX, inputZ, inputLength);

	bool isWallSliding = !state.isGround && state.isTouchWall && state.velocity.y <= 0.0f;

	// 壁スライドに入った瞬間だけ音を鳴らす。
	if (isWallSliding && !m_PrevWallSliding)
	{
		soundController.PlayWallSlide();
	}

	if (isWallSliding)
	{
		m_WallSlideSparkTimer--;

		if (m_WallSlideSparkTimer <= 0)
		{
			m_WallSlideSparkTimer = 4;

			Vector3 sparkPosition = playerPosition;
			sparkPosition.x -= state.wallNormal.x * 0.45f;

			if (state.velocity.z > 0.5f)
			{
				sparkPosition.z += 2.2f;
				sparkPosition.y -= 0.75f;
			}
			else if (state.velocity.z < -0.5f)
			{
				sparkPosition.z -= 2.2f;
				sparkPosition.y -= 0.75f;
			}
			else
			{
				sparkPosition.y -= 0.6f;
			}

			effectController.EmitWallSlidSpark(sparkPosition);
		}
	}
	else
	{
		m_WallSlideSparkTimer = 0;
	}

	m_PrevWallSliding = isWallSliding;

	if (isWallSliding && moveParameter.wallSlideAutoForward)
	{
		inputZ = moveParameter.wallSlideAutoForwardInput;

		// 壁スライド中はA/D入力を入れても、自動前進量が弱まらないようにする。
		inputLength = 1.0f;

		if (inputX < -1.0f)
		{
			inputX = -1.0f;
		}

		if (inputX > 1.0f)
		{
			inputX = 1.0f;
		}
	}

	if (Input::GetKeyPress(VK_CONTROL))
	{
		action.SetCrouchHold(true);
	}

	action.SetInputLength(inputLength);

	Vector3 targetMove = GetCameraMoveDirection(inputX, inputZ, rotation, effectController);
	float moveSpeed = action.GetMoveSpeed(moveParameter);

	if (isWallSliding)
	{
		moveSpeed *= moveParameter.wallSlideMoveSpeedRate;
	}

	targetMove.x *= moveSpeed;
	targetMove.z *= moveSpeed;

	if (inputLength > 0.0f)
	{
		action.SetHorizontalTargetVelocity(targetMove);
	}
}

// 斜め入力でも速くならないように方向を正規化する
void PlayerMoveController::GetMoveInput(
	float& inputX,
	float& inputZ,
	float& inputLength
)
{
	inputX = 0.0f;
	inputZ = 0.0f;
	inputLength = 0.0f;

	if (Input::GetKeyPress('A'))
	{
		inputX -= 1.0f;
	}

	if (Input::GetKeyPress('D'))
	{
		inputX += 1.0f;
	}

	if (Input::GetKeyPress('W'))
	{
		inputZ += 1.0f;
	}

	if (Input::GetKeyPress('S'))
	{
		inputZ -= 1.0f;
	}

	inputLength = sqrtf(inputX * inputX + inputZ * inputZ);

	if (inputLength > 0.0f)
	{
		inputX /= inputLength;
		inputZ /= inputLength;
	}
}

// カメラ基準の入力をワールド方向に変換する
Vector3 PlayerMoveController::GetCameraMoveDirection(
	float inputX,
	float inputZ,
	Vector3& rotation,
	PlayerEffectController& effectController
)
{
	(void)effectController;

	Vector3 move = Vector3(0.0f, 0.0f, 0.0f);
	Scene* scene = Manager::GetScene();

	if (scene == nullptr)
	{
		return move;
	}

	Camera* camera = scene->GetGameObject<Camera>();

	if (camera == nullptr)
	{
		return move;
	}

	Vector3 cameraForward = camera->GetTarget() - camera->GetPosition();
	cameraForward.y = 0.0f;

	float cameraForwardLength = sqrtf(cameraForward.x * cameraForward.x + cameraForward.z * cameraForward.z);

	if (cameraForwardLength <= 0.001f)
	{
		return move;
	}

	cameraForward.x /= cameraForwardLength;
	cameraForward.z /= cameraForwardLength;

	float cameraYaw = atan2f(cameraForward.x, cameraForward.z);
	rotation.y = cameraYaw;

	Vector3 cameraRight;
	cameraRight.x = cameraForward.z;
	cameraRight.y = 0.0f;
	cameraRight.z = -cameraForward.x;

	move.x = cameraRight.x * inputX + cameraForward.x * inputZ;
	move.y = 0.0f;
	move.z = cameraRight.z * inputX + cameraForward.z * inputZ;

	return move;
}
