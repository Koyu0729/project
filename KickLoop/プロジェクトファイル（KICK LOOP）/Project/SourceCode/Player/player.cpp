#include "main.h"
#include "renderer.h"

#include "player.h"
#include "animationModel.h"

#include <Windows.h>

#include "manager.h"
#include "scene.h"

#include "stageBlockManager.h"

#include "gameTime.h"
#include "input.h"

#include <cmath>

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

	m_State = PlayerRuntimeState();
	m_StateController.Init(m_State);
	m_ActionResolver.Init(m_State);
	m_WallKickController.Reset(m_State);
	m_OldLeftClick = false;
	m_GoalLaunchFrame = 0;

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

// 状態固有の更新後、アニメーションとリスナー位置を一度だけ更新する。
void Player::Update()
{
	UpdateMainState(true);

	switch (m_State.mainState)
	{
	case PlayerMainState::Wait:
	case PlayerMainState::Dead:
		m_EffectController.Disable(true);
		break;

	case PlayerMainState::Freeze:
		m_EffectController.Disable(false);
		break;

	case PlayerMainState::Playing:
	case PlayerMainState::Falling:
		UpdateGameplay();
		break;

	case PlayerMainState::GoalLaunch:
		UpdateGoalLaunch();
		CheckGoalTopBlock();
		break;

	case PlayerMainState::Goal:
		m_EffectController.UpdateGoalEffect(m_Position);
		break;
	}

	UpdateAnimation();
	m_SoundController.UpdateListenerPosition(m_Position);
}

void Player::SetSceneReferences(Camera* camera, StageBlockManager* stageBlockManager, Time* time)
{
	m_StageBlockManager = stageBlockManager;
	m_GameTime = time;
	m_MoveController.SetCamera(camera);
	m_WallKickController.SetStageBlockManager(stageBlockManager);
	m_ActionResolver.SetSceneReferences(camera, stageBlockManager);
}

void Player::UpdateMainState(bool advanceFrame)
{
	bool isInputBlocked = m_GameTime == nullptr || !m_GameTime->IsPlaying();
	PlayerMainState previousState = m_State.mainState;

	if (advanceFrame)
	{
		m_StateController.UpdateState(
			m_State, m_Position, m_FallCameraStartY, m_FallSceneChangeY, isInputBlocked
		);
	}
	else
	{
		m_StateController.CheckGameplayState(
			m_State, m_Position, m_FallCameraStartY, m_FallSceneChangeY, isInputBlocked
		);
	}

	OnMainStateChanged(previousState);
}

