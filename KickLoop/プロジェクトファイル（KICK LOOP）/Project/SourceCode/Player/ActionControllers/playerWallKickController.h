#pragma once

#include "main.h"

#include "playerRuntimeState.h"
#include "playerWallKickParameter.h"
#include "playerActionRequest.h"

class StageBlock;
class StageBlockManager;

struct PlayerWallKickResult
{
	bool isSuccess = false;
	StageBlock* wallBlock = nullptr;
};

// 壁キックの成立判定と、壁キック用の移動要求を作るクラス。
class PlayerWallKickController
{
private:
	const StageBlockManager* m_StageBlockManager = nullptr;

	bool m_WasTouchingWall = false;
	Vector3 m_CoyoteWallPosition = Vector3(0.0f, 0.0f, 0.0f);
	Vector3 m_CoyoteWallRotation = Vector3(0.0f, 0.0f, 0.0f);
	Vector3 m_CoyoteWallScale = Vector3(0.0f, 0.0f, 0.0f);

public:
	void SetStageBlockManager(const StageBlockManager* stageBlockManager) { m_StageBlockManager = stageBlockManager; }

	void Reset(PlayerRuntimeState& state);

	// 他のジャンプ入力を判定する前に、1フレームに一度呼ぶ。
	void UpdateCoyote(
		PlayerRuntimeState& state,
		const PlayerWallKickParameter& wallKickParameter
	);

	PlayerWallKickResult BuildAction(
		PlayerRuntimeState& state,
		PlayerWallKickParameter& wallKickParameter,
		PlayerActionRequest& action
	);

private:
	// ステージで有効な壁か確認してから参照する。
	bool IsAvailableWall(StageBlock* wallBlock);
	bool IsSameCoyoteWall(StageBlock* wallBlock);

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
