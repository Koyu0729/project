#include "main.h"
#include "renderer.h"

#include "player.h"
#include "animationModel.h"

#include <Windows.h>

#include "manager.h"
#include "scene.h"

#include "goalBuilding.h"
#include "stageBlockManager.h"
#include "stageBlock.h"

#include "gameTime.h"

// モデルと各アクションの制御を準備する
void Player::Init()
{
	m_AnimationModel = new AnimationModel();

	// リグ入りのプレイヤーモデル。
	m_AnimationModel->Load("asset\\model\\AnimePlayer.fbx");

	// 現在使用している走りアニメーション。
	m_AnimationModel->LoadAnimation(
		"asset\\model\\WallRun.fbx",
		"Run"
	);

	m_AnimationFrame = 0;

	Renderer::CreateVertexShader(
		&m_VertexShader,
		&m_VertexLayout,
		"shader\\unlitTextureVS.cso"
	);

	Renderer::CreatePixelShader(
		&m_PixelShader,
		"shader\\unlitTexturePS.cso"
	);

	SetScale(Vector3(0.25f, 0.25f, 0.25f));

	m_Position = Vector3(25.0f, 3.0f, 13.0f);

	// 進行距離計算用に、開始位置を保持する。
	m_StartPosition = m_Position;

	m_State.velocity = Vector3(0.0f, 0.0f, 0.0f);

	m_State.wallKickVelocity = Vector3(0.0f, 0.0f, 0.0f);

	m_State.isGround = false;
	m_State.isTouchWall = false;
	m_State.isWallKick = false;

	m_State.wallNormal = Vector3(0.0f, 0.0f, 0.0f);

	m_State.freezeFrame = 0;
	m_State.isFallCameraMode = false;

	m_State.wallKickCoyoteFrame = 0;

	m_State.wallKickCoyoteNormal = Vector3(0.0f, 0.0f, 0.0f);

	m_State.wallKickCoyoteBlock = nullptr;

	m_State.isWallSlideLocked = false;

	m_State.wallSlideLockNormal = Vector3(0.0f, 0.0f, 0.0f);

	m_State.wallSlideLockPositionX = 0.0f;
	m_State.wallSlideLockPositionZ = 0.0f;
	m_State.wallSlideLockBlock = nullptr;

	m_State.isLedgeClimb = false;
	m_State.ledgeClimbFrame = 0;

	m_State.ledgeClimbStartPosition = Vector3(0.0f, 0.0f, 0.0f);

	m_State.ledgeClimbEndPosition = Vector3(0.0f, 0.0f, 0.0f);

	m_State.ledgeClimbForward = Vector3(0.0f, 0.0f, 0.0f);

	m_State.currentWallBlock = nullptr;
	m_State.oldBonusWallBlock = nullptr;

	m_OldLeftClick = false;

	m_StateController.Init(m_State);

	m_RunController.Init();

	m_BonusActionController.Init();

	m_AirDiveController.Init();

	m_ActionEffects.clear();

	m_SoundController.Init();

	// 初期フレームを反映しておく。
	UpdateAnimation();
}

