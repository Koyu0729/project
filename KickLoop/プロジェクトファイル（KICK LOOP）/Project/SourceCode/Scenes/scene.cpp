#include "main.h"
#include "manager.h"
#include "renderer.h"
#include "scene.h"
#include "input.h"
#include "camera.h"

#include "optionUI.h"

void Scene::Init()
{
	DataLoad();
}

void Scene::Uninit()
{
	// 終了処理で相互に参照するため、全員の Uninit が終わるまで削除しない。
	for (int i = 0; i < LAYER_COUNT; i++)
	{
		for (auto gameObject : m_GameObject[i])
		{
			if (gameObject == nullptr)
			{
				continue;
			}

			gameObject->Uninit();
		}
	}

	for (int i = 0; i < LAYER_COUNT; i++)
	{
		for (auto gameObject : m_GameObject[i])
		{
			delete gameObject;
		}

		m_GameObject[i].clear();
	}
}

void Scene::Update()
{
	OptionUI* optionUI = GetGameObject<OptionUI>();

	bool isOptionOpen = false;

	if (optionUI != nullptr)
	{
		isOptionOpen = optionUI->IsOpen();
	}

	for (int i = 0; i < LAYER_COUNT; i++)
	{
		for (auto gameObject : m_GameObject[i])
		{
			if (gameObject == nullptr)
			{
				continue;
			}

			if (!gameObject->Active())
			{
				continue;
			}

			if (isOptionOpen)
			{
				OptionUI* currentOptionUI = dynamic_cast<OptionUI*>(gameObject);

				if (currentOptionUI == nullptr)
				{
					continue;
				}
			}

			gameObject->Update();
		}
	}

	for (int i = 0; i < LAYER_COUNT; i++)
	{
		auto list = m_GameObject[i].begin();

		while (list != m_GameObject[i].end())
		{
			GameObject* gameObject = *list;

			if (gameObject == nullptr)
			{
				list = m_GameObject[i].erase(list);

				continue;
			}

			if (!gameObject->Destroy())
			{
				++list;

				continue;
			}

			gameObject->Uninit();

			delete gameObject;

			list = m_GameObject[i].erase(list);
		}
	}
}

void Scene::Draw()
{
	// 奥から描くレイヤーを並べ替える。更新時も同じ順序を使う。
	Camera* camera = GetGameObject<Camera>();

	if (camera != nullptr)
	{
		Vector3 cameraPosition = camera->GetPosition();
		Vector3 cameraForward = camera->GetForward();

		m_GameObject[static_cast<int>(SceneLayer::DepthSorted)].sort([&](GameObject* object1, GameObject* object2)
			{
				return object1->GetZ(cameraPosition, cameraForward) > object2->GetZ(cameraPosition, cameraForward);
			});
	}

	for (int i = 0; i < LAYER_COUNT; i++)
	{
		for (auto gameObject : m_GameObject[i])
		{
			if (gameObject == nullptr)
			{
				continue;
			}

			if (!gameObject->Active())
			{
				continue;
			}

			if (!gameObject->Visible())
			{
				continue;
			}

			gameObject->Draw();
		}
	}
}

