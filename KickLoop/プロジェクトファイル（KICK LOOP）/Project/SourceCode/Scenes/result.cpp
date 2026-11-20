#include "main.h"
#include "manager.h"
#include "renderer.h"
#include "result.h"
#include "polygon2D.h"
#include "input.h"
#include "title.h"

#include "camera.h"
#include "sky.h"
#include "player.h"

#include "UI3DButton.h"

#include "resultTime.h"
#include "GameData.h"

//ƒIƒvƒVƒ‡ƒ“UI
#include "optionUI.h"

void Result::Init()
{
	Camera* camera = AddGameObject<Camera>(SceneLayer::Camera);

	AddGameObject<Sky>(SceneLayer::DepthSorted);

	Player* player = AddGameObject<Player>(SceneLayer::PreviewPlayer);
	if (camera != nullptr)
	{
		camera->SetSceneReferences(player);
	}

	m_ResultButton = AddGameObject<UI3DButton>(SceneLayer::Foreground);

	if (m_ResultButton != nullptr)
	{
		m_ResultButton->SetTexture(
			"asset\\texture\\returnToTitleButton.png"
		);

		m_ResultButton->SetPosition(
			Vector3(
				25.0f,
				0.0f,
				30.0f
			)
		);

		m_ResultButton->SetSize(
			5.0f,
			2.0f
		);

		m_ResultButton->SetHitDistance(100.0f);
		m_ResultButton->SetHoverApproach(true);
	}

	AddGameObject<ResultTime>(SceneLayer::Foreground);

	Polygon2D* resultGuide = AddGameObject<Polygon2D>(SceneLayer::Hud);
	if (resultGuide != nullptr)
	{
		const float width = 660.0f;
		const float height = 64.0f;
		const float bottomMargin = 28.0f;

		resultGuide->SetTexture("asset\\texture\\resultGuide.png");
		resultGuide->SetRect((SCREEN_WIDTH - width) * 0.5f, SCREEN_HEIGHT - height - bottomMargin, width, height);
	}

	AddGameObject<OptionUI>(SceneLayer::Overlay);
}

void Result::Update()
{
	Scene::Update();

	if (m_ResultButton == nullptr)
	{
		return;
	}

	if (m_ResultButton->IsPressed())
	{
		Manager::SetScene<Title>(true);

		return;
	}
}

void Result::Uninit()
{
	Scene::Uninit();
}

void Result::Draw()
{
	Scene::Draw();
}