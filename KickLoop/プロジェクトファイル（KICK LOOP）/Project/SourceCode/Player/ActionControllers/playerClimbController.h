#pragma once

#include "main.h"

#include "playerRuntimeState.h"
#include "playerMoveParameter.h"
#include "playerActionRequest.h"
#include "playerClimbParameter.h"

class StageBlock;
class PlayerSoundController;

// よじ登りの開始位置を設定し、到達位置まで補間する。
class PlayerClimbController
{
public:
	void Init();


	void Start(
		Vector3& position,
		PlayerRuntimeState& state,
		StageBlock* wallBlock,
		const Vector3& wallNormal,
		const PlayerClimbParameter& parameter,
		PlayerSoundController* soundController = nullptr
	);

	bool Update(
		Vector3& position,
		PlayerRuntimeState& state,
		PlayerMoveParameter& moveParameter,
		const PlayerClimbParameter& parameter
	);

private:
	// よじ登りの補間と、XZ平面上の方向計算に使う補助処理。
	float SmoothStep(float t);
	Vector3 NormalizeXZ(Vector3 vector);
};
