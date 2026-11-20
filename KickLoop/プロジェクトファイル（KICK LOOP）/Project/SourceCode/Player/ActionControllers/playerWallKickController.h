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
	Vector3 wallNormal = Vector3(0.0f, 0.0f, 0.0f);
	Vector3 contactPosition = Vector3(0.0f, 0.0f, 0.0f);
};

// 壁キックの成立判定と、壁キック用の移動要求を作るクラス。
class PlayerWallKickController
{
private:
	const StageBlockManager* m_StageBlockManager = nullptr;

	bool m_WasTouchingWall = false;
	Vector3 m_CoyotePlayerPosition = Vector3(0.0f, 0.0f, 0.0f);  // キックの接触位置を求めるために覚えておく最後の接触時の座標
	Vector3 m_CoyoteWallPosition = Vector3(0.0f, 0.0f, 0.0f);
	Vector3 m_CoyoteWallRotation = Vector3(0.0f, 0.0f, 0.0f);
	Vector3 m_CoyoteWallScale = Vector3(0.0f, 0.0f, 0.0f);

public:
	void SetStageBlockManager(const StageBlockManager* stageBlockManager) { m_StageBlockManager = stageBlockManager; }

	void Reset(PlayerRuntimeState& state);

	// 壁を離れたあとに壁キックを受け付ける猶予時間を更新する
	void UpdateCoyote(
		PlayerRuntimeState& state,
		Vector3 playerPosition,
		const PlayerWallKickParameter& wallKickParameter
	);

	bool CanWallKickOnNextUpdate(const PlayerRuntimeState& state, const PlayerWallKickParameter& wallKickParameter) const;

	// 壁キックの速度と一時停止を要求し、蹴った壁と接触位置を返す
	PlayerWallKickResult BuildAction(
		PlayerRuntimeState& state,
		PlayerWallKickParameter& wallKickParameter,
		PlayerActionRequest& action
	);

private:
	// ステージで有効な壁か確認してから参照する。
	bool IsAvailableWall(const StageBlock* wallBlock) const;
	// 最後に触れたときと壁の位置・回転・大きさが同じか判定する
	bool IsSameCoyoteWall(const StageBlock* wallBlock) const;
	Vector3 GetWallKickNormal(const PlayerRuntimeState& state) const;

	// 接触中の壁と最後に触れた壁から、今キックできる壁を返す
	StageBlock* GetKickableWall(
		const PlayerRuntimeState& state,
		const PlayerWallKickParameter& wallKickParameter
	) const;

	// 壁法線をもとに、壁から離れる方向と前進方向の速度を作る。
	Vector3 BuildWallKickVelocity(
		Vector3 wallNormal,
		PlayerWallKickParameter& wallKickParameter
	);

	Vector3 NormalizeXZ(Vector3 vector);
};
