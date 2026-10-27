#pragma once

#include <list>
#include <vector>

#include "gameObject.h"

#define LAYER_MAX (10)

class Scene
{
private:
	std::list<GameObject*> m_GameObject[LAYER_MAX];

private:
	int ClampLayer(int layer) const
	{
		if (layer < 0)
		{
			return 0;
		}

		if (layer >= LAYER_MAX)
		{
			return LAYER_MAX - 1;
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
	T* AddGameObject(int layer)
	{
		layer =
			ClampLayer(layer);

		T* gameObject =
			new T();

		m_GameObject[layer].push_back(
			gameObject
		);

		gameObject->Init();

		return gameObject;
	}

	template <typename T>
	T* GetGameObject()
	{
		for (int i = 0; i < LAYER_MAX; i++)
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

				T* find =
					dynamic_cast<T*>(gameObject);

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

		for (int i = 0; i < LAYER_MAX; i++)
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

				T* find =
					dynamic_cast<T*>(gameObject);

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