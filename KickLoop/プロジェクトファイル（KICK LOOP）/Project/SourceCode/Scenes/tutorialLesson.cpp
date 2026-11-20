#include "tutorialLesson.h"

#include "tutorialRoom.h"
#include "player.h"

bool TutorialLesson::CheckClear(const TutorialRoom& room, Player& player)
{
	constexpr int WallSlideClearFrames = 6;  // 壁滑りのクリアに必要な連続フレーム数

	switch (room.GetType())
	{
	case StageRoomType::JumpOnlyRoom:
		return room.HasLandedOnLastBlock(player);

	case StageRoomType::ZigzagRoom:
		return room.IsAtExit(player);

	case StageRoomType::WallKickWaveRoom:
	case StageRoomType::WallKickRoom:
		if (player.IsWallKick())
		{
			++m_WallKickCount;
		}

		if (room.GetType() == StageRoomType::WallKickRoom)
		{
			return m_WallKickCount >= 2;
		}

		return m_WallKickCount >= 1;

	case StageRoomType::WallSlideRoom:
		if (player.IsWallSliding())
		{
			++m_WallSlideFrames;
		}
		else
		{
			m_WallSlideFrames = 0;
		}

		return m_WallSlideFrames >= WallSlideClearFrames;

	case StageRoomType::LaserWallSlideRoom:
		if (player.IsLedgeClimbing())
		{
			m_StartedClimb = true;
			return false;
		}

		return m_StartedClimb;

	case StageRoomType::JumpPadConnectRoom:
		m_UsedJumpPad = m_UsedJumpPad || room.IsJumpPadOnCooldown();
		return m_UsedJumpPad && room.IsAtExit(player);

	case StageRoomType::FinalJumpPadRoom:
		return player.IsGoalReached();

	default:
		return false;
	}
}
