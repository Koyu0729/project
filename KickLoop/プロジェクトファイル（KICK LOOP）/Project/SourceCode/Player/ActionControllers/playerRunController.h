#pragma once

#include "playerRunParameter.h"
#include "playerMoveParameter.h"
#include "playerActionRequest.h"

struct PlayerRuntimeState;
struct PlayerWallKickResult;
class StageBlock;
class PlayerSoundController;

// 壁キックや壁スライドによるコンボ、ボーナスゲージ、走行速度要求を管理するクラス。
class PlayerRunController
{
private:
	int m_Combo = 0;
	float m_BonusGauge = 0.0f;

	// 同じ壁または壁グループで滑ったフレーム数。
	// 規定フレーム数に達すると、滑走中は毎フレームゲージを加算する。
	int m_WallSlideFrame = 0;
	// ゲージ加算音の再生間隔を数える。
	int m_WallSlideGaugeSoundFrame = 0;

	StageBlock* m_WallSlideFrameBlock = nullptr;
	int m_WallSlideFrameGroupId = -1;

	// 同じ壁や同じグループから連続でゲージ加算されないように、
	// 最後にボーナスを得た壁情報を保持する。
	StageBlock* m_LastWallKickBonusBlock = nullptr;
	int m_LastWallKickBonusGroupId = -1;

public:
	void Init();

	void BuildAction(
		PlayerRuntimeState& state,
		PlayerMoveParameter& moveParameter,
		PlayerRunParameter& runParameter,
		const PlayerWallKickResult& wallKickResult,
		PlayerSoundController* soundController,
		PlayerActionRequest& action
	);

	void UpdateWallSlideBonusGauge(
		PlayerRuntimeState& state,
		PlayerRunParameter& runParameter,
		bool isWallSliding,
		PlayerSoundController* soundController
	);

	int GetCombo();
	float GetBonusGauge();

	float GetBonusGaugeRate(
		PlayerRunParameter& runParameter
	);

	bool IsComboActive();
	bool ConsumeBonusGauge(float amount);

private:
	// 壁キックや壁スライドによるゲージ加算を管理する。
	// 同じ壁からの連続加算を防ぎ、コンボ数に応じて加算量を変える。
	void ResetWallSlideBonusFrame();

	bool CanAddWallKickBonus(
		StageBlock* wallKickBlock
	);

	void AddWallKickBonusGauge(
		PlayerRunParameter& runParameter,
		StageBlock* wallKickBlock,
		PlayerSoundController* soundController
	);

	void AddWallSlideBonusGauge(
		PlayerRunParameter& runParameter,
		PlayerSoundController* soundController
	);

	void AddBonusGauge(
		PlayerRunParameter& runParameter,
		float addGauge,
		PlayerSoundController* soundController,
		bool playGaugeSound
	);

	void PlayComboSound(
		PlayerRunParameter& runParameter,
		PlayerSoundController* soundController
	);

	// 現在の走行速度をActionRequestへ渡し、最終反映はResolverに任せる。
	void BuildMoveSpeedAction(
		PlayerMoveParameter& moveParameter,
		PlayerActionRequest& action
	);
};
