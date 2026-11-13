#pragma once

#include "playerRuntimeState.h"

// 待機・操作・停止・落下・死亡・ゴールの遷移と開始/終了処理を管理する。
// 接地や壁接触は、各アクションと当たり判定で更新する。
class PlayerStateController
{
private:
	PlayerMainState m_FreezeReturnState = PlayerMainState::Playing;

public:
	void Init(PlayerRuntimeState& state);

	// フレーム開始時に一度だけ呼ぶ。停止時間もここで進める。
	void UpdateState(
		PlayerRuntimeState& state,
		const Vector3& position,
		float fallCameraStartY,
		float fallSceneChangeY,
		bool isInputBlocked
	);

	// 移動後の判定。停止時間は進めない。
	void CheckGameplayState(
		PlayerRuntimeState& state,
		const Vector3& position,
		float fallCameraStartY,
		float fallSceneChangeY,
		bool isInputBlocked
	);

	void ChangeState(PlayerRuntimeState& state, PlayerMainState nextState);
	void StartFreeze(PlayerRuntimeState& state, int frame);

	bool IsPlaying(const PlayerRuntimeState& state);
	bool IsDead(const PlayerRuntimeState& state);

private:
	void EnterState(PlayerRuntimeState& state);
	void ExitState(PlayerRuntimeState& state);
	void UpdateFreezeState(PlayerRuntimeState& state);
	void ResetMotion(PlayerRuntimeState& state);
};
