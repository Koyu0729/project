#include "stageModuleManager.h"

#include <cstdlib>

namespace
{
	StageModuleObjectData CreateRoomBlock(Vector3 position, Vector3 size, StageBlockPresetType preset)
	{
		StageModuleObjectData block;
		block.objectType = StageModuleObjectType::Block;
		block.localPosition = position;
		block.size = size;
		ApplyBlockPreset(block, preset);
		return block;
	}

	StageModuleObjectData CreateRoomTimeCoin(Vector3 position, float size, float addTime)
	{
		StageModuleObjectData coin;
		coin.objectType = StageModuleObjectType::TimeCoin;
		coin.localPosition = position;
		coin.size = Vector3(size, size, size);
		coin.timeCoinSetting.hitSize = coin.size;
		coin.timeCoinSetting.addTime = addTime;
		return coin;
	}

	StageModuleObjectData CreateRoomLaserWall(Vector3 position, Vector3 size)
	{
		StageModuleObjectData laser;
		laser.objectType = StageModuleObjectType::LaserWall;
		laser.localPosition = position;
		laser.size = size;
		laser.laserSetting.timePenalty = 5.0f;
		return laser;
	}

	StageModuleObjectData CreateRoomJumpPad(Vector3 position, Vector3 jumpVelocity, int cooldownFrame)
	{
		StageModuleObjectData jumpPad;
		jumpPad.objectType = StageModuleObjectType::JumpPad;
		jumpPad.localPosition = position;
		jumpPad.jumpPadSetting.jumpVelocity = jumpVelocity;
		jumpPad.jumpPadSetting.cooldownFrame = cooldownFrame;
		return jumpPad;
	}
}

// 通常区間と最終区間の構成を登録する
void StageModuleManager::InitDefaultModules()
{
	m_Modules.clear();

	m_Modules.push_back(CreateJumpOnlyRoom());
	m_Modules.push_back(CreateZigzagRoom());

	m_Modules.push_back(CreateWallKickWaveRoom());
	m_Modules.push_back(CreateWallKickRoom());

	m_Modules.push_back(CreateWallSlideRoom());
	m_Modules.push_back(CreateLaserWallSlideRoom());

	m_Modules.push_back(CreateJumpPadConnectRoom());

	m_FinalJumpPadRoom = CreateFinalJumpPadRoom();
}

const std::vector<StageModuleData>& StageModuleManager::GetModules() const
{
	return m_Modules;
}

const StageModuleData* StageModuleManager::GetModule(int index) const
{
	if (index < 0)
	{
		return nullptr;
	}

	if (index >= (int)m_Modules.size())
	{
		return nullptr;
	}

	return &m_Modules[index];
}

const StageModuleData* StageModuleManager::GetRandomModule() const
{
	if (m_Modules.empty())
	{
		return nullptr;
	}

	int index = rand() % (int)m_Modules.size();

	return &m_Modules[index];
}

const StageModuleData* StageModuleManager::GetFinalJumpPadRoom() const
{
	return &m_FinalJumpPadRoom;
}

// ジャンプとよじ登り用の区間を作る
StageModuleData StageModuleManager::CreateJumpOnlyRoom()
{
	StageModuleData room;

	room.name = "JumpOnlyRoom";
	room.roomType = StageRoomType::JumpOnlyRoom;
	room.difficulty = 0;
	room.length = 85.0f;

	// Block 1 : first jump obstacle
	room.objects.push_back(CreateRoomBlock(
		Vector3(31.0f, 3.5f, 0.0f),
		Vector3(4.0f, 3.0f, 4.0f),
		StageBlockPresetType::NoAction
	));

	// Block 2 : second jump obstacle
	room.objects.push_back(CreateRoomBlock(
		Vector3(27.0f, 7.5f, 18.0f),
		Vector3(8.0f, 7.0f, 4.0f),
		StageBlockPresetType::WallAction
	));

	// Block 3 : slightly wider jump obstacle
	room.objects.push_back(CreateRoomBlock(
		Vector3(25.0f, 10.5f, 36.0f),
		Vector3(10.0f, 10.0f, 4.0f),
		StageBlockPresetType::ClimbOnly
	));

	return room;
}

