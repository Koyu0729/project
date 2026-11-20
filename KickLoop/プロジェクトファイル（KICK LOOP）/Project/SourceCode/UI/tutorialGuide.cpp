#include "main.h"
#include "tutorialGuide.h"

#include "player.h"
#include "polygon2D.h"
#include "scene.h"
#include "tutorialRoom.h"
#include "UITextureText.h"

namespace
{
	// チュートリアルの案内画像1行分のサイズ（ピクセル）
	constexpr float AtlasWidth = 960.0f;     // X（横幅）
	constexpr float AtlasRowHeight = 88.0f;  // Y（高さ）

	constexpr int WallKickReadyRow = 8;
	constexpr int RoomClearRow = 9;
	constexpr int AllClearRow = 10;

	constexpr UIRect GuideRect{160.0f, 614.0f, AtlasWidth, AtlasRowHeight};
	constexpr UIRect StatusRect{400.0f, 554.0f, 480.0f, 44.0f};
	const char* LessonTexture = "asset\\texture\\tutorialLessons.png";

	void SetAtlasRow(UITextureText& label, const UIRect& rect, int row)
	{
		float top = AtlasRowHeight * (float)row;
		label.SetAtlasRect(rect, XMFLOAT4(0.0f, top, AtlasWidth, top + AtlasRowHeight),
			XMFLOAT2(AtlasWidth, AtlasRowHeight * (float)(AllClearRow + 1)));
	}
}

void TutorialGuide::Init(Scene& scene, int roomIndex)
{
	Polygon2D* controls = scene.AddGameObject<Polygon2D>(SceneLayer::Hud);
	controls->SetTexture("asset\\texture\\tutorialControls.png");
	controls->SetRect(24.0f, 24.0f, 486.0f, 46.0f);

	UITextureText* guide = scene.AddGameObject<UITextureText>(SceneLayer::Hud);
	guide->SetTexture(LessonTexture);
	SetAtlasRow(*guide, GuideRect, roomIndex);

	m_Status = scene.AddGameObject<UITextureText>(SceneLayer::Hud);
	m_Status->SetTexture(LessonTexture);
	m_Status->SetVisible(false);
}

void TutorialGuide::Update(const TutorialRoom& room, Player& player, bool isCleared)
{
	int row = -1;

	if (isCleared)
	{
		if (room.IsFinal())
		{
			row = AllClearRow;
		}
		else
		{
			row = RoomClearRow;
		}
	}
	else if ((room.GetType() == StageRoomType::WallKickWaveRoom || room.GetType() == StageRoomType::WallKickRoom) && player.CanWallKickOnNextUpdate())
	{
		row = WallKickReadyRow;
	}

	if (row != m_StatusRow)
	{
		m_StatusRow = row;
		m_Status->SetVisible(row >= 0);

		if (row >= 0)
		{
			SetAtlasRow(*m_Status, StatusRect, row);
		}
	}
}
