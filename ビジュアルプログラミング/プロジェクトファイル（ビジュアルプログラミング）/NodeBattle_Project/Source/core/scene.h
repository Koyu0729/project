// シーン内のオブジェクトを管理し、初期化・更新・描画を行う
#pragma once

#include <list>
#include <vector>
#include "gameObject3D.h"
#include "gameObject2D.h"

#define LAYER_MAX (10)

class Scene
{
  private:
	// レイヤーごとに3Dオブジェクトを保持する
	std::list<GameObject3D *> m_GameObject3D[LAYER_MAX];

	// レイヤーごとに2Dオブジェクトを保持する

	std::vector<GameObject2D *> m_GameObject2D[LAYER_MAX];

	// 共通リソースの画像をCPU側で展開する
	static void PrepareSceneTextures(const std::string &directory);

  public:
	virtual ~Scene() = default;

	// CPU側のデータを準備する
	virtual bool InitCPU();

	// 描画に必要なデータを準備する
	virtual bool InitGPU();

	// 初期化処理を行う
	virtual bool Init();

	// 終了処理を行う
	virtual void Uninit();

	// 毎フレームの処理
	virtual void Update();

	// 画面に描く処理
	virtual void Draw();

	GameObject2D *GetTopHitObject();

	// 指定した型の3Dオブジェクトを生成し、レイヤーに追加する
	template <typename T>
	T *AddGameObject3D(int layer)
	{
		T *gameObject = new T();

		gameObject->Init();

		m_GameObject3D[layer].push_back(gameObject);

		return gameObject;
	}

	template <typename T>
	T *AddGameObject2D(int layer)
	{
		T *gameObject = new T();

		gameObject->Init();

		m_GameObject2D[layer].push_back(gameObject);

		return gameObject;
	}

	template <typename T>

	T *GetGameObject()
	{
		for (int i = 0; i < LAYER_MAX; i++)
		{
			for (auto gameObject : m_GameObject3D[i])
			{
				T *find = dynamic_cast<T *>(gameObject);

				if (find != nullptr)
				{
					return find;
				}
			}

			for (auto gameObject : m_GameObject2D[i])
			{
				T *find = dynamic_cast<T *>(gameObject);

				if (find != nullptr)
				{
					return find;
				}
			}
		}

		return nullptr;
	}

	template <typename T>

	T *GetGameObject(const std::wstring &name)
	{
		for (int i = 0; i < LAYER_MAX; i++)
		{
			for (auto gameObject : m_GameObject3D[i])
			{
				T *find = dynamic_cast<T *>(gameObject);

				if (find != nullptr)
				{
					if (find->GetName() == name)
					{
						return find;
					}
				}
			}

			for (auto gameObject : m_GameObject2D[i])
			{
				T *find = dynamic_cast<T *>(gameObject);

				if (find != nullptr)
				{
					if (find->GetName() == name)
					{
						return find;
					}
				}
			}
		}

		return nullptr;
	}

	// 指定した型のオブジェクトを一覧で取得する

	template <typename T>

	std::vector<T *> GetGameObjects() 
	{
		std::vector<T *> finds;

		for (int i = 0; i < LAYER_MAX; i++)
		{
			for (auto gameObject : m_GameObject3D[i])
			{

				T *find = dynamic_cast<T *>(gameObject); // gameObjectをTに型変換　型変換できない場合はfindにnullが入る

				if (find != nullptr)
				{
					finds.push_back(find);
				}
			}

			for (auto gameObject : m_GameObject2D[i])
			{

				T *find = dynamic_cast<T *>(gameObject); // gameObjectをTに型変換　型変換できない場合はfindにnullが入る

				if (find != nullptr)
				{
					finds.push_back(find);
				}
			}
		}

		return finds;
	}
};
