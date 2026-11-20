#pragma once

#include "scene.h"
#include "tutorialRoom.h"
#include "tutorialLesson.h"
#include "tutorialGuide.h"

class Player;

class Tutorial : public Scene
{
private:
	const int m_RoomIndex;
	Player* m_Player = nullptr;

	TutorialRoom m_Room;
	TutorialLesson m_Lesson;
	TutorialGuide m_Guide;

	bool m_IsCleared = false;

	void InitEffects();

public:
	explicit Tutorial(int roomIndex = 0);

	void Init() override;
	void Update() override;
};
