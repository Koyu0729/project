#include "main.h"
#include "manager.h"
#include "title.h"
#include "loadingScene.h"

#include "camera.h"
#include "sky.h"
#include "player.h"

#include "polygon3D.h"
#include "UI3DButton.h"

//ƒIƒvƒVƒ‡ƒ“UI
#include "optionUI.h"

void Title::Init()
{
	AddGameObject<Camera>(0);

	AddGameObject<Sky>(1);

	AddGameObject<Player>(5);
	m_StartButton = AddGameObject<UI3DButton>(2);

	if (m_StartButton != nullptr)
	{
		m_StartButton->SetTexture(
			"asset\\texture\\pressAK.png"
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
	}

	Polygon3D* titleName =
		AddGameObject<Polygon3D>(2);

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

	AddGameObject<OptionUI>(9);
}

void Title::Update()
{
	Scene::Update();

	if (m_StartButton == nullptr)
	{
		return;
	}

	if (m_StartButton->IsPressed())
	{
		Manager::SetNextScene(
			new LoadingScene()
		);

		return;
	}
}

void Title::Uninit()
{
	Scene::Uninit();

	m_StartButton = nullptr;
}

void Title::Draw()
{
	Scene::Draw();
}