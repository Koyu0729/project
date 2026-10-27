#include "playerStateController.h"

#include "manager.h"
#include "scene.h"

#include "gameTime.h"

// 開始待ちの状態にする
void PlayerStateController::Init(PlayerRuntimeState& state)
{
	state.mainState = PlayerMainState::Wait;

	state.freezeFrame = 0;

	state.isFallCameraMode = false;
}

// 待機・プレイ・停止・落下の状態を切り替える
void PlayerStateController::UpdateState(
	PlayerRuntimeState& state,
	const Vector3& position,
	float fallCameraStartY,
	float fallSceneChangeY
)
{
	switch (state.mainState)
	{
	case PlayerMainState::Wait:
		UpdateWaitState(
			state
		);
		break;

	case PlayerMainState::Playing:
		UpdatePlayingState(
			state,
			position,
			fallCameraStartY,
			fallSceneChangeY
		);
		break;

	case PlayerMainState::Freeze:
		UpdateFreezeState(
			state
		);
		break;

	case PlayerMainState::Falling:
		UpdateFallingState(
			state,
			position,
			fallSceneChangeY
		);
		break;

	case PlayerMainState::Dead:
		break;

	default:
		ChangeState(
			state,
			PlayerMainState::Wait
		);
		break;
	}
}

// タイマーの開始に合わせて操作を有効にする
void PlayerStateController::UpdateWaitState(PlayerRuntimeState& state)
{
	if (!IsInputBlocked())
	{
		ChangeState(
			state,
			PlayerMainState::Playing
		);
	}
}

// プレイ中の落下と死亡条件を調べる
void PlayerStateController::UpdatePlayingState(
	PlayerRuntimeState& state,
	const Vector3& position,
	float fallCameraStartY,
	float fallSceneChangeY
)
{
	if (IsInputBlocked())
	{
		ChangeState(
			state,
			PlayerMainState::Wait
		);

		return;
	}

	if (position.y <= fallSceneChangeY)
	{
		ChangeToDead(
			state
		);

		return;
	}

	if (position.y <= fallCameraStartY)
	{
		state.isFallCameraMode = true;

		ChangeState(
			state,
			PlayerMainState::Falling
		);
	}
}

// 停止時間が切れたら操作を再開する
void PlayerStateController::UpdateFreezeState(PlayerRuntimeState& state)
{
	state.freezeFrame--;

	if (state.freezeFrame <= 0)
	{
		state.freezeFrame = 0;

		ChangeState(
			state,
			PlayerMainState::Playing
		);
	}
}

// 落下後のシーン切り替えを判定する
void PlayerStateController::UpdateFallingState(
	PlayerRuntimeState& state,
	const Vector3& position,
	float fallSceneChangeY
)
{
	if (position.y <= fallSceneChangeY)
	{
		ChangeToDead(state);
	}
}

// 指定フレームだけ動きを止める
void PlayerStateController::StartFreeze(
	PlayerRuntimeState& state,
	int frame
)
{
	state.freezeFrame = frame;

	ChangeState(
		state,
		PlayerMainState::Freeze
	);
}

bool PlayerStateController::IsPlaying(const PlayerRuntimeState& state)
{
	return state.mainState == PlayerMainState::Playing || state.mainState == PlayerMainState::Falling;
}


bool PlayerStateController::IsDead(const PlayerRuntimeState& state)
{
	return state.mainState == PlayerMainState::Dead;
}

bool PlayerStateController::IsInputBlocked()
{
	Time* time = Manager::GetScene()->GetGameObject<Time>();

	if (time == nullptr)
	{
		return true;
	}

	if (!time->IsPlaying())
	{
		return true;
	}

	return false;
}

void PlayerStateController::ChangeToDead(PlayerRuntimeState& state)
{
	ChangeState(
		state,
		PlayerMainState::Dead
	);
}

void PlayerStateController::ChangeState(
	PlayerRuntimeState& state,
	PlayerMainState nextState
)
{
	state.mainState = nextState;
}