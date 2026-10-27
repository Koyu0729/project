#pragma once

#include "main.h"

#include "playerRuntimeState.h"
#include "playerWallKickParameter.h"
#include "playerActionRequest.h"

class StageBlock;
class PlayerSoundController;

struct PlayerWallKickResult
{
	bool isSuccess = false;
	StageBlock* wallBlock = nullptr;
};

// 壁キックの成立判定と、壁キック用の移動要求を作るクラス。
class PlayerWallKickController
{
public:
	PlayerWallKickResult BuildAction(
		Vector3 playerPosition,
		PlayerRuntimeState& state,
		PlayerWallKickParameter& wallKickParameter,
		PlayerSoundController* soundController,
		PlayerActionRequest& action
	);

private:
	// 壁に触れている間と、壁を離れた直後は壁キックを許可する。
	bool CanWallKick(
		PlayerRuntimeState& state,
		PlayerWallKickParameter& wallKickParameter,
		StageBlock*& wallBlock
	);

	// 壁法線をもとに、壁から離れる方向と前進方向の速度を作る。
	Vector3 BuildWallKickVelocity(
		PlayerRuntimeState& state,
		PlayerWallKickParameter& wallKickParameter
	);

	Vector3 NormalizeXZ(Vector3 vector);
};
