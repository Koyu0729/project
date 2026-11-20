#include "main.h"
#include "manager.h"
#include "title.h"
#include "loadingScene.h"
#include "tutorial.h"

#include "camera.h"
#include "sky.h"
#include "player.h"

#include "polygon2D.h"
#include "polygon3D.h"
#include "UI3DButton.h"

//ƒIƒvƒVƒ‡ƒ“UI
#include "optionUI.h"

void Title::Init()
{
	Camera* camera = AddGameObject<Camera>(SceneLayer::Camera);

	AddGameObject<Sky>(SceneLayer::DepthSorted);

	Player* player = AddGameObject<Player>(SceneLayer::PreviewPlayer);
	if (camera != nullptr)
	{
		camera->SetSceneReferences(player);
	}
	m_StartButton = AddGameObject<UI3DButton>(SceneLayer::Foreground);

	if (m_StartButton != nullptr)
	{
		m_StartButton->SetTexture(
			"asset\\texture\\startButton.png"
		);

		m_StartButton->SetPosition(
			Vector3(
				25.0f,
				0.0f,
				30.0f
			)
		);

		m_StartButton->SetSize(
			5.0f,
			2.0f
		);

		m_StartButton->SetHitDistance(
			100.0f
		);
		m_StartButton->SetHoverApproach(true);
	}

	m_TutorialButton = AddGameObject<UI3DButton>(SceneLayer::Foreground);
	if (m_TutorialButton != nullptr)
	{
		m_TutorialButton->SetTexture("asset\\texture\\tutorialButton.png");
		m_TutorialButton->SetRect3D(25.0f, -2.3f, 30.0f, 5.0f, 1.8f);
		m_TutorialButton->SetHitDistance(100.0f);
		m_TutorialButton->SetHoverApproach(true);
	}

	Polygon3D* titleName = AddGameObject<Polygon3D>(SceneLayer::Foreground);

	if (titleName != nullptr)
	{
		titleName->SetTexture(
			"asset\\texture\\titleName.png"
		);

		titleName->SetRect3D(
			25.0f,
			5.0f,
			30.0f,
			5.0f,
			2.0f
		);

		titleName->SetColor(
			1.0f,
			1.0f,
			1.0f,
			1.0f
		);

		titleName->SetRotation(
			0.0f,
			0.0f,
			0.0f
		);

		titleName->SetFloating(
			0.3f,
			3.0f
		);
	}

	Polygon2D* startGuide = AddGameObject<Polygon2D>(SceneLayer::Hud);
	if (startGuide != nullptr)
	{
		const float width = 660.0f;
		const float height = 64.0f;
		const float bottomMargin = 28.0f;

		startGuide->SetTexture("asset\\texture\\startGuide.png");
		startGuide->SetRect(
			(SCREEN_WIDTH - width) * 0.5f,
			SCREEN_HEIGHT - height - bottomMargin,
			width,
			height
		);
	}

	AddGameObject<OptionUI>(SceneLayer::Overlay);
}

void Title::Update()
{
	Scene::Update();

	if (m_StartButton != nullptr && m_StartButton->IsPressed())
	{
		Manager::SetScene<LoadingScene>(true);
		return;
	}

	if (m_TutorialButton != nullptr && m_TutorialButton->IsPressed())
	{
		Manager::SetScene<Tutorial>(true);
	}
}

void Title::Uninit()
{
	Scene::Uninit();

	m_StartButton = nullptr;
	m_TutorialButton = nullptr;
}

void Title::Draw()
{
	Scene::Draw();
}