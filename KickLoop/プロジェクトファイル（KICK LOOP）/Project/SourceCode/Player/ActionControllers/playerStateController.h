#pragma once

#include "main.h"
#include "playerRuntimeState.h"

class PlayerEffectController;
class PlayerSoundController;

// 待機・操作中・停止・落下・死亡の状態を管理する。
// Wait、Playing、Freeze、Falling、Deadの切り替えを担当し、
// 壁キックやジャンプなどの細かいアクション処理は各ControllerとResolverに任せる。
class PlayerStateController
{
public:
	void Init(PlayerRuntimeState& state);

	void UpdateState(
		PlayerRuntimeState& state,
		const Vector3& position,
		float fallCameraStartY,
		float fallSceneChangeY
	);

	void StartFreeze(
		PlayerRuntimeState& state,
		int frame
	);

	bool IsPlaying(const PlayerRuntimeState& state);


	bool IsDead(const PlayerRuntimeState& state);

private:
	// タイマー開始前や停止中など、プレイヤー入力を受け付けない状態。
	bool IsInputBlocked();

	void UpdateWaitState(PlayerRuntimeState& state);

	void UpdatePlayingState(
		PlayerRuntimeState& state,
		const Vector3& position,
		float fallCameraStartY,
		float fallSceneChangeY
	);

	void UpdateFreezeState(PlayerRuntimeState& state);

	void UpdateFallingState(
		PlayerRuntimeState& state,
		const Vector3& position,
		float fallSceneChangeY
	);

	void ChangeToDead(PlayerRuntimeState& state);

	void ChangeState(
		PlayerRuntimeState& state,
		PlayerMainState nextState
	);
};