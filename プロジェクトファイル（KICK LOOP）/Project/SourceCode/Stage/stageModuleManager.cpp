#include "stageModuleManager.h"

#include <cstdlib>

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
	{
		StageModuleObjectData obstacle;

		obstacle.objectType = StageModuleObjectType::Block;

		obstacle.localPosition =
			Vector3(
				31.0f,
				3.5f,
				0.0f
			);

		obstacle.size =
			Vector3(
				4.0f,
				3.0f,
				4.0f
			);

		ApplyBlockPreset(
			obstacle,
			StageBlockPresetType::NoAction
		);

		room.objects.push_back(obstacle);
	}

	// Block 2 : second jump obstacle
	{
		StageModuleObjectData obstacle;

		obstacle.objectType = StageModuleObjectType::Block;

		obstacle.localPosition =
			Vector3(
				27.0f,
				7.5f,
				18.0f
			);

		obstacle.size =
			Vector3(
				8.0f,
				7.0f,
				4.0f
			);

		ApplyBlockPreset(
			obstacle,
			StageBlockPresetType::WallAction
		);

		room.objects.push_back(obstacle);
	}

	// Block 3 : slightly wider jump obstacle
	{
		StageModuleObjectData obstacle;

		obstacle.objectType = StageModuleObjectType::Block;

		obstacle.localPosition =
			Vector3(
				25.0f,
				10.5f,
				36.0f
			);

		obstacle.size =
			Vector3(
				10.0f,
				10.0f,
				4.0f
			);

		ApplyBlockPreset(
			obstacle,
			StageBlockPresetType::ClimbOnly
		);

		room.objects.push_back(obstacle);
	}

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

	// Block 1 : right side wall
	{
		StageModuleObjectData obstacle;

		obstacle.objectType = StageModuleObjectType::Block;

		obstacle.localPosition =
			Vector3(
				23.0f,
				7.5f,
				15.0f
			);

		obstacle.size =
			Vector3(
				8.0f,
				7.0f,
				2.0f
			);

		ApplyBlockPreset(
			obstacle,
			StageBlockPresetType::WallKickOnly
		);

		room.objects.push_back(obstacle);
	}

	// Block 2 : center and right side wall
	{
		StageModuleObjectData obstacle;

		obstacle.objectType = StageModuleObjectType::Block;

		obstacle.localPosition =
			Vector3(
				27.0f,
				7.5f,
				32.0f
			);

		obstacle.size =
			Vector3(
				8.0f,
				7.0f,
				2.0f
			);

		ApplyBlockPreset(
			obstacle,
			StageBlockPresetType::NoAction
		);

		room.objects.push_back(obstacle);
	}

	// Block 3 : almost right half blocked
	{
		StageModuleObjectData obstacle;

		obstacle.objectType = StageModuleObjectType::Block;

		obstacle.localPosition =
			Vector3(
				23.0f,
				7.5f,
				51.0f
			);

		obstacle.size =
			Vector3(
				8.0f,
				7.0f,
				2.0f
			);

		ApplyBlockPreset(
			obstacle,
			StageBlockPresetType::WallKickOnly
		);

		room.objects.push_back(obstacle);
	}

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

	// Left wall
	for (int i = 0; i < 5; i++)
	{
		StageModuleObjectData obstacle;

		obstacle.objectType = StageModuleObjectType::Block;

		obstacle.localPosition =
			Vector3(
				17.0f,
				14.5f + (8.2f * i),
				15.0f + (30.0f * i)
			);

		obstacle.size =
			Vector3(
				2.0f,
				6.0f,
				8.0f
			);

		ApplyBlockPreset(
			obstacle,
			StageBlockPresetType::WallKickOnly
		);

		room.objects.push_back(obstacle);
	}

	// Right wall
	for (int i = 0; i < 5; i++)
	{
		StageModuleObjectData obstacle;

		obstacle.objectType = StageModuleObjectType::Block;

		obstacle.localPosition =
			Vector3(
				33.0f,
				14.5f + (8.2f * i),
				15.0f + (30.0f * i)
			);

		obstacle.size =
			Vector3(
				2.0f,
				6.0f,
				8.0f
			);

		ApplyBlockPreset(
			obstacle,
			StageBlockPresetType::WallKickOnly
		);

		room.objects.push_back(obstacle);
	}

	// TimeCoin
	for (int i = 0; i < 5; i++)
	{
		StageModuleObjectData coin;

		coin.objectType = StageModuleObjectType::TimeCoin;

		coin.localPosition =
			Vector3(
				25.0f,
				6.5f + (8.5f * i),
				0.0f + (30.0f * i)
			);

		coin.size =
			Vector3(
				3.0f,
				3.0f,
				3.0f
			);

		coin.timeCoinSetting.texturePath = "asset\\texture\\siro.png";

		coin.timeCoinSetting.addTime = 2.0f;

		coin.timeCoinSetting.hitSize = coin.size;

		room.objects.push_back(coin);
	}

	// Laser wall 1
	{
		StageModuleObjectData laser;

		laser.objectType = StageModuleObjectType::LaserWall;

		laser.localPosition =
			Vector3(
				25.0f,
				10.5f,
				75.0f
			);

		laser.size =
			Vector3(
				10.0f,
				9.8f,
				2.0f
			);

		laser.texturePath = "asset\\texture\\siro.png";

		laser.color =
			XMFLOAT4(
				0.0f,
				0.85f,
				1.0f,
				1.0f
			);

		laser.laserSetting.useTimePenaltyEffect = true;
		laser.laserSetting.timePenalty = 5.0f;
		laser.laserSetting.timePenaltyCooldownFrame = 180;

		room.objects.push_back(laser);
	}

	// Laser wall 2
	{
		StageModuleObjectData laser;

		laser.objectType = StageModuleObjectType::LaserWall;

		laser.localPosition =
			Vector3(
				25.0f,
				24.5f,
				135.0f
			);

		laser.size =
			Vector3(
				10.0f,
				14.8f,
				2.0f
			);

		laser.texturePath = "asset\\texture\\siro.png";

		laser.color =
			XMFLOAT4(
				0.0f,
				0.85f,
				1.0f,
				1.0f
			);

		laser.laserSetting.useTimePenaltyEffect = true;
		laser.laserSetting.timePenalty = 5.0f;
		laser.laserSetting.timePenaltyCooldownFrame = 180;

		room.objects.push_back(laser);
	}

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

	// Left wall
	for (int i = 0; i < 3; i++)
	{
		{
			StageModuleObjectData obstacle;

			obstacle.objectType = StageModuleObjectType::Block;

			obstacle.localPosition =
				Vector3(
					17.0f,
					14.5f + (16.1f * i),
					0.0f + (60.0f * i)
				);

			obstacle.size =
				Vector3(
					2.0f,
					6.0f,
					8.0f
				);

			ApplyBlockPreset(
				obstacle,
				StageBlockPresetType::WallKickOnly
			);

			room.objects.push_back(obstacle);
		}

		// TimeCoin
		{
			StageModuleObjectData coin;

			coin.objectType = StageModuleObjectType::TimeCoin;

			coin.localPosition =
				Vector3(
					21.0f,
					14.5f + (16.0f * i),
					0.0f + (60.0f * i)
				);

			coin.size =
				Vector3(
					3.0f,
					3.0f,
					3.0f
				);

			coin.timeCoinSetting.texturePath = "asset\\texture\\siro.png";

			coin.timeCoinSetting.addTime = 2.0f;

			coin.timeCoinSetting.hitSize = coin.size;

			room.objects.push_back(coin);
		}
	}

	// Right wall
	for (int i = 0; i < 3; i++)
	{
		{
			StageModuleObjectData obstacle;

			obstacle.objectType = StageModuleObjectType::Block;

			obstacle.localPosition =
				Vector3(
					33.0f,
					22.5f + (16.0f * i),
					30.0f + (60.0f * i)
				);

			obstacle.size =
				Vector3(
					2.0f,
					6.0f,
					8.0f
				);

			ApplyBlockPreset(
				obstacle,
				StageBlockPresetType::WallKickOnly
			);

			room.objects.push_back(obstacle);
		}

		// TimeCoin
		{
			StageModuleObjectData coin;

			coin.objectType = StageModuleObjectType::TimeCoin;

			coin.localPosition =
				Vector3(
					29.0f,
					22.5f + (16.0f * i),
					30.0f + (60.0f * i)
				);

			coin.size =
				Vector3(
					3.0f,
					3.0f,
					3.0f
				);

			coin.timeCoinSetting.texturePath = "asset\\texture\\siro.png";

			coin.timeCoinSetting.addTime = 5.0f;

			coin.timeCoinSetting.hitSize = coin.size;

			room.objects.push_back(coin);
		}
	}

	// Laser wall 1
	{
		StageModuleObjectData laser;

		laser.objectType = StageModuleObjectType::LaserWall;

		laser.localPosition =
			Vector3(
				25.0f,
				10.5f,
				60.0f
			);

		laser.size =
			Vector3(
				10.0f,
				9.8f,
				2.0f
			);

		laser.texturePath = "asset\\texture\\siro.png";

		laser.color =
			XMFLOAT4(
				0.0f,
				0.85f,
				1.0f,
				1.0f
			);

		laser.laserSetting.useTimePenaltyEffect = true;
		laser.laserSetting.timePenalty = 5.0f;
		laser.laserSetting.timePenaltyCooldownFrame = 180;

		room.objects.push_back(laser);
	}

	// Laser wall 2
	{
		StageModuleObjectData laser;

		laser.objectType = StageModuleObjectType::LaserWall;

		laser.localPosition =
			Vector3(
				25.0f,
				24.5f,
				120.0f
			);

		laser.size =
			Vector3(
				10.0f,
				14.8f,
				2.0f
			);

		laser.texturePath = "asset\\texture\\siro.png";

		laser.color =
			XMFLOAT4(
				0.0f,
				0.85f,
				1.0f,
				1.0f
			);

		laser.laserSetting.useTimePenaltyEffect = true;
		laser.laserSetting.timePenalty = 5.0f;
		laser.laserSetting.timePenaltyCooldownFrame = 180;

		room.objects.push_back(laser);
	}

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

	int leftWallGroupId = 1;

	for (int i = 0; i < 7; i++)
	{
		for (int j = 0; j <= i; j++)
		{
			StageModuleObjectData wall;

			wall.objectType = StageModuleObjectType::Block;

			wall.localPosition =
				Vector3(
					18.0f,
					6.5f + (j * 11.9f),
					40.0f + (i * 19.9f)
				);

			wall.size =
				Vector3(
					3.0f,
					6.0f,
					10.0f
				);

			wall.uvRepeat =
				XMFLOAT2(
					1.0f,
					1.0f
				);

			ApplyBlockPreset(
				wall,
				StageBlockPresetType::WallKickOnly
			);

			wall.wallKickGroupId = leftWallGroupId;

			room.objects.push_back(wall);
		}
	}

	int rightWallGroupId = 2;

	for (int i = 0; i < 7; i++)
	{
		for (int j = 0; j <= i; j++)
		{
			StageModuleObjectData wall;

			wall.objectType = StageModuleObjectType::Block;

			wall.localPosition =
				Vector3(
					32.0f,
					6.5f + (j * 11.9f),
					40.0f + (i * 19.9f)
				);

			wall.size =
				Vector3(
					3.0f,
					6.0f,
					10.0f
				);

			wall.uvRepeat =
				XMFLOAT2(
					1.0f,
					1.0f
				);

			ApplyBlockPreset(
				wall,
				StageBlockPresetType::WallKickOnly
			);

			wall.wallKickGroupId = rightWallGroupId;

			room.objects.push_back(wall);
		}
	}

	// Center climb blocks
	for (int i = 0; i < 5; i++)
	{
		for (int j = 0; j <= i; j++)
		{
			StageModuleObjectData wall;

			wall.objectType = StageModuleObjectType::Block;

			wall.localPosition =
				Vector3(
					25.0f,
					6.5f + (j * 12.0f),
					80.0f + (i * 19.9f)
				);

			wall.size =
				Vector3(
					4.1f,
					6.0f,
					10.0f
				);

			ApplyBlockPreset(
				wall,
				StageBlockPresetType::ClimbOnly
			);

			room.objects.push_back(wall);
		}
	}

	// TimeCoin
	for (int i = 0; i < 7; i++)
	{
		StageModuleObjectData coin;

		coin.objectType = StageModuleObjectType::TimeCoin;

		coin.localPosition =
			Vector3(
				25.0f,
				6.5f + (i * 12.0f),
				44.0f + (20.0f * i)
			);

		coin.size =
			Vector3(
				3.0f,
				3.0f,
				3.0f
			);

		coin.timeCoinSetting.texturePath = "asset\\texture\\siro.png";

		coin.timeCoinSetting.addTime = 1.0f;

		coin.timeCoinSetting.hitSize = coin.size;

		room.objects.push_back(coin);
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

	// First jump pad
	{
		StageModuleObjectData jumpPad;

		jumpPad.objectType = StageModuleObjectType::JumpPad;

		jumpPad.localPosition =
			Vector3(
				25.0f,
				2.0f,
				0.0f
			);

		jumpPad.size =
			Vector3(
				1.0f,
				1.0f,
				1.0f
			);

		jumpPad.jumpPadSetting.jumpVelocity =
			Vector3(
				0.0f,
				1.25f,
				0.0f
			);

		jumpPad.jumpPadSetting.floatPowerRate = 5.0f;
		jumpPad.jumpPadSetting.cooldownFrame = 20;
		jumpPad.jumpPadSetting.rotationY = 0.0f;
		jumpPad.jumpPadSetting.useJumpEffect = true;

		room.objects.push_back(jumpPad);
	}

	// Left slide wall
	{
		StageModuleObjectData wall;

		wall.objectType = StageModuleObjectType::Block;

		wall.localPosition =
			Vector3(
				17.0f,
				45.5f,
				45.0f
			);

		wall.size =
			Vector3(
				2.0f,
				12.0f,
				30.0f
			);

		wall.rotation =
			Vector3(
				0.20f,
				0.0f,
				0.0f
			);

		wall.useOBBHit = true;

		ApplyBlockPreset(
			wall,
			StageBlockPresetType::WallKickOnly
		);

		room.objects.push_back(wall);
	}

	// Left TimeCoin
	for (int i = 0; i < 5; i++)
	{
		StageModuleObjectData coin;

		coin.objectType = StageModuleObjectType::TimeCoin;

		coin.localPosition =
			Vector3(
				21.0f,
				48.5f - (1.0f * i),
				25.0f + (9.0f * i)
			);

		coin.size =
			Vector3(
				3.0f,
				3.0f,
				3.0f
			);

		coin.timeCoinSetting.texturePath = "asset\\texture\\siro.png";

		coin.timeCoinSetting.addTime = 1.0f;

		coin.timeCoinSetting.hitSize = coin.size;

		room.objects.push_back(coin);
	}

	// Right slide wall
	{
		StageModuleObjectData wall;

		wall.objectType = StageModuleObjectType::Block;

		wall.localPosition =
			Vector3(
				33.0f,
				45.5f,
				120.0f
			);

		wall.size =
			Vector3(
				2.0f,
				12.0f,
				30.0f
			);

		wall.rotation =
			Vector3(
				0.20f,
				0.0f,
				0.0f
			);

		wall.useOBBHit = true;

		ApplyBlockPreset(
			wall,
			StageBlockPresetType::WallSlideOnly
		);

		room.objects.push_back(wall);
	}

	// Right TimeCoin
	for (int i = 0; i < 5; i++)
	{
		StageModuleObjectData coin;

		coin.objectType = StageModuleObjectType::TimeCoin;

		coin.localPosition =
			Vector3(
				29.0f,
				45.5f - (1.0f * i),
				105.0f + (9.0f * i)
			);

		coin.size =
			Vector3(
				3.0f,
				3.0f,
				3.0f
			);

		coin.timeCoinSetting.texturePath = "asset\\texture\\siro.png";

		coin.timeCoinSetting.addTime = 1.0f;

		coin.timeCoinSetting.hitSize = coin.size;

		room.objects.push_back(coin);
	}

	// Horizontal lasers
	for (int i = 0; i < 4; i++)
	{
		StageModuleObjectData laser;

		laser.objectType = StageModuleObjectType::LaserWall;

		laser.localPosition =
			Vector3(
				25.0f,
				6.2f,
				40.0f + (35.0f * i)
			);

		// HorizontalXZ:
		// size.x = X half width
		// size.y = Z half depth
		// size.z = hit height half size
		laser.size =
			Vector3(
				10.0f,
				35.0f,
				0.25f
			);

		laser.texturePath = "asset\\texture\\siro.png";

		laser.color =
			XMFLOAT4(
				0.0f,
				0.85f,
				1.0f,
				1.0f
			);

		laser.laserSetting.planeType = StageModuleLaserPlaneType::HorizontalXZ;

		laser.laserSetting.appearStartDistance = 80.0f;

		laser.laserSetting.enablePatternAnimation = false;

		laser.laserSetting.verticalLineCount = 10;
		laser.laserSetting.horizontalLineCount = 15;

		laser.laserSetting.useTimePenaltyEffect = true;
		laser.laserSetting.timePenalty = 5.0f;
		laser.laserSetting.timePenaltyCooldownFrame = 180;

		laser.laserSetting.useGameOverEffect = false;

		room.objects.push_back(laser);
	}

	// 報酬床
	{
		StageModuleObjectData wall;

		wall.objectType = StageModuleObjectType::Block;

		wall.localPosition =
			Vector3(
				25.0f,
				26.5f,
				160.0f
			);

		wall.size =
			Vector3(
				10.0f,
				2.0f,
				10.0f
			);

		ApplyBlockPreset(
			wall,
			StageBlockPresetType::ClimbOnly
		);

		room.objects.push_back(wall);
	}

	// 報酬
	{
		StageModuleObjectData jumpPad;

		jumpPad.objectType = StageModuleObjectType::JumpPad;

		jumpPad.localPosition =
			Vector3(
				25.0f,
				28.5f,
				160.0f
			);

		jumpPad.size =
			Vector3(
				1.0f,
				1.0f,
				1.0f
			);

		jumpPad.jumpPadSetting.jumpVelocity =
			Vector3(
				0.0f,
				1.35f,
				4.5f
			);

		jumpPad.jumpPadSetting.floatPowerRate = 5.0f;
		jumpPad.jumpPadSetting.cooldownFrame = 20;
		jumpPad.jumpPadSetting.rotationY = 0.0f;
		jumpPad.jumpPadSetting.useJumpEffect = true;

		room.objects.push_back(jumpPad);
	}

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

	// JumpPad
	{
		StageModuleObjectData jumpPad;

		jumpPad.objectType = StageModuleObjectType::JumpPad;

		jumpPad.localPosition =
			Vector3(
				25.0f,
				1.55f,
				18.0f
			);

		jumpPad.size =
			Vector3(
				1.0f,
				1.0f,
				1.0f
			);

		jumpPad.jumpPadSetting.jumpVelocity =
			Vector3(
				0.0f,
				1.15f,
				5.8f
			);

		jumpPad.jumpPadSetting.floatPowerRate = 5.0f;

		jumpPad.jumpPadSetting.cooldownFrame = 25;

		jumpPad.jumpPadSetting.rotationY = 0.0f;

		jumpPad.jumpPadSetting.useJumpEffect = true;

		room.objects.push_back(jumpPad);
	}

	//コイン
	{
		StageModuleObjectData coin;

		coin.objectType = StageModuleObjectType::TimeCoin;

		coin.localPosition =
			Vector3(
				25.0f,
				1.55f,
				12.0f
			);

		coin.size =
			Vector3(
				2.0f,
				2.0f,
				2.0f
			);

		coin.timeCoinSetting.texturePath = "asset\\texture\\siro.png";

		coin.timeCoinSetting.addTime = 5.0f;

		coin.timeCoinSetting.hitSize = coin.size;

		room.objects.push_back(coin);
	}

	//柱
	{
		StageModuleObjectData obstacle;

		obstacle.objectType = StageModuleObjectType::Block;

		obstacle.localPosition =
			Vector3(
				31.0f,
				5.5f,
				34.0f
			);

		obstacle.size =
			Vector3(
				2.0f,
				5.0f,
				2.0f
			);

		ApplyBlockPreset(
			obstacle,
			StageBlockPresetType::WallAction
		);

		obstacle.wallKickGroupId = 401;

		room.objects.push_back(obstacle);
	}

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

	// Final block
	{
		StageModuleObjectData block;

		block.objectType = StageModuleObjectType::Block;

		block.localPosition =
			Vector3(
				16.0f,
				6.5f,
				30.0f
			);

		block.size =
			Vector3(
				1.0f,
				6.0f,
				1.0f
			);

		ApplyBlockPreset(
			block,
			StageBlockPresetType::NoAction
		);

		block.color =
			XMFLOAT4(
				0.0f,
				0.85f,
				1.0f,
				1.0f
			);

		block.emissivePower = 1.3f;

		room.objects.push_back(block);
	}

	// Final block
	{
		StageModuleObjectData block;

		block.objectType = StageModuleObjectType::Block;

		block.localPosition =
			Vector3(
				34.0f,
				6.5f,
				30.0f
			);

		block.size =
			Vector3(
				1.0f,
				6.0f,
				1.0f
			);

		ApplyBlockPreset(
			block,
			StageBlockPresetType::NoAction
		);

		block.color =
			XMFLOAT4(
				0.0f,
				0.85f,
				1.0f,
				1.0f
			);

		block.emissivePower = 1.3f;

		room.objects.push_back(block);
	}

	// Final JumpPad
	{
		StageModuleObjectData jumpPad;

		jumpPad.objectType = StageModuleObjectType::JumpPad;

		jumpPad.localPosition =
			Vector3(
				25.0f,
				1.55f,
				30.0f
			);

		jumpPad.size =
			Vector3(
				1.0f,
				1.0f,
				1.0f
			);

		jumpPad.jumpPadSetting.jumpVelocity =
			Vector3(
				0.0f,
				0.0f,
				0.0f
			);

		jumpPad.jumpPadSetting.floatPowerRate = 0.0f;

		jumpPad.jumpPadSetting.cooldownFrame = 9999;

		jumpPad.jumpPadSetting.rotationY = 0.0f;

		jumpPad.jumpPadSetting.useJumpEffect = true;

		jumpPad.jumpPadSetting.useGoalLaunch = true;

		jumpPad.jumpPadSetting.goalTargetPosition =
			Vector3(
				25.0f,
				180.0f,
				75.0f
			);

		jumpPad.jumpPadSetting.goalLaunchFrameMax = 120;

		jumpPad.jumpPadSetting.goalLaunchHeight = 60.0f;

		room.objects.push_back(jumpPad);
	}

	return room;
}
