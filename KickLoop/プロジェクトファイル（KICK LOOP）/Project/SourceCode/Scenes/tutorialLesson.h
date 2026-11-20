#pragma once

class Player;
class TutorialRoom;

class TutorialLesson
{
private:
	int m_WallKickCount = 0;
	int m_WallSlideFrames = 0;

	bool m_StartedClimb = false;
	bool m_UsedJumpPad = false;

public:
	// プレイヤーの行動をもとに、今の部屋のクリア条件を満たしたか判定する
	bool CheckClear(const TutorialRoom& room, Player& player);
};