// 左右の壁を交互に使う区間を作る
StageModuleData StageModuleManager::CreateZigzagRoom()
{
	StageModuleData room;

	room.name = "ZigzagRoom";
	room.roomType = StageRoomType::ZigzagRoom;
	room.difficulty = 1;
	room.length = 85.0f;

	const Vector3 wallSize = Vector3(8.0f, 7.0f, 2.0f);

	// Block 1 leaves a gap on the right.
	room.objects.push_back(CreateRoomBlock(
		Vector3(23.0f, 7.5f, 15.0f),
		wallSize,
		StageBlockPresetType::WallKickOnly
	));

	// Block 2 leaves a gap on the left.
	room.objects.push_back(CreateRoomBlock(
		Vector3(27.0f, 7.5f, 32.0f),
		wallSize,
		StageBlockPresetType::NoAction
	));

	// Block 3 leaves a gap on the right.
	room.objects.push_back(CreateRoomBlock(
		Vector3(23.0f, 7.5f, 51.0f),
		wallSize,
		StageBlockPresetType::WallKickOnly
	));

	return room;
}

// 壁キック用の壁を連続して配置する
StageModuleData StageModuleManager::CreateWallKickWaveRoom()
{
	StageModuleData room;

	room.name = "WallKickWaveRoom";
	room.roomType = StageRoomType::WallKickWaveRoom;
	room.difficulty = 1;
	room.length = 190.0f;

	const Vector3 wallSize = Vector3(2.0f, 6.0f, 8.0f);
	const float wallPositionsX[] = { 17.0f, 33.0f };

	for (float x : wallPositionsX)
	{
		for (int i = 0; i < 5; i++)
		{
			Vector3 position = Vector3(x, 14.5f + (8.2f * i), 15.0f + (30.0f * i));
			room.objects.push_back(CreateRoomBlock(position, wallSize, StageBlockPresetType::WallKickOnly));
		}
	}

	for (int i = 0; i < 5; i++)
	{
		Vector3 position = Vector3(25.0f, 6.5f + (8.5f * i), 0.0f + (30.0f * i));
		room.objects.push_back(CreateRoomTimeCoin(position, 3.0f, 2.0f));
	}

	room.objects.push_back(CreateRoomLaserWall(
		Vector3(25.0f, 10.5f, 75.0f),
		Vector3(10.0f, 9.8f, 2.0f)
	));
	room.objects.push_back(CreateRoomLaserWall(
		Vector3(25.0f, 24.5f, 135.0f),
		Vector3(10.0f, 14.8f, 2.0f)
	));

	return room;
}

// 壁キック中心の区間を作る
StageModuleData StageModuleManager::CreateWallKickRoom()
{
	StageModuleData room;

	room.name = "WallKickRoom";
	room.roomType = StageRoomType::WallKickRoom;
	room.difficulty = 2;
	room.length = 200.0f;

	const Vector3 wallSize = Vector3(2.0f, 6.0f, 8.0f);

	// Left wall
	for (int i = 0; i < 3; i++)
	{
		Vector3 wallPosition = Vector3(17.0f, 14.5f + (16.1f * i), 0.0f + (60.0f * i));
		room.objects.push_back(CreateRoomBlock(wallPosition, wallSize, StageBlockPresetType::WallKickOnly));

		Vector3 coinPosition = Vector3(21.0f, 14.5f + (16.0f * i), 0.0f + (60.0f * i));
		room.objects.push_back(CreateRoomTimeCoin(coinPosition, 3.0f, 2.0f));
	}

	// Right wall
	for (int i = 0; i < 3; i++)
	{
		Vector3 wallPosition = Vector3(33.0f, 22.5f + (16.0f * i), 30.0f + (60.0f * i));
		room.objects.push_back(CreateRoomBlock(wallPosition, wallSize, StageBlockPresetType::WallKickOnly));

		Vector3 coinPosition = Vector3(29.0f, 22.5f + (16.0f * i), 30.0f + (60.0f * i));
		room.objects.push_back(CreateRoomTimeCoin(coinPosition, 3.0f, 5.0f));
	}

	room.objects.push_back(CreateRoomLaserWall(
		Vector3(25.0f, 10.5f, 60.0f),
		Vector3(10.0f, 9.8f, 2.0f)
	));
	room.objects.push_back(CreateRoomLaserWall(
		Vector3(25.0f, 24.5f, 120.0f),
		Vector3(10.0f, 14.8f, 2.0f)
	));

	return room;
}