// 状態の切り替えに伴う、アクションと演出の後始末。
void Player::OnMainStateChanged(PlayerMainState previousState)
{
	if (previousState == m_State.mainState)
	{
		return;
	}

	if (previousState == PlayerMainState::GoalLaunch)
	{
		m_GoalLaunchFrame = 0;
	}

	switch (m_State.mainState)
	{
	case PlayerMainState::Dead:
	case PlayerMainState::GoalLaunch:
	case PlayerMainState::Goal:
		m_ActionEffects.clear();
		m_ActionResolver.ResetClimbAndSlide(m_State);
		m_WallKickController.Reset(m_State);
		m_AirDiveController.Init();
		m_EffectController.Disable(true);
		m_EffectController.SetWallSlideEffect(false, 0.0f);
		break;

	default:
		break;
	}

	if (m_State.mainState == PlayerMainState::Goal)
	{
		m_EffectController.StartGoalEffect(m_Position);
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
	m_WallKickController.UpdateCoyote(m_State, m_WallKickParameter);

	m_BonusActionController.BuildAction(
		m_State,
		m_BonusActionParameter,
		m_RunController,
		m_SoundController,
		action
	);

	m_JumpController.BuildAction(
		m_State,
		m_MovementParameters.jump,
		m_SoundController,
		action
	);

	PlayerWallKickResult wallKickResult = m_WallKickController.BuildAction(m_State, m_WallKickParameter, action);
	if (wallKickResult.isSuccess)
	{
		m_SoundController.PlayWallKick();
		m_EffectController.EmitWallKickSpark(m_Position);
	}

	m_RunController.BuildAction(
		m_State,
		m_MovementParameters.move,
		m_RunParameter,
		wallKickResult,
		&m_SoundController,
		action
	);

	m_MoveController.BuildAction(
		m_Position,
		m_Rotation,
		m_State,
		m_MovementParameters.move,
		m_EffectController,
		m_SoundController,
		action
	);

	m_AirDiveController.BuildAction(
		m_State,
		m_DiveParameter,
		action
	);

	PlayerMainState previousState = m_State.mainState;

	PlayerMovementResult movementResult = ResolveMovement(action);

	ApplyMovementEffects(movementResult);

#ifdef DEBUG
	if (movementResult.updateEffects && Input::GetKeyTrigger('P'))
	{
		m_ActionResolver.DebugPlayerStatus(
			m_Position,
			m_State,
			m_MovementParameters.move,
			action
		);
	}
#endif // DEBUG

	OnMainStateChanged(previousState);
	if (IsDead())
	{
		return;
	}

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
		Scene* scene = Manager::GetScene();

		if (scene != nullptr)
		{
			StageBlockManager* stageBlockManager = scene->GetGameObject<StageBlockManager>();

			if (stageBlockManager != nullptr)
			{
				m_Position.x = 25.0f;
				m_Position.y = 5.0f;
				m_Position.z = stageBlockManager->GetStageGenerateEndDistance() + 13.0f;

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

	UpdateMainState(false);
	CheckGoalTopBlock();
}

PlayerMovementResult Player::ResolveMovement(PlayerActionRequest& action)
{
	PlayerMovementResult result;

	// 死亡要求は、よじ登り中の補間より先に反映する。
	if (action.requestGameOver)
	{
		m_StateController.ChangeState(m_State, PlayerMainState::Dead);
		return result;
	}

	if (m_ActionResolver.UpdateClimb(
		m_Position,
		m_State
	))
	{
		result.updateEffects = true;
		return result;
	}

	bool playBoost =
		m_ActionResolver.PrepareMovement(
			m_Position,
			m_State,
			m_MovementParameters.move,
			action,
			m_StateController
		);

	if (playBoost)
	{
		m_EffectController.PlayBoost();
	}

	if (m_StateController.IsDead(m_State))
	{
		return result;
	}

	return m_ActionResolver.Resolve(
		m_Position,
		m_State,
		m_MovementParameters,
		action,
		&m_SoundController
	);
}

void Player::ApplyMovementEffects(const PlayerMovementResult& result)
{
	if (!result.updateEffects)
	{
		return;
	}

	if (result.landed)
	{
		m_SoundController.PlayLanding();
		m_EffectController.EmitLandingSpark(m_Position);
	}

	float moveAmount = fabsf(result.forwardMove);
	float maxSpeed = m_MovementParameters.move.maxMoveSpeed;
	float speedRate = maxSpeed <= 0.001f ? 0.0f : moveAmount / maxSpeed;

	m_EffectController.SetSpeedRate(speedRate);

	bool isWallSlideEffect =
		!m_State.isGround &&
		m_State.isTouchWall &&
		m_State.velocity.y <= 0.0f &&
		fabsf(m_State.wallNormal.x) > 0.1f;

	m_EffectController.SetWallSlideEffect(
		isWallSlideEffect,
		m_State.wallNormal.x
	);
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

// ジャンプ台から受けた効果を追加する
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

	XMMATRIX scale = XMMatrixScaling(m_Scale.x, m_Scale.y, m_Scale.z);

	XMMATRIX playerRot = XMMatrixRotationRollPitchYaw(m_Rotation.x, m_Rotation.y, m_Rotation.z);

	XMMATRIX trans = XMMatrixTranslation(m_Position.x + 0.30f, m_Position.y + 1.0f, m_Position.z);

	XMMATRIX world = modelRotationOffset * scale * playerRot * trans;

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
	PlayerMainState previousState = m_State.mainState;
	m_StateController.ChangeState(m_State, state);
	OnMainStateChanged(previousState);
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
