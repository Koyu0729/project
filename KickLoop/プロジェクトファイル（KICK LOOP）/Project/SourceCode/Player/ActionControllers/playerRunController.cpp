#include "playerRunController.h"

#include "playerRuntimeState.h"
#include "stageBlock.h"
#include "playerSoundController.h"

// コンボ、ゲージ、壁アクションの記録を初期化する。
void PlayerRunController::Init()
{
	m_Combo = 0;
	m_BonusGauge = 0.0f;

	ResetWallSlideBonusFrame();

	m_LastWallKickBonusBlock = nullptr;
	m_LastWallKickBonusGroupId = -1;
}

// 壁アクションの結果から走行速度を決める
void PlayerRunController::BuildAction(
	PlayerRuntimeState& state,
	PlayerMoveParameter& moveParameter,
	PlayerRunParameter& runParameter,
	bool isWallKickSuccess,
	StageBlock* wallKickBlock,
	PlayerSoundController* soundController,
	PlayerActionRequest& action
)
{
	if (state.isGround)
	{
		m_Combo = 0;

		m_LastWallKickBonusBlock = nullptr;
		m_LastWallKickBonusGroupId = -1;

		state.oldBonusWallBlock = nullptr;

		ResetWallSlideBonusFrame();
	}

	if (isWallKickSuccess)
	{
		ResetWallSlideBonusFrame();

		if (CanAddWallKickBonus(wallKickBlock))
		{
			AddWallKickBonusGauge(
				runParameter,
				wallKickBlock,
				soundController
			);
		}
	}

	BuildMoveSpeedAction(
		moveParameter,
		action
	);
}

// 同じ壁を滑った時間に応じてゲージを増やす
void PlayerRunController::UpdateWallSlideBonusGauge(
	PlayerRuntimeState& state,
	PlayerRunParameter& runParameter,
	bool isWallSliding,
	PlayerSoundController* soundController
)
{
	if (!isWallSliding)
	{
		ResetWallSlideBonusFrame();
		return;
	}

	StageBlock* wallSlideBlock = state.currentWallBlock;

	if (wallSlideBlock == nullptr)
	{
		ResetWallSlideBonusFrame();
		return;
	}

	if (!wallSlideBlock->CanAddWallKickGauge())
	{
		ResetWallSlideBonusFrame();
		return;
	}

	int groupId = wallSlideBlock->GetWallKickGroupId();
	bool isDifferentWall = false;

	if (groupId >= 0)
	{
		if (m_WallSlideFrameGroupId != groupId)
		{
			isDifferentWall = true;
		}
	}
	else
	{
		if (m_WallSlideFrameBlock != wallSlideBlock)
		{
			isDifferentWall = true;
		}
	}

	if (isDifferentWall)
	{
		m_WallSlideFrameBlock = wallSlideBlock;
		m_WallSlideFrameGroupId = groupId;
		m_WallSlideFrame = 0;
		m_WallSlideGaugeSoundFrame = 0;
	}

	m_WallSlideFrame++;

	if (m_WallSlideFrame < runParameter.wallSlideBonusStartFrame)
	{
		return;
	}

	AddWallSlideBonusGauge(
		runParameter,
		soundController
	);
}

// 壁滑りボーナスの計測状態を初期化する。
void PlayerRunController::ResetWallSlideBonusFrame()
{
	m_WallSlideFrame = 0;
	m_WallSlideGaugeSoundFrame = 0;

	m_WallSlideFrameBlock = nullptr;
	m_WallSlideFrameGroupId = -1;
}

// 同じ壁や同じグループへの連続ボーナスを防ぐ
bool PlayerRunController::CanAddWallKickBonus(
	StageBlock* wallKickBlock
)
{
	if (wallKickBlock == nullptr)
	{
		return false;
	}

	if (!wallKickBlock->CanAddWallKickGauge())
	{
		return false;
	}

	int groupId = wallKickBlock->GetWallKickGroupId();

	if (groupId >= 0)
	{
		if (m_LastWallKickBonusGroupId == groupId)
		{
			return false;
		}

		return true;
	}

	if (m_LastWallKickBonusBlock == wallKickBlock)
	{
		return false;
	}

	return true;
}

// コンボ数に応じてゲージの加算量を増やす
void PlayerRunController::AddWallKickBonusGauge(
	PlayerRunParameter& runParameter,
	StageBlock* wallKickBlock,
	PlayerSoundController* soundController
)
{
	m_Combo++;

	if (m_Combo > runParameter.maxCombo)
	{
		m_Combo = runParameter.maxCombo;
	}

	float gaugeRate = 1.0f + (float)(m_Combo - 1) * runParameter.comboGaugeAddRate;

	if (gaugeRate > runParameter.maxGaugeAddRate)
	{
		gaugeRate = runParameter.maxGaugeAddRate;
	}

	float addGauge = runParameter.baseGaugeAdd * gaugeRate;

	AddBonusGauge(
		runParameter,
		addGauge,
		soundController,
		true
	);

	PlayComboSound(
		runParameter,
		soundController
	);

	m_LastWallKickBonusBlock = wallKickBlock;

	if (wallKickBlock != nullptr)
	{
		m_LastWallKickBonusGroupId = wallKickBlock->GetWallKickGroupId();
	}
	else
	{
		m_LastWallKickBonusGroupId = -1;
	}
}

// 壁滑り時間に応じてボーナスゲージを加算する。
void PlayerRunController::AddWallSlideBonusGauge(
	PlayerRunParameter& runParameter,
	PlayerSoundController* soundController
)
{
	bool playGaugeSound = false;

	// 壁スライドは毎フレーム加算されるため、ゲージ加算音が鳴りすぎないように間隔を空ける。
	if (m_WallSlideGaugeSoundFrame <= 0)
	{
		playGaugeSound = true;
		m_WallSlideGaugeSoundFrame = 10;
	}
	else
	{
		m_WallSlideGaugeSoundFrame--;
	}

	AddBonusGauge(
		runParameter,
		runParameter.wallSlideGaugeAddPerFrame,
		soundController,
		playGaugeSound
	);
}

// ゲージの上限を補正して加算音を鳴らす
void PlayerRunController::AddBonusGauge(
	PlayerRunParameter& runParameter,
	float addGauge,
	PlayerSoundController* soundController,
	bool playGaugeSound
)
{
	float oldBonusGauge = m_BonusGauge;

	m_BonusGauge += addGauge;

	if (m_BonusGauge > runParameter.maxBonusGauge)
	{
		m_BonusGauge = runParameter.maxBonusGauge;
	}

	if (m_BonusGauge < 0.0f)
	{
		m_BonusGauge = 0.0f;
	}

	if (playGaugeSound)
	{
		if (soundController != nullptr)
		{
			if (m_BonusGauge > oldBonusGauge)
			{
				soundController->PlayComboGauge();
			}
		}
	}
}

void PlayerRunController::PlayComboSound(
	PlayerRunParameter& runParameter,
	PlayerSoundController* soundController
)
{
	if (soundController == nullptr)
	{
		return;
	}

	if (m_Combo >= runParameter.maxCombo)
	{
		soundController->PlayComboLast();
		return;
	}

	soundController->PlayCombo(m_Combo);
}

void PlayerRunController::BuildMoveSpeedAction(
	PlayerMoveParameter& moveParameter,
	PlayerActionRequest& action
)
{
	float moveSpeed = moveParameter.baseMoveSpeed;

	if (moveSpeed > moveParameter.maxMoveSpeed)
	{
		moveSpeed = moveParameter.maxMoveSpeed;
	}

	if (moveSpeed < moveParameter.minMoveSpeed)
	{
		moveSpeed = moveParameter.minMoveSpeed;
	}

	action.SetResolvedMoveSpeed(moveSpeed);
}

int PlayerRunController::GetCombo()
{
	return m_Combo;
}

float PlayerRunController::GetBonusGauge()
{
	return m_BonusGauge;
}

float PlayerRunController::GetBonusGaugeRate(
	PlayerRunParameter& runParameter
)
{
	float gaugeRate = 1.0f + (float)(m_Combo - 1) * runParameter.comboGaugeAddRate;

	if (gaugeRate > runParameter.maxGaugeAddRate)
	{
		gaugeRate = runParameter.maxGaugeAddRate;
	}

	return gaugeRate;
}

bool PlayerRunController::IsComboActive()
{
	return m_Combo > 0;
}

bool PlayerRunController::ConsumeBonusGauge(float amount)
{
	if (m_BonusGauge < amount)
	{
		return false;
	}

	m_BonusGauge -= amount;

	if (m_BonusGauge < 0.0f)
	{
		m_BonusGauge = 0.0f;
	}

	return true;
}