// 壁滑りとレーザー回避の区間を作る
StageModuleData StageModuleManager::CreateLaserWallSlideRoom()
{
	StageModuleData room;

	room.name = "LaserWallSlideRoom";
	room.roomType = StageRoomType::WallSlideRoom;
	room.difficulty = 1;
	room.length = 220.0f;

	struct WallSide
	{
		float x;
		int wallKickGroupId;
	};

	const WallSide wallSides[] = { {18.0f, 1}, {32.0f, 2} };
	const Vector3 sideWallSize = Vector3(3.0f, 6.0f, 10.0f);

	for (const WallSide& side : wallSides)
	{
		for (int i = 0; i < 7; i++)
		{
			for (int j = 0; j <= i; j++)
			{
				Vector3 position = Vector3(side.x, 6.5f + (j * 11.9f), 40.0f + (i * 19.9f));
				StageModuleObjectData wall = CreateRoomBlock(position, sideWallSize, StageBlockPresetType::WallKickOnly);
				wall.wallKickGroupId = side.wallKickGroupId;
				room.objects.push_back(wall);
			}
		}
	}

	// Center climb blocks
	const Vector3 climbBlockSize = Vector3(4.1f, 6.0f, 10.0f);

	for (int i = 0; i < 5; i++)
	{
		for (int j = 0; j <= i; j++)
		{
			Vector3 position = Vector3(25.0f, 6.5f + (j * 12.0f), 80.0f + (i * 19.9f));
			room.objects.push_back(CreateRoomBlock(position, climbBlockSize, StageBlockPresetType::ClimbOnly));
		}
	}

	for (int i = 0; i < 7; i++)
	{
		Vector3 position = Vector3(25.0f, 6.5f + (i * 12.0f), 44.0f + (20.0f * i));
		room.objects.push_back(CreateRoomTimeCoin(position, 3.0f, 1.0f));
	}

	return room;
}

// 壁滑りで高さを調整する区間を作る
StageModuleData StageModuleManager::CreateWallSlideRoom()
{
	StageModuleData room;

	room.name = "WallSlideRoom";
	room.roomType = StageRoomType::WallSlideRoom;
	room.difficulty = 1;
	room.length = 220.0f;

	room.objects.push_back(CreateRoomJumpPad(
		Vector3(25.0f, 2.0f, 0.0f),
		Vector3(0.0f, 1.25f, 0.0f),
		20
	));

	const Vector3 wallSize = Vector3(2.0f, 12.0f, 30.0f);
	const Vector3 wallRotation = Vector3(0.20f, 0.0f, 0.0f);

	// Left slide wall
	StageModuleObjectData leftWall = CreateRoomBlock(
		Vector3(17.0f, 45.5f, 45.0f),
		wallSize,
		StageBlockPresetType::WallKickOnly
	);
	leftWall.rotation = wallRotation;
	leftWall.useOBBHit = true;
	room.objects.push_back(leftWall);

	for (int i = 0; i < 5; i++)
	{
		Vector3 position = Vector3(21.0f, 48.5f - (1.0f * i), 25.0f + (9.0f * i));
		room.objects.push_back(CreateRoomTimeCoin(position, 3.0f, 1.0f));
	}

	// Right slide wall
	StageModuleObjectData rightWall = CreateRoomBlock(
		Vector3(33.0f, 45.5f, 120.0f),
		wallSize,
		StageBlockPresetType::WallSlideOnly
	);
	rightWall.rotation = wallRotation;
	rightWall.useOBBHit = true;
	room.objects.push_back(rightWall);

	for (int i = 0; i < 5; i++)
	{
		Vector3 position = Vector3(29.0f, 45.5f - (1.0f * i), 105.0f + (9.0f * i));
		room.objects.push_back(CreateRoomTimeCoin(position, 3.0f, 1.0f));
	}

	// HorizontalXZ: sizeはX方向・Z方向・高さの半分。
	const Vector3 laserSize = Vector3(10.0f, 35.0f, 0.25f);

	for (int i = 0; i < 4; i++)
	{
		Vector3 position = Vector3(25.0f, 6.2f, 40.0f + (35.0f * i));
		StageModuleObjectData laser = CreateRoomLaserWall(position, laserSize);

		laser.laserSetting.planeType = StageModuleLaserPlaneType::HorizontalXZ;
		laser.laserSetting.appearStartDistance = 80.0f;
		laser.laserSetting.verticalLineCount = 10;
		laser.laserSetting.horizontalLineCount = 15;

		room.objects.push_back(laser);
	}

	// 報酬床
	room.objects.push_back(CreateRoomBlock(
		Vector3(25.0f, 26.5f, 160.0f),
		Vector3(10.0f, 2.0f, 10.0f),
		StageBlockPresetType::ClimbOnly
	));

	// 報酬
	room.objects.push_back(CreateRoomJumpPad(
		Vector3(25.0f, 28.5f, 160.0f),
		Vector3(0.0f, 1.35f, 4.5f),
		20
	));

	return room;
}

// ジャンプ台で区間をつなぐ
StageModuleData StageModuleManager::CreateJumpPadConnectRoom()
{
	StageModuleData room;

	room.name = "JumpPadConnectRoom";
	room.roomType = StageRoomType::JumpPadConnectRoom;
	room.difficulty = 1;
	room.length = 55.0f;

	room.objects.push_back(CreateRoomJumpPad(
		Vector3(25.0f, 1.55f, 18.0f),
		Vector3(0.0f, 1.15f, 5.8f),
		25
	));

	room.objects.push_back(CreateRoomTimeCoin(Vector3(25.0f, 1.55f, 12.0f), 2.0f, 5.0f));

	// 柱
	StageModuleObjectData obstacle = CreateRoomBlock(
		Vector3(31.0f, 5.5f, 34.0f),
		Vector3(2.0f, 5.0f, 2.0f),
		StageBlockPresetType::WallAction
	);
	obstacle.wallKickGroupId = 401;
	room.objects.push_back(obstacle);

	return room;
}

// ゴールへ飛ばす最後の区間を作る
StageModuleData StageModuleManager::CreateFinalJumpPadRoom()
{
	StageModuleData room;

	room.name = "FinalJumpPadRoom";
	room.roomType = StageRoomType::JumpPadConnectRoom;
	room.difficulty = 999;
	room.length = 80.0f;

	room.walkCenterX = 25.0f;
	room.walkHalfSizeX = 10.0f;

	const float blockPositionsX[] = { 16.0f, 34.0f };
	const Vector3 blockSize = Vector3(1.0f, 6.0f, 1.0f);

	for (float x : blockPositionsX)
	{
		Vector3 position = Vector3(x, 6.5f, 30.0f);
		StageModuleObjectData block = CreateRoomBlock(position, blockSize, StageBlockPresetType::NoAction);
		block.color = XMFLOAT4(0.0f, 0.85f, 1.0f, 1.0f);
		block.emissivePower = 1.3f;
		room.objects.push_back(block);
	}

	StageModuleObjectData jumpPad = CreateRoomJumpPad(
		Vector3(25.0f, 1.55f, 30.0f),
		Vector3(0.0f, 0.0f, 0.0f),
		9999
	);
	jumpPad.jumpPadSetting.floatPowerRate = 0.0f;
	jumpPad.jumpPadSetting.useGoalLaunch = true;
	jumpPad.jumpPadSetting.goalTargetPosition = Vector3(25.0f, 180.0f, 75.0f);
	jumpPad.jumpPadSetting.goalLaunchFrameMax = 120;
	jumpPad.jumpPadSetting.goalLaunchHeight = 60.0f;
	room.objects.push_back(jumpPad);

	return room;
}
