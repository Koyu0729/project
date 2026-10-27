#include "main.h"
#include "manager.h"
#include "renderer.h"
#include "result.h"
#include "polygon3D.h"
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
	Camera* camera = AddGameObject<Camera>(0);

	AddGameObject<Sky>(1);

	AddGameObject<Player>(5);

	m_ResultButton = AddGameObject<UI3DButton>(2);

	if (m_ResultButton != nullptr)
	{
		m_ResultButton->SetTexture(
			"asset\\texture\\pressAK.png"
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

		m_ResultButton->SetHitDistance(
			100.0f
		);
	}

	ResultTime* time = AddGameObject<ResultTime>(2);

	AddGameObject<OptionUI>(9);
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
		Manager::SetNextScene(
			new Title()
		);

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