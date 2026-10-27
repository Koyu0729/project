#pragma once

// カメラ調整用パラメータ。
// 数値をCamera本体から分離し、視点操作や演出を調整しやすくする。
struct CameraParameter
{
	// マウス感度。
	float mouseSensitivity = 0.0035f;

	// ピッチ角度制限。
	float pitchMin = -1.2f;
	float pitchMax = 1.2f;

	// プレイヤーの足元からカメラを置く高さ。
	float eyeHeight = 1.2f;

	// しゃがみ滑走中に使う低めのカメラ高さ。
	float crouchEyeHeight = 0.55f;

	// しゃがみ高さへ下がる補間速度。
	float crouchEyeLerpRate = 0.18f;

	// しゃがみ解除時に通常高さへ戻る補間速度。
	float crouchEyeReturnLerpRate = 0.12f;

	// FPS視点で近距離の壁が欠けにくいよう、小さめに設定する。
	float nearClip = 0.05f;

	// 遠距離クリップ。
	float farClip = 1000.0f;

	// カメラ判定で、プレイヤーのX/Z判定をどれくらい広げるか。
	float cameraCollisionXZMargin = 0.05f;

	// Y方向のカメラ判定厚み。
	float cameraCollisionThinY = 0.01f;

	// 通常時のFOV。
	float normalFOV = 1.771f;

	// 落下中のFOV。
	float fallFOV = 1.80f;

	// FOVインパクトの戻り速度。
	float fovImpactDecay = 0.85f;

	// 通常FOV / 落下FOVへ戻す補間速度。
	float fovLerpRate = 0.08f;

	// 落下中に向かせるPitch。
	float fallTargetPitch = -1.0f;

	// 落下中Pitch補間速度。
	float fallPitchLerpRate = 0.035f;

	// 落下揺れを開始する速度。
	float fallShakeStartSpeed = 0.40f;

	// 落下揺れが最大になる速度。
	float fallShakeMaxSpeed = 1.00f;

	// 落下揺れの最大強さ。
	float fallShakeMaxPower = 0.32f;

	// 落下揺れ速度。
	float fallShakeSpeed = 65.0f;

	// 壁スライド中の最大傾き角度。
	float maxRollDegree = 20.0f;

	// Roll補間速度。
	float rollLerpRate = 0.15f;

	// 壁スライド終了後、少し傾きを残す時間。
	int wallSlideRollKeepFrame = 10;

	// 壁スライド終了後、残っているRollを戻す速さ。
	float wallSlideRollReturnRate = 0.10f;

	// 地上走り中の揺れ強さ。
	float runShakePower = 0.020f;

	// 地上走り中の揺れ速度。
	float runShakeSpeed = 12.0f;

	// 空中時の走り揺れ時間進行倍率。
	float airRunShakeSpeedRate = 0.004f;

	// 壁スライド中の揺れ強さ。
	float wallSlideShakePower = 0.075f;

	// 壁スライド中の揺れ速度。
	float wallSlideShakeSpeed = 22.0f;

	// 壁スライドしていない時の壁スライド揺れ時間進行倍率。
	float wallSlideIdleSpeedRate = 0.002f;

	// 壁キック成功時に一瞬だけ足すFOV量。
	float wallKickFOVImpact = 0.35f;

	// 壁キック成功時の揺れ強さ。
	float wallKickShakePower = 0.16f;

	// インパクト揺れの減衰。
	float impactShakeDecay = 0.80f;

	// 壁キック成功時にPitchへ一瞬足す値。
	float wallKickPitchImpact = -0.06f;

	// Pitchインパクトの減衰。
	float wallKickPitchImpactDecay = 0.82f;

	// 壁キック成功時にRollへ一瞬足す角度。
	float wallKickRollImpactDegree = 6.0f;

	// Rollインパクトの減衰。
	float wallKickRollImpactDecay = 0.80f;

	// 壁キック時、直前の壁スライド傾きをどれくらい残すか。
	float wallKickUseSlideRollRate = 1.0f;
};
