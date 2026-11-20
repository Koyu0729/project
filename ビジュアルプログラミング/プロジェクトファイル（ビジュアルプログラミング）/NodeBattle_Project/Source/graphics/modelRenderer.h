#pragma once

// マテリアル構造体
struct MODEL_MATERIAL
{
	char Name[256];

	MATERIAL Material;

	char TextureName[256];

	ID3D11ShaderResourceView *Texture;
};

// 描画サブセット構造体
struct SUBSET
{
	unsigned int StartIndex;

	unsigned int IndexNum;

	MODEL_MATERIAL Material;
};

// モデル構造体
struct MODEL_OBJ
{
	VERTEX_3D *VertexArray = nullptr;

	unsigned int VertexNum;

	unsigned int *IndexArray = nullptr;

	unsigned int IndexNum;

	SUBSET *SubsetArray = nullptr;

	unsigned int SubsetNum = 0;

	// CPUロード失敗・遷移キャンセルでも、確保済みの配列を破棄する
	~MODEL_OBJ()
	{
		delete[] VertexArray;
		delete[] IndexArray;
		delete[] SubsetArray;
	}
};

struct MODEL
{
	ID3D11Buffer *VertexBuffer = nullptr;

	ID3D11Buffer *IndexBuffer = nullptr;

	SUBSET *SubsetArray = nullptr;

	unsigned int SubsetNum = 0;

	// モデルが所有するGPUバッファだけを解放する。画像はTextureが所有する
	~MODEL()
	{
		if (VertexBuffer)
		{
			VertexBuffer->Release();
		}

		if (IndexBuffer)
		{
			IndexBuffer->Release();
		}

		delete[] SubsetArray;
	}
};

#include <string>
#include <mutex>
#include <unordered_map>
#include "vector3.h"

class ModelRenderer
{
  private:
	static std::unordered_map<std::string, MODEL *> m_ModelPool;

	// GPU作成まで保持するモデルの頂点・面・マテリアル
	static std::unordered_map<std::string, MODEL_OBJ *> m_CPUModels;
	static std::mutex m_ModelMutex;

	static bool LoadModel(const char *FileName, MODEL *Model);

	static bool LoadObj(const char *FileName, MODEL_OBJ *ModelObj);

	static bool LoadMaterial(const char *FileName, MODEL_MATERIAL **MaterialArray, unsigned int *MaterialNum);

	MODEL *m_Model{};

  public:
	// 初期化・読み込みは成功ならtrue、失敗ならfalseを返す
	static bool PrepareCPU(const char *fileName);

	static bool Preload(const char *FileName);

	bool Load(const char *FileName);

	static void UnloadAll();

	// 画面に描く処理
	void Draw();

	// 本体の色を変えずに、武器などを指定した色で描く
	void DrawTinted(const XMFLOAT4 &tint);

	static std::vector<Vector3> LoadLineV(const char *FileName);

	ModelRenderer() = default;
};
