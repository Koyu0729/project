#pragma once

#include <list>
#include <vector>

#include "gameObject.h"

enum class SceneLayer
{
	Camera = 0,
	DepthSorted = 1,
	Foreground = 2,
	SceneObjects = 3,
	PreviewPlayer = 5,
	Particles = 7,
	Hud = 8,
	Overlay = 9,
	Count = 10,
};

class Scene
{
private:
	static constexpr int LAYER_COUNT = static_cast<int>(SceneLayer::Count);
	std::list<GameObject*> m_GameObject[LAYER_COUNT];

private:
	int ClampLayer(int layer) const
	{
		if (layer < 0)
		{
			return 0;
		}

		if (layer >= LAYER_COUNT)
		{
			return LAYER_COUNT - 1;
		}

		return layer;
	}

public:
	virtual ~Scene() {}

	virtual void DataLoad() {}

	virtual void Init();
	virtual void Uninit();
	virtual void Update();
	virtual void Draw();

	template<typename T>
	T* AddGameObject(SceneLayer layer)
	{
		const int layerIndex = ClampLayer(static_cast<int>(layer));

		T* gameObject = new T();

		m_GameObject[layerIndex].push_back(
			gameObject
		);

		gameObject->Init();

		return gameObject;
	}

	template <typename T>
	T* GetGameObject()
	{
		for (int i = 0; i < LAYER_COUNT; i++)
		{
			for (GameObject* gameObject : m_GameObject[i])
			{
				if (gameObject == nullptr)
				{
					continue;
				}

				if (gameObject->Destroy())
				{
					continue;
				}

				T* find = dynamic_cast<T*>(gameObject);

				if (find != nullptr)
				{
					return find;
				}
			}
		}

		return nullptr;
	}

	template <typename T>
	std::vector<T*> GetGameObjects()
	{
		std::vector<T*> finds;

		for (int i = 0; i < LAYER_COUNT; i++)
		{
			for (GameObject* gameObject : m_GameObject[i])
			{
				if (gameObject == nullptr)
				{
					continue;
				}

				if (gameObject->Destroy())
				{
					continue;
				}

				T* find = dynamic_cast<T*>(gameObject);

				if (find != nullptr)
				{
					finds.push_back(
						find
					);
				}
			}
		}

		return finds;
	}
};