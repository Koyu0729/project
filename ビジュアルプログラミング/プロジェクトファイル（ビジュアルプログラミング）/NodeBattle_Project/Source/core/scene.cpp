#include "main.h"
#include "manager.h"
#include "renderer.h"
#include "scene.h"
#include "input.h"
#include "camera.h"
#include "bullet.h"
#include "explosion.h"
#include <algorithm>
#include <vector>
#include "texture.h"
#include "modelRenderer.h"
// 前半：別スレッドで画像とOBJを読み、メモリに保管する。まだ描画用バッファは作らない
bool Scene::InitCPU()
{
	// 画像フォルダー内の画像をCPU側で展開して一時保存する
	PrepareSceneTextures("asset\\texture");

	return ModelRenderer::PrepareCPU("asset\\model\\box.obj");
}

// 後半：メイン側で保管済みのデータを取り出し、DirectXの画像とバッファを作る
// InitGPUという名前でも、この関数自体はCPU上で実行してGPU用の準備を指示する
bool Scene::InitGPU()
{
	// 展開済みの画像からGPU画像を作る。メイン側で実行する
	Texture::UploadPrepared();
	return ModelRenderer::Preload("asset\\model\\box.obj");
}

// 初期化処理を行う
bool Scene::Init()
{

	return true;
}

// 終了処理を行う
void Scene::Uninit()
{
	for (int i = 0; i < LAYER_MAX; ++i)
	{
		for (auto *object : m_GameObject3D[i])
		{
			if (object)
			{
				object->Uninit();
			}
		}

		for (auto *object : m_GameObject2D[i])
		{
			if (object)
			{
				object->Uninit();
			}
		}
	}

	for (int i = 0; i < LAYER_MAX; ++i)
	{
		for (auto *object : m_GameObject3D[i])
		{
			delete object;
		}

		for (auto *object : m_GameObject2D[i])
		{
			delete object;
		}

		m_GameObject3D[i].clear();

		m_GameObject2D[i].clear();
	}
}

// 毎フレームの処理
void Scene::Update()
{

	if (Mouse::LeftClick() || Mouse::LeftHold() || Mouse::LeftRelease() ||
		Mouse::RightClick() || Mouse::RightHold() || Mouse::RightRelease() ||
		Mouse::LeftDoubleClick())
	{
		GameObject2D *hitObj = Manager::GetScene()->GetTopHitObject();

		for (int i = 0; i < LAYER_MAX; i++)
		{
			for (auto *obj : m_GameObject2D[i])
			{
				if (!obj)
				{
					continue;
				}

				if (!obj->GetHitBox())
				{
					continue;
				}

				obj->GetHitBox()->SetSelect(false);
			}
		}

		if (hitObj)
		{
			hitObj->GetHitBox()->SetSelect(true);
		}
	}

	for (int i = 0; i < LAYER_MAX; i++)
	{
		for (auto *obj : m_GameObject3D[i])
		{
			if (!obj)
			{
				continue;
			}

			if (obj->Destroy())
			{
				continue;
			}

			obj->Update();
		}

		for (auto *obj : m_GameObject2D[i])
		{
			if (!obj)
			{
				continue;
			}

			if (obj->Destroy())
			{
				continue;
			}

			obj->Update();
		}
	}

	// ゲームオブジェクトのデリートをする

	for (int i = 0; i < LAYER_MAX; i++)
	{
		m_GameObject3D[i].remove_if([](GameObject3D *object)
									{
										// リソース開放

										if (!object)
										{
											return true;
										}

										if (object->Destroy())
										{
											object->Uninit();

											delete object;

											return true;
										}

										return false; });

		// レイヤーのゲームオブジェクトを取得
		auto &layer = m_GameObject2D[i];

		for (int idx = (int)layer.size() - 1; idx >= 0; --idx)
		{
			GameObject2D *object = layer[idx];

			if (!object)
			{
				layer.erase(layer.begin() + idx);

				continue;
			}

			// Destroy が true のときだけ消す

			if (object->Destroy())
			{
				object->Uninit();

				delete object;

				layer.erase(layer.begin() + idx);
			}
		}
	}
}

// 画面に描く処理
void Scene::Draw()
{
	// 透明な弾・着弾の光は、床・壁・プレイヤーを描いた後に重ねる
	std::vector<GameObject3D *> billboards;

	// Zソート
	Camera *camera = GetGameObject<Camera>();

	if (camera != nullptr)
	{
		Vector3 cameraPosition = camera->GetPosition();

		Vector3 cameraForward = camera->GetForward();

		m_GameObject3D[1].sort([&](GameObject3D *object1, GameObject3D *object2)
							   { return object1->GetZ(cameraPosition, cameraForward) > object2->GetZ(cameraPosition, cameraForward); });
	}

	for (int i = 0; i < LAYER_MAX; i++)
	{
		for (auto gameObject : m_GameObject3D[i])
		{
			if (!gameObject || gameObject->Destroy())
			{
				continue;
			}

			if (!gameObject || gameObject->Destroy())
			{
				continue;
			}

			if (dynamic_cast<Bullet *>(gameObject) || dynamic_cast<Explosion *>(gameObject))
			{
				billboards.push_back(gameObject);

				continue;
			}

			gameObject->Draw();
		}
	}

	if (camera)
	{
		const Vector3 origin = camera->GetPosition();

		std::stable_sort(billboards.begin(), billboards.end(), [&](GameObject3D *a, GameObject3D *b)
						 { return (a->GetPosition() - origin).length() > (b->GetPosition() - origin).length(); });
	}

	for (auto *billboard : billboards)
	{
		billboard->Draw();
	}

	for (int i = 0; i < LAYER_MAX; i++)
	{

		for (auto gameObject : m_GameObject2D[i])
		{
			if (!gameObject || gameObject->Destroy())
			{
				continue;
			}

			if (!gameObject->GetVisible())
			{
				continue;
			}

			gameObject->Draw();
		}
	}
}

// マウスが重なるUIを、描画順の手前から探す
GameObject2D *Scene::GetTopHitObject()
{
	Vector2 mouse = Mouse::Pos();

	for (int i = LAYER_MAX - 1; i >= 0; i--)
	{
		// レイヤーの中でも格納されている中で逆から見る

		for (int idx = (int)m_GameObject2D[i].size() - 1; idx >= 0; --idx)
		{
			// レイヤー(i)の後ろ(idx)から探す
			auto obj = m_GameObject2D[i][idx];

			if (!obj)
			{
				continue;
			}

			auto *hit = obj->GetHitBox();

			if (!hit)
			{
				continue;
			}

			if (!hit->GetEnable())
			{
				continue;
			}

			if (hit->Overlap())
			{
				if (hit->GetBlockSelect())
				{
					continue;
				}

				return obj;
			}
		}
	}

	return nullptr;
}

// 画像フォルダーをたどり、画像の展開だけを行う
void Scene::PrepareSceneTextures(const std::string &directory)
{
	WIN32_FIND_DATAA entry{};
	const HANDLE search = FindFirstFileA((directory + "\\*").c_str(), &entry);

	if (search == INVALID_HANDLE_VALUE)
	{
		return;
	}

	do
	{
		const std::string name = entry.cFileName;

		if (name == "." || name == "..")
		{
			continue;
		}

		const std::string path = directory + "\\" + name;

		if (entry.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			PrepareSceneTextures(path);
		}
		else if (name.size() >= 4)
		{
			const std::string extension = name.substr(name.size() - 4);

			if (extension == ".png" || extension == ".jpg")
			{
				Texture::PrepareCPU(path.c_str());
			}
		}
	} while (FindNextFileA(search, &entry));

	FindClose(search);
}