void Player::Uninit()
{
	m_ActionEffects.clear();

	m_SoundController.Uninit();

	if (m_AnimationModel)
	{
		m_AnimationModel->Uninit();

		delete m_AnimationModel;
		m_AnimationModel = nullptr;
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

// 現在の状態に応じた処理を呼ぶ
void Player::Update()
{
	// ゴール後はプレイヤーの通常移動を止める。
	// ただし、花火やカメラ開始タイマーなどのゴール演出だけは更新する。
	if (m_IsGoal)
	{
		m_EffectController.UpdateGoalEffect(m_Position);

		UpdateAnimation();

		m_SoundController.UpdateListenerPosition(m_Position);

		return;
	}

	if (m_IsGoalLaunch)
	{
		UpdateGoalLaunch();

		CheckGoalTopBlock();

		UpdateAnimation();

		m_SoundController.UpdateListenerPosition(m_Position);

		return;
	}

	m_StateController.UpdateState(
		m_State,
		m_Position,
		m_FallCameraStartY,
		m_FallSceneChangeY
	);

	switch (m_State.mainState)
	{
	case PlayerMainState::Wait:
		m_EffectController.Disable(true);

		m_SoundController.UpdateListenerPosition(m_Position);

		UpdateAnimation();

		break;

	case PlayerMainState::Freeze:
		m_EffectController.Disable(false);

		m_SoundController.UpdateListenerPosition(m_Position);

		UpdateAnimation();

		break;

	case PlayerMainState::Playing:
		UpdateGameplay();

		break;
	case PlayerMainState::Falling:
		UpdateGameplay();

		break;

	case PlayerMainState::Dead:
		m_EffectController.Disable(true);

		m_SoundController.UpdateListenerPosition(m_Position);

		UpdateAnimation();

		break;

	default:
		m_EffectController.Disable(true);

		m_SoundController.UpdateListenerPosition(m_Position);

		UpdateAnimation();

		break;
	}
}

// 操作の要求を集めて移動に反映する
void Player::UpdateGameplay()
{
	PlayerActionRequest action;

	action.Clear();


	bool nowLeftClick = (GetAsyncKeyState(VK_LBUTTON) & 0x8000) != 0;

	if (nowLeftClick && !m_OldLeftClick)
	{
		action.RequestLedgeClimb();
	}

	m_OldLeftClick = nowLeftClick;

	BuildActionEffects(action);

	m_BonusActionController.BuildAction(
		m_State,
		m_BonusActionParameter,
		m_RunController,
		m_SoundController,
		action
	);

	m_JumpController.BuildAction(
		m_State,
		m_JumpParameter,
		m_SoundController,
		action
	);

	PlayerWallKickResult wallKickResult =
		m_WallKickController.BuildAction(
			m_Position,
			m_State,
			m_WallKickParameter,
			&m_SoundController,
			action
		);

	m_RunController.BuildAction(
		m_State,
		m_MoveParameter,
		m_RunParameter,
		wallKickResult.isSuccess,
		wallKickResult.wallBlock,
		&m_SoundController,
		action
	);

	m_MoveController.BuildAction(
		m_Position,
		m_Rotation,
		m_State,
		m_MoveParameter,
		m_EffectController,
		m_SoundController,
		action
	);

	m_AirDiveController.BuildAction(
		m_State,
		m_DiveParameter,
		action
	);

	m_ActionResolver.Resolve(
		m_Position,
		m_State,
		m_MoveParameter,
		m_JumpParameter,
		m_ClimbParameter,
		m_SlopeRideController,
		m_SlopeRideParameter,
		action,
		m_EffectController,
		m_StateController,
		&m_SoundController
	);

#ifdef _DEBUG
	// DEBUG ONLY: ステージ確認用の強制加速・前方移動。
	if (GetAsyncKeyState('T') & 0x8000)
	{
		m_State.velocity.z = 8.0f;
		m_Position.z += 8.0f;
	}

	// DEBUG ONLY: 少しだけ前方へ移動。
	if (GetAsyncKeyState('G') & 0x0001)
	{
		m_Position.z += 20.0f;
		m_State.velocity = Vector3(0.0f, 0.0f, 8.0f);
		m_State.wallKickVelocity = Vector3(0.0f, 0.0f, 0.0f);
	}

	// DEBUG ONLY: ゴール生成距離へ転移。
	if (GetAsyncKeyState('H') & 0x0001)
	{
		Scene* scene =
			Manager::GetScene();

		if (scene != nullptr)
		{
			StageBlockManager* stageBlockManager =
				scene->GetGameObject<StageBlockManager>();

			if (stageBlockManager != nullptr)
			{
				m_Position.x = 25.0f;
				m_Position.y = 5.0f;
				m_Position.z =
					stageBlockManager->GetStageGenerateEndDistance() +
					13.0f;

				m_State.velocity = Vector3(0.0f, 0.0f, 8.0f);
				m_State.wallKickVelocity = Vector3(0.0f, 0.0f, 0.0f);

				printf(
					"[DEBUG] Teleport to StageGenerateEndDistance. z=%.2f\n",
					m_Position.z
				);
			}
		}
	}
#endif

	m_RunController.UpdateWallSlideBonusGauge(
		m_State,
		m_RunParameter,
		IsWallSliding(),
		&m_SoundController
	);

	m_StateController.UpdateState(
		m_State,
		m_Position,
		m_FallCameraStartY,
		m_FallSceneChangeY
	);

	CheckGoalTopBlock();

	UpdateAnimation();

	m_SoundController.UpdateListenerPosition(m_Position);
}

// 走行アニメーションを進める
void Player::UpdateAnimation()
{
	if (m_AnimationModel == nullptr)
	{
		return;
	}

	m_AnimationFrame++;

	// 現在は走りアニメーションを固定で再生する。
	m_AnimationModel->Update(
		"Run",
		m_AnimationFrame,
		"Run",
		m_AnimationFrame,
		0.0f
	);
}

// 継続中の外部効果を移動の要求に変換する
void Player::BuildActionEffects(
	PlayerActionRequest& action
)
{
	for (int i = (int)m_ActionEffects.size() - 1; i >= 0; i--)
	{
		m_ActionEffects[i].UpdateGroundEnd(
			m_State.isGround
		);

		if (!m_ActionEffects[i].isActive)
		{
			m_ActionEffects.erase(m_ActionEffects.begin() + i);

			continue;
		}

		m_ActionEffects[i].Apply(
			action
		);

		m_ActionEffects[i].Update();

		if (m_ActionEffects[i].IsFinished())
		{
			m_ActionEffects.erase(m_ActionEffects.begin() + i);
		}
	}
}

// ジャンプ台やレーザーから受けた効果を記録する
void Player::AddActionEffect(
	PlayerActionEffect effect
)
{
	m_ActionEffects.push_back(effect);
}

// プレイヤーモデルを描画する
void Player::Draw()
{
	if (!m_Visible)
	{
		return;
	}

	if (m_AnimationModel == nullptr)
	{
		return;
	}

	Renderer::GetDeviceContext()->IASetInputLayout(
		m_VertexLayout
	);

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

	XMMATRIX modelRotationOffset =
		XMMatrixRotationRollPitchYaw(
			XM_PIDIV2,
			XM_PI + XM_PIDIV2,
			XM_PIDIV2
		);

	XMMATRIX scale =
		XMMatrixScaling(
			m_Scale.x,
			m_Scale.y,
			m_Scale.z
		);

	XMMATRIX playerRot =
		XMMatrixRotationRollPitchYaw(
			m_Rotation.x,
			m_Rotation.y,
			m_Rotation.z
		);

	XMMATRIX trans =
		XMMatrixTranslation(
			m_Position.x + 0.30f,
			m_Position.y + 1.0f,
			m_Position.z
		);

	XMMATRIX world =
		modelRotationOffset *
		scale *
		playerRot *
		trans;

	Renderer::SetWorldMatrix(world);

	m_AnimationModel->Draw();
}

// 落下用カメラを使う状態か調べる
bool Player::IsFallCameraMode()
{
	return m_State.isFallCameraMode;
}

void Player::SetMainState(
	PlayerMainState state
)
{
	m_State.mainState = state;
}

PlayerMainState Player::GetMainState()
{
	return m_State.mainState;
}

float Player::GetProgressDistance() const
{
	float distance = m_Position.z - m_StartPosition.z;

	if (distance < 0.0f)
	{
		distance = 0.0f;
	}

	return distance;
}

// 補間値を滑らかにする
float Player::SmoothStep01(float value)
{
	if (value < 0.0f)
	{
		value = 0.0f;
	}

	if (value > 1.0f)
	{
		value = 1.0f;
	}

	return
		value *
		value *
		(3.0f - 2.0f * value);
}

void Player::StartGoalLaunch(
	Vector3 targetPosition,
	int frameMax,
	float height
)
{
	m_IsGoalLaunch = true;

	m_GoalLaunchStartPosition = m_Position;

	m_GoalLaunchTargetPosition = targetPosition;

	m_GoalLaunchFrame = 0;

	m_GoalLaunchFrameMax = frameMax;

	if (m_GoalLaunchFrameMax <= 1)
	{
		m_GoalLaunchFrameMax = 1;
	}

	m_GoalLaunchHeight = height;

	if (m_GoalLaunchHeight < 0.0f)
	{
		m_GoalLaunchHeight = 0.0f;
	}

	m_State.velocity = Vector3(0.0f, 0.0f, 0.0f);

	m_State.wallKickVelocity = Vector3(0.0f, 0.0f, 0.0f);

	m_State.isGround = false;
	m_State.isTouchWall = false;
	m_State.isWallKick = false;
	m_State.isLedgeClimb = false;

	m_State.wallNormal = Vector3(0.0f, 0.0f, 0.0f);

	m_State.currentWallBlock = nullptr;
}

// ゴールへ向かう放物線上に移動する
void Player::UpdateGoalLaunch()
{
	m_GoalLaunchFrame++;

	float t = (float)m_GoalLaunchFrame / (float)m_GoalLaunchFrameMax;

	if (t > 1.0f)
	{
		t = 1.0f;
	}

	float smoothT = SmoothStep01(t);

	Vector3 basePosition;

	basePosition.x =
		m_GoalLaunchStartPosition.x +
		(m_GoalLaunchTargetPosition.x - m_GoalLaunchStartPosition.x) *
		smoothT;

	basePosition.y =
		m_GoalLaunchStartPosition.y +
		(m_GoalLaunchTargetPosition.y - m_GoalLaunchStartPosition.y) *
		smoothT;

	basePosition.z =
		m_GoalLaunchStartPosition.z +
		(m_GoalLaunchTargetPosition.z - m_GoalLaunchStartPosition.z) *
		smoothT;

	// 放物線の高さ。
	// t=0 と t=1 では0、t=0.5で最大。
	float arc =
		4.0f *
		t *
		(1.0f - t);

	basePosition.y += arc * m_GoalLaunchHeight;

	m_Position = basePosition;

	if (m_GoalLaunchFrame >= m_GoalLaunchFrameMax)
	{
		m_Position = m_GoalLaunchTargetPosition;

		m_IsGoalLaunch = false;

		m_GoalLaunchFrame = 0;

		m_State.velocity = Vector3(0.0f, 0.0f, 0.0f);

		m_State.wallKickVelocity = Vector3(0.0f, 0.0f, 0.0f);

		m_State.isGround = true;
		m_State.isTouchWall = false;
		m_State.isWallKick = false;

		m_State.wallNormal = Vector3(0.0f, 0.0f, 0.0f);

		m_State.currentWallBlock = nullptr;
	}
}

bool Player::IsGoalLaunching() const
{
	return m_IsGoalLaunch;
}

// ゴール建物の上面への到達を調べる
void Player::CheckGoalTopBlock()
{
	if (m_IsGoal)
	{
		return;
	}

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

	StageBlock* hitBlock =
		stageBlockManager->GetHitStageBlock(
			m_Position,
			m_MoveParameter.hitSize
		);

	if (hitBlock == nullptr)
	{
		return;
	}

	if (!hitBlock->IsGoalTopBlock())
	{
		return;
	}

	Time* time = scene->GetGameObject<Time>();

	if (time != nullptr)
	{
		time->StopTime();
	}

	StartGoalSequence();
}

// ゴール演出を開始する
void Player::StartGoalSequence()
{
	if (m_IsGoal)
	{
		return;
	}

	m_IsGoal = true;

	m_State.velocity = Vector3(0.0f, 0.0f, 0.0f);

	m_State.wallKickVelocity = Vector3(0.0f, 0.0f, 0.0f);

	m_State.isGround = true;
	m_State.isTouchWall = false;
	m_State.isWallKick = false;
	m_State.isLedgeClimb = false;

	m_State.wallNormal = Vector3(0.0f, 0.0f, 0.0f);

	m_State.currentWallBlock = nullptr;

	m_EffectController.StartGoalEffect(m_Position);
}
