#pragma once

#include <string>
#include <vector>

#include "main.h"

// ランゲームのステージを「部屋」単位で定義するためのデータ群。
// ブロック、レーザー、ジャンプ台、タイムコインなどを同じ形式で持ち、
// StageModuleManagerで各ルームを組み立てる。

// 生成するルームの種類。
enum class StageRoomType
{
	None,

	JumpOnlyRoom,

	WallKickWaveRoom,
	WallKickRoom,

	ZigzagRoom,

	WallSlideRoom,

	JumpPadConnectRoom,
};

// ルーム内に配置するオブジェクトの種類。
enum class StageModuleObjectType
{
	Block,
	LaserWall,
	JumpPad,
	TimeCoin,

	CommentUI
};

// レーザーをどの平面に配置するかを表す。
enum class StageModuleLaserPlaneType
{
	VerticalXY,
	HorizontalXZ,
	SideYZ
};

// ブロックの色と、プレイヤーが使える壁アクションをまとめたプリセット。
enum class StageBlockPresetType
{
	Default,

	// 赤：障害物。壁アクション不可
	NoAction,

	// 白：壁キック・壁滑り・よじ登りが可能
	WallAction,

	// 緑：よじ登りのみ可能
	ClimbOnly,

	// 青：壁キックのみ可能
	WallKickOnly,

	// 黄：壁滑りのみ可能
	WallSlideOnly,
};

// レーザー壁の見た目、出現演出、時間ペナルティをまとめた設定。
struct StageModuleLaserWallSetting
{
	bool useStartAnimation = true;

	float appearStartDistance = 60.0f;

	bool enablePatternAnimation = false;

	int verticalLineCount = 6;

	int horizontalLineCount = 6;

	// Time Penalty
	// 過去の減速系設定は使わず、現在はTime側を直接減らす。
	bool useTimePenaltyEffect = true;

	// レーザーに当たった時に減らす秒数
	float timePenalty = 2.0f;

	// 連続で時間が減り続けないようにするクールタイム
	int timePenaltyCooldownFrame = 180;

	bool useGameOverEffect = false;

	StageModuleLaserPlaneType planeType =
		StageModuleLaserPlaneType::VerticalXY;
};

// ジャンプ台の加速量、再発動時間、ゴール演出用の設定。
struct StageModuleJumpPadSetting
{
	// 通常ジャンプ台の加速量
	Vector3 jumpVelocity =
		Vector3(
			0.0f,
			1.15f,
			5.8f
		);

	// プレイヤー側で浮かせる力の倍率
	float floatPowerRate = 5.0f;

	// 再発動までのクールタイム
	int cooldownFrame = 25;

	// JumpPadモデルのY回転
	float rotationY = 0.0f;

	// ジャンプ演出を使うか
	bool useJumpEffect = true;

	// trueなら通常ジャンプではなく、
	// Player::StartGoalLaunch() でゴール方向へ飛ばす。
	bool useGoalLaunch = false;

	Vector3 goalTargetPosition =
		Vector3(
			25.0f,
			180.0f,
			5400.0f
		);

	int goalLaunchFrameMax = 120;

	float goalLaunchHeight = 60.0f;
};

// タイムコインの加算時間と当たり判定設定。
struct StageModuleTimeCoinSetting
{
	float addTime = 5.0f;

	Vector3 hitSize =
		Vector3(
			0.7f,
			0.7f,
			0.7f
		);

	std::string texturePath =
		"asset\\texture\\siro.png";
};

// ルーム内に配置する1つのオブジェクト情報。
// objectTypeによって、block / laser / jumpPad / timeCoin として使い分ける。
struct StageModuleObjectData
{
	StageModuleObjectType objectType =
		StageModuleObjectType::Block;

	Vector3 localPosition =
		Vector3(
			0.0f,
			0.0f,
			0.0f
		);

	Vector3 size =
		Vector3(
			1.0f,
			1.0f,
			1.0f
		);

	// Block用の見た目回転
	// ラジアン指定
	Vector3 rotation =
		Vector3(
			0.0f,
			0.0f,
			0.0f
		);

	StageBlockPresetType blockPreset =
		StageBlockPresetType::Default;

	// Block用のOBB当たり判定を使うか
	// falseなら従来通りAABB
	bool useOBBHit = false;

	int wallKickGroupId = -1;

	bool canAddWallKickGauge = true;

	// Player Action Permission
	// 初期状態ではすべての壁アクションを許可する
	// 障害物 / よじ登り専用 / 壁キック専用などは false にする
	bool canWallKick = true;

