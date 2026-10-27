#include "playerAirDiveController.h"

#include "input.h"

// 空中ダイブに使う状態を初期化する。
void PlayerAirDiveController::Init()
{
	m_State = PlayerAirDiveState::None;
	m_Frame = 0;
	m_CanDive = true;
}

// 浮上と急降下の速度を要求する
void PlayerAirDiveController::BuildAction(
	PlayerRuntimeState& state,
	PlayerDiveParameter& parameter,
	PlayerActionRequest& action
)
{
	if (state.isGround)
	{
		m_State = PlayerAirDiveState::None;
		m_Frame = 0;
		m_CanDive = true;
		return;
	}

	if (m_State == PlayerAirDiveState::FloatUp)
	{
		UpdateFloatUp(parameter, action);
		return;
	}

	if (m_State == PlayerAirDiveState::DiveDown)
	{
		UpdateDiveDown(parameter, action);
		return;
	}

	if (!m_CanDive)
	{
		return;
	}

	if (!Input::GetKeyTrigger('C'))
	{
		return;
	}

	StartFloatUp(parameter, action);
}

// 急降下前に少し浮き上がる
void PlayerAirDiveController::StartFloatUp(
	PlayerDiveParameter& parameter,
	PlayerActionRequest& action
)
{
	m_State = PlayerAirDiveState::FloatUp;
	m_Frame = parameter.floatUpFrame;
	m_CanDive = false;

	action.RequestAirFloat(parameter.floatUpVelocity, parameter.horizontalBrakeRate);
}

void PlayerAirDiveController::UpdateFloatUp(
	PlayerDiveParameter& parameter,
	PlayerActionRequest& action
)
{
	if (m_Frame > 0)
	{
		m_Frame--;
		action.RequestAirFloat(parameter.floatUpVelocity, parameter.horizontalBrakeRate);
		return;
	}

	m_State = PlayerAirDiveState::DiveDown;
	action.RequestAirDive(parameter.diveDownVelocity, parameter.horizontalBrakeRate);
}

// 浮上後に下向きの速度を要求する
void PlayerAirDiveController::UpdateDiveDown(
	PlayerDiveParameter& parameter,
	PlayerActionRequest& action
)
{
	action.RequestAirDive(parameter.diveDownVelocity, parameter.horizontalBrakeRate);
}
