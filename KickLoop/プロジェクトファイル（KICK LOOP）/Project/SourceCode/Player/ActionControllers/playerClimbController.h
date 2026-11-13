#pragma once

#include "playerRuntimeState.h"
#include "playerClimbParameter.h"

class StageBlock;
class PlayerSoundController;

struct PlayerActionRequest;
struct PlayerMoveParameter;

// 登れる壁の判定と、よじ登りの開始・補間・終了を管理する。
class PlayerClimbController
{
private:
	PlayerClimbParameter m_ClimbParameter;

public:
	void UpdateBlockRay(
		Vector3 position,
		PlayerRuntimeState& state,
		const PlayerMoveParameter& moveParameter
	);

	bool CanStartLedgeClimb(
		const Vector3& position,
		PlayerRuntimeState& state,
		const PlayerActionRequest& action
	);

	void Start(
		Vector3& position,
		PlayerRuntimeState& state,
		StageBlock* wallBlock,
		PlayerSoundController* soundController = nullptr
	);

	bool Update(
		Vector3& position,
		PlayerRuntimeState& state
	);

	void End(PlayerRuntimeState& state);

private:
	void ClearWallContact(PlayerRuntimeState& state);
	float SmoothStep(float t);
	Vector3 NormalizeXZ(Vector3 vector);
};