	bool canWallSlide = true;

	bool canClimb = true;

	std::string texturePath =
		"asset\\texture\\siro.png";

	XMFLOAT2 uvRepeat =
		XMFLOAT2(
			1.0f,
			1.0f
		);

	// マスクの白い部分に乗せる色
	// StageMaskBlockでは、この色をマスク部分に反映する
	XMFLOAT4 color =
		XMFLOAT4(
			0.0f,
			0.85f,
			1.0f,
			1.0f
		);

	bool isRainbow = false;

	float rainbowSpeed = 1.0f;

	float alpha = 1.0f;

	std::string maskTexturePath =
		"asset\\texture\\cyberWallMask.png";

	// マスク色の強さ
	float emissivePower = 1.8f;

	// マスクの薄い部分に色を適用する範囲
	float maskColorStart = 0.08f;
	float maskColorEnd = 0.60f;

	// 芯を強く上書きする範囲
	float maskCoreStart = 0.15f;
	float maskCoreEnd = 0.65f;

	StageModuleLaserWallSetting laserSetting;

	StageModuleJumpPadSetting jumpPadSetting;

	StageModuleTimeCoinSetting timeCoinSetting;
};

// 1つのルームを構成するデータ。
// ルーム長、歩けるX範囲、配置オブジェクトをまとめて持つ。
struct StageModuleData
{
	std::string name = "None";

	StageRoomType roomType =
		StageRoomType::None;

	int difficulty = 0;

	float length = 40.0f;

	// このRoom内でプレイヤーが動けるX範囲。
	// 実際の範囲は walkCenterX ± walkHalfSizeX。
	// 例：
	// walkCenterX = 25
	// walkHalfSizeX = 10
	// → X 15 ～ 35 の範囲だけ動ける。
	float walkCenterX = 25.0f;
	float walkHalfSizeX = 10.0f;

	std::vector<StageModuleObjectData> objects;
};

// ブロックプリセットを反映する。
// 色と壁アクション可否をまとめて設定し、ステージ側の配置コードを短くする。
inline void ApplyBlockPreset(
	StageModuleObjectData& block,
	StageBlockPresetType preset
)
{
	block.blockPreset = preset;

	block.isRainbow = false;
	block.rainbowSpeed = 1.0f;
	block.alpha = 1.0f;

	block.texturePath =
		"asset\\texture\\cyberWall.png";

	block.maskTexturePath =
		"asset\\texture\\cyberWallMask.png";

	// マスク部分の色を強く反映する設定
	block.emissivePower = 1.8f;

	block.maskColorStart = 0.08f;
	block.maskColorEnd = 0.60f;

	block.maskCoreStart = 0.15f;
	block.maskCoreEnd = 0.65f;

	if (preset == StageBlockPresetType::Default)
	{
		block.color =
			XMFLOAT4(
				0.0f,
				0.85f,
				1.0f,
				1.0f
			);

		block.canWallKick = true;
		block.canWallSlide = true;
		block.canClimb = true;

		return;
	}

	if (preset == StageBlockPresetType::NoAction)
	{
		block.color =
			XMFLOAT4(
				1.0f,
				0.0f,
				0.0f,
				1.0f
			);

		block.canWallKick = false;
		block.canWallSlide = false;
		block.canClimb = false;

		return;
	}

	if (preset == StageBlockPresetType::WallAction)
	{
		block.color =
			XMFLOAT4(
				1.0f,
				1.0f,
				1.0f,
				1.0f
			);

		block.canWallKick = true;
		block.canWallSlide = true;
		block.canClimb = true;

		return;
	}

	if (preset == StageBlockPresetType::ClimbOnly)
	{
		block.color =
			XMFLOAT4(
				0.0f,
				1.0f,
				0.0f,
				1.0f
			);

		block.canWallKick = false;
		block.canWallSlide = false;
		block.canClimb = true;

		return;
	}

	if (preset == StageBlockPresetType::WallKickOnly)
	{
		block.color =
			XMFLOAT4(
				0.0f,
				0.25f,
				1.0f,
				1.0f
			);

		block.canWallKick = true;
		block.canWallSlide = true;
		block.canClimb = false;

		return;
	}

	if (preset == StageBlockPresetType::WallSlideOnly)
	{
		block.color =
			XMFLOAT4(
				1.0f,
				0.9f,
				0.0f,
				1.0f
			);

		block.canWallKick = false;
		block.canWallSlide = true;
		block.canClimb = false;

		return;
	}
}