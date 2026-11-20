#pragma once

class Player;
class Scene;
class TutorialRoom;
class UITextureText;

class TutorialGuide
{
private:
	UITextureText* m_Status = nullptr;
	int m_StatusRow = -1;  // 表示中の案内画像の行番号（-1で非表示）

public:
	void Init(Scene& scene, int roomIndex);
	// 壁キックの受付状態と部屋のクリア状況に合わせて案内表示を切り替える
	void Update(const TutorialRoom& room, Player& player, bool isCleared);
};
