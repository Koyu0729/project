#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <shlwapi.h>
#pragma comment(lib, "shlwapi.lib")

#include "main.h"
#include "renderer.h"
#include "modelRenderer.h"
#include "texture.h"
#include <memory>

// 描画できる状態のモデルを共有する一覧
std::unordered_map<std::string, MODEL *> ModelRenderer::m_ModelPool;

// ファイルから読んだ頂点・面を一時保管する一覧。ここからGPUバッファを作る
std::unordered_map<std::string, MODEL_OBJ *> ModelRenderer::m_CPUModels;
std::mutex ModelRenderer::m_ModelMutex;

// 読み込みの前半：OBJを解析して一時保管する。別スレッドから呼び出せる
bool ModelRenderer::PrepareCPU(const char *fileName)
{
	if (!fileName || !*fileName)
	{
		return false;
	}

	// 共有モデルの一覧を同時に変更しないようにする
	std::lock_guard<std::mutex> lock(m_ModelMutex);

	if (m_ModelPool.count(fileName) || m_CPUModels.count(fileName))
	{
		return true;
	}

	// OBJの頂点・面・マテリアルをCPU側へ読み込む。GPUバッファはまだ作らない
	auto model = std::make_unique<MODEL_OBJ>();
	if (!LoadObj(fileName, model.get()))
	{
		return false;
	}

	for (unsigned int i = 0; i < model->SubsetNum; ++i)
	{
		Texture::PrepareCPU(model->SubsetArray[i].Material.TextureName);
	}

	// 解析済みデータを保管し、GPU作成時または終了時に解放する
	m_CPUModels.emplace(fileName, model.release());

	return true;
}

// 読み込みの後半：メイン側でGPUバッファを作り、描画用モデルの一覧へ登録する
bool ModelRenderer::Preload(const char *FileName)
{
	if (!FileName || !*FileName)
	{
		return false;
	}

	{
		std::lock_guard<std::mutex> lock(m_ModelMutex);
		if (m_ModelPool.count(FileName) > 0)
		{
			return true;
		}
	}

	auto model = std::make_unique<MODEL>();

	if (!LoadModel(FileName, model.get()))
	{
		return false;
	}

	std::lock_guard<std::mutex> lock(m_ModelMutex);
	if (m_ModelPool.emplace(FileName, model.get()).second)
	{
		model.release();
	}

	return true;
}

// オブジェクトが使うモデルを取得する。事前準備がなければ、その場で読み込みも行う
bool ModelRenderer::Load(const char *FileName)
{
	m_Model = nullptr;
	if (!FileName || !*FileName)
	{
		return false;
	}

	if (!Preload(FileName))
	{
		return false;
	}

	std::lock_guard<std::mutex> lock(m_ModelMutex);
	m_Model = m_ModelPool.at(FileName);
	return true;
}

void ModelRenderer::UnloadAll()
{
	std::lock_guard<std::mutex> lock(m_ModelMutex);
	for (std::pair<const std::string, MODEL *> pair : m_ModelPool)
	{

		delete pair.second;
	}

	m_ModelPool.clear();

	for (const auto &entry : m_CPUModels)
	{
		delete entry.second;
	}

	m_CPUModels.clear();
}

// 画面に描く処理
void ModelRenderer::Draw()
{
	DrawTinted(XMFLOAT4(1, 1, 1, 1));
}

// 本体の色を変えずに、武器などを指定した色で描く
void ModelRenderer::DrawTinted(const XMFLOAT4 &tint)
{
	if (!m_Model)
	{
		return;
	}

	// 頂点バッファ設定
	UINT stride = sizeof(VERTEX_3D);

	UINT offset = 0;

	Renderer::GetDeviceContext()->IASetVertexBuffers(0, 1, &m_Model->VertexBuffer, &stride, &offset);

	// インデックスバッファ設定
	Renderer::GetDeviceContext()->IASetIndexBuffer(m_Model->IndexBuffer, DXGI_FORMAT_R32_UINT, 0);

	// プリミティブトポロジ設定
	Renderer::GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	for (unsigned int i = 0; i < m_Model->SubsetNum; i++)
	{
		// マテリアル設定
		MATERIAL material = m_Model->SubsetArray[i].Material.Material;

		material.Diffuse.x *= tint.x;

		material.Diffuse.y *= tint.y;

		material.Diffuse.z *= tint.z;

		material.Diffuse.w *= tint.w;

		Renderer::SetMaterial(material);

		// テクスチャ設定

		Renderer::GetDeviceContext()->PSSetShaderResources(0, 1, &m_Model->SubsetArray[i].Material.Texture);

		// ポリゴン描画
		Renderer::GetDeviceContext()->DrawIndexed(m_Model->SubsetArray[i].IndexNum, m_Model->SubsetArray[i].StartIndex, 0);
	}
}

bool ModelRenderer::LoadModel(const char *FileName, MODEL *Model)
{

	if (!PrepareCPU(FileName))
	{
		return false;
	}
	std::lock_guard<std::mutex> lock(m_ModelMutex);
	// 保管したCPUデータを取り出し、GPUバッファの作成に使用する
	std::unique_ptr<MODEL_OBJ> prepared(m_CPUModels.at(FileName));
	m_CPUModels.erase(FileName);
	MODEL_OBJ &modelObj = *prepared;

	// 頂点バッファ生成
	{
		D3D11_BUFFER_DESC bd;

		ZeroMemory(&bd, sizeof(bd));

		bd.Usage = D3D11_USAGE_DEFAULT;

		bd.ByteWidth = sizeof(VERTEX_3D) * modelObj.VertexNum;

		bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;

		bd.CPUAccessFlags = 0;

		D3D11_SUBRESOURCE_DATA sd;

		ZeroMemory(&sd, sizeof(sd));

		sd.pSysMem = modelObj.VertexArray;

		if (FAILED(Renderer::GetDevice()->CreateBuffer(&bd, &sd, &Model->VertexBuffer)))
		{
			return false;
		}
	}

	// インデックスバッファ生成
	{
		D3D11_BUFFER_DESC bd;

		ZeroMemory(&bd, sizeof(bd));

		bd.Usage = D3D11_USAGE_DEFAULT;

		bd.ByteWidth = sizeof(unsigned int) * modelObj.IndexNum;

		bd.BindFlags = D3D11_BIND_INDEX_BUFFER;

		bd.CPUAccessFlags = 0;

		D3D11_SUBRESOURCE_DATA sd;

		ZeroMemory(&sd, sizeof(sd));

		sd.pSysMem = modelObj.IndexArray;

		if (FAILED(Renderer::GetDevice()->CreateBuffer(&bd, &sd, &Model->IndexBuffer)))
		{
			return false;
		}
	}

	// サブセット設定
	{
		Model->SubsetArray = new SUBSET[modelObj.SubsetNum];

		Model->SubsetNum = modelObj.SubsetNum;

		for (unsigned int i = 0; i < modelObj.SubsetNum; i++)
		{
			Model->SubsetArray[i].StartIndex = modelObj.SubsetArray[i].StartIndex;

			Model->SubsetArray[i].IndexNum = modelObj.SubsetArray[i].IndexNum;

			Model->SubsetArray[i].Material.Material = modelObj.SubsetArray[i].Material.Material;

			Model->SubsetArray[i].Material.Texture = nullptr;

			// テクスチャ読み込み
			Model->SubsetArray[i].Material.Texture = Texture::Load(modelObj.SubsetArray[i].Material.TextureName);

			if (Model->SubsetArray[i].Material.Texture)
			{
				Model->SubsetArray[i].Material.Material.TextureEnable = true;
			}
			else
			{
				Model->SubsetArray[i].Material.Material.TextureEnable = false;
			}
		}
	}

	// 頂点・面・サブセットはpreparedの破棄時に解放される

	return true;
}

// モデル読込
bool ModelRenderer::LoadObj(const char *FileName, MODEL_OBJ *ModelObj)
{

	char dir[MAX_PATH];

	strcpy(dir, FileName);

	PathRemoveFileSpec(dir);

	std::unique_ptr<XMFLOAT3[]> positionArray;

	std::unique_ptr<XMFLOAT3[]> normalArray;

	std::unique_ptr<XMFLOAT2[]> texcoordArray;

	unsigned int positionNum = 0;

	unsigned int normalNum = 0;

	unsigned int texcoordNum = 0;

	unsigned int vertexNum = 0;

	unsigned int indexNum = 0;

	unsigned int in = 0;

	unsigned int subsetNum = 0;

	MODEL_MATERIAL *materialArray = nullptr;

	std::unique_ptr<MODEL_MATERIAL[]> materialOwner;

	unsigned int materialNum = 0;

	char str[256];

	char *s;

	char c;

	FILE *file;

	file = fopen(FileName, "rt");

	if (!file)
	{
		return false;
	}

	std::unique_ptr<FILE, decltype(&fclose)> fileOwner(file, &fclose);

	// 要素数カウント

	while (true)
	{
		fscanf(file, "%255s", str);

		if (feof(file) != 0)
		{
			break;
		}

		if (strcmp(str, "v") == 0)
		{
			positionNum++;
		}
		else if (strcmp(str, "vn") == 0)
		{
			normalNum++;
		}
		else if (strcmp(str, "vt") == 0)
		{
			texcoordNum++;
		}
		else if (strcmp(str, "usemtl") == 0)
		{
			subsetNum++;
		}
		else if (strcmp(str, "f") == 0)
		{
			in = 0;

			do
			{
				fscanf(file, "%255s", str);

				vertexNum++;

				in++;

				c = fgetc(file);

			} while (c != '\n' && c != '\r');

			// 四角は三角に分割

			if (in == 4)
			{
				in = 6;
			}

			indexNum += in;
		}
	}

	// メモリ確保
	positionArray.reset(new XMFLOAT3[positionNum]);

	normalArray.reset(new XMFLOAT3[normalNum]);

	texcoordArray.reset(new XMFLOAT2[texcoordNum]);

	ModelObj->VertexArray = new VERTEX_3D[vertexNum];

	ModelObj->VertexNum = vertexNum;

	ModelObj->IndexArray = new unsigned int[indexNum];

	ModelObj->IndexNum = indexNum;

	ModelObj->SubsetArray = new SUBSET[subsetNum];

	ModelObj->SubsetNum = subsetNum;

	// 要素読込
	XMFLOAT3 *position = positionArray.get();

	XMFLOAT3 *normal = normalArray.get();

	XMFLOAT2 *texcoord = texcoordArray.get();

	unsigned int vc = 0;

	unsigned int ic = 0;

	unsigned int sc = 0;

	fseek(file, 0, SEEK_SET);

	while (true)
	{
		fscanf(file, "%255s", str);

		if (feof(file) != 0)
		{
			break;
		}

		if (strcmp(str, "mtllib") == 0)
		{
			// マテリアルファイル
			fscanf(file, "%255s", str);

			char path[256];

			strcpy(path, dir);

			strcat(path, "\\");

			strcat(path, str);

			if (!LoadMaterial(path, &materialArray, &materialNum))
			{
				return false;
			}

			materialOwner.reset(materialArray);
		}
		else if (strcmp(str, "o") == 0)
		{
			// オブジェクト名
			fscanf(file, "%255s", str);
		}
		else if (strcmp(str, "v") == 0)
		{
			// 頂点座標
			fscanf(file, "%f", &position->x);

			fscanf(file, "%f", &position->y);

			fscanf(file, "%f", &position->z);

			position++;
		}
		else if (strcmp(str, "vn") == 0)
		{
			// 法線
			fscanf(file, "%f", &normal->x);

			fscanf(file, "%f", &normal->y);

			fscanf(file, "%f", &normal->z);

			normal++;
		}
		else if (strcmp(str, "vt") == 0)
		{
			// テクスチャ座標
			fscanf(file, "%f", &texcoord->x);

			fscanf(file, "%f", &texcoord->y);

			texcoord->x = 1.0f - texcoord->x;

			texcoord->y = 1.0f - texcoord->y;

			texcoord++;
		}
		else if (strcmp(str, "usemtl") == 0)
		{
			// マテリアル
			fscanf(file, "%255s", str);

			if (sc != 0)
			{
				ModelObj->SubsetArray[sc - 1].IndexNum = ic - ModelObj->SubsetArray[sc - 1].StartIndex;
			}

			ModelObj->SubsetArray[sc].StartIndex = ic;

			for (unsigned int i = 0; i < materialNum; i++)
			{
				if (strcmp(str, materialArray[i].Name) == 0)
				{
					ModelObj->SubsetArray[sc].Material.Material = materialArray[i].Material;

					strcpy(ModelObj->SubsetArray[sc].Material.TextureName, materialArray[i].TextureName);

					strcpy(ModelObj->SubsetArray[sc].Material.Name, materialArray[i].Name);

					break;
				}
			}

			sc++;
		}
		else if (strcmp(str, "f") == 0)
		{
			// 面
			in = 0;

			do
			{
				fscanf(file, "%255s", str);

				s = strtok(str, "/");

				ModelObj->VertexArray[vc].Position = positionArray[atoi(s) - 1];

				if (s[strlen(s) + 1] != '/')
				{
					// テクスチャ座標が存在しない場合もある
					s = strtok(nullptr, "/");

					ModelObj->VertexArray[vc].TexCoord = texcoordArray[atoi(s) - 1];
				}

				s = strtok(nullptr, "/");

				ModelObj->VertexArray[vc].Normal = normalArray[atoi(s) - 1];

				ModelObj->VertexArray[vc].Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);

				ModelObj->IndexArray[ic] = vc;

				ic++;

				vc++;

				in++;

				c = fgetc(file);

			} while (c != '\n' && c != '\r');

			// 四角は三角に分割

			if (in == 4)
			{
				ModelObj->IndexArray[ic] = vc - 4;

				ic++;

				ModelObj->IndexArray[ic] = vc - 2;

				ic++;
			}
		}
	}

	if (sc != 0)
	{
		ModelObj->SubsetArray[sc - 1].IndexNum = ic - ModelObj->SubsetArray[sc - 1].StartIndex;
	}

	// fileOwnerがファイルを閉じる

	// materialOwnerがマテリアルの仮配列を解放する

	return true;
}

// マテリアル読み込み
bool ModelRenderer::LoadMaterial(const char *FileName, MODEL_MATERIAL **MaterialArray, unsigned int *MaterialNum)
{

	char dir[MAX_PATH];

	strcpy(dir, FileName);

	PathRemoveFileSpec(dir);

	char str[256];

	FILE *file;

	file = fopen(FileName, "rt");

	if (!file)
	{
		return false;
	}

	MODEL_MATERIAL *materialArray;

	unsigned int materialNum = 0;

	std::unique_ptr<FILE, decltype(&fclose)> fileOwner(file, &fclose);

	// 要素数カウント

	while (true)
	{
		fscanf(file, "%255s", str);

		if (feof(file) != 0)
		{
			break;
		}

		if (strcmp(str, "newmtl") == 0)
		{
			materialNum++;
		}
	}

	// メモリ確保
	auto materialOwner = std::make_unique<MODEL_MATERIAL[]>(materialNum);

	materialArray = materialOwner.get();

	// 要素読込
	int mc = -1;

	fseek(file, 0, SEEK_SET);

	while (true)
	{
		fscanf(file, "%255s", str);

		if (feof(file) != 0)
		{
			break;
		}

		if (strcmp(str, "newmtl") == 0)
		{
			// マテリアル名
			mc++;

			fscanf(file, "%255s", materialArray[mc].Name);

			strcpy(materialArray[mc].TextureName, "");

			materialArray[mc].Material.Emission.x = 0.0f;

			materialArray[mc].Material.Emission.y = 0.0f;

			materialArray[mc].Material.Emission.z = 0.0f;

			materialArray[mc].Material.Emission.w = 0.0f;
		}
		else if (strcmp(str, "Ka") == 0)
		{
			// アンビエント
			fscanf(file, "%f", &materialArray[mc].Material.Ambient.x);

			fscanf(file, "%f", &materialArray[mc].Material.Ambient.y);

			fscanf(file, "%f", &materialArray[mc].Material.Ambient.z);

			materialArray[mc].Material.Ambient.w = 1.0f;
		}
		else if (strcmp(str, "Kd") == 0)
		{
			// ディフューズ
			fscanf(file, "%f", &materialArray[mc].Material.Diffuse.x);

			fscanf(file, "%f", &materialArray[mc].Material.Diffuse.y);

			fscanf(file, "%f", &materialArray[mc].Material.Diffuse.z);

			materialArray[mc].Material.Diffuse.w = 1.0f;
		}
		else if (strcmp(str, "Ks") == 0)
		{
			// スペキュラ
			fscanf(file, "%f", &materialArray[mc].Material.Specular.x);

			fscanf(file, "%f", &materialArray[mc].Material.Specular.y);

			fscanf(file, "%f", &materialArray[mc].Material.Specular.z);

			materialArray[mc].Material.Specular.w = 1.0f;
		}
		else if (strcmp(str, "Ns") == 0)
		{
			// スペキュラ強度
			fscanf(file, "%f", &materialArray[mc].Material.Shininess);
		}
		else if (strcmp(str, "d") == 0)
		{
			// アルファ
			fscanf(file, "%f", &materialArray[mc].Material.Diffuse.w);
		}
		else if (strcmp(str, "map_Kd") == 0)
		{
			// テクスチャ
			fscanf(file, "%255s", str);

			char path[256];

			strcpy(path, dir);

			strcat(path, "\\");

			strcat(path, str);

			strcat(materialArray[mc].TextureName, path);
		}
	}

	// fileOwnerがファイルを閉じる

	*MaterialArray = materialOwner.release();

	*MaterialNum = materialNum;

	return true;
}

// モデルから頂点座標を取得する
std::vector<Vector3> ModelRenderer::LoadLineV(const char *FileName)
{
	std::vector<Vector3> vertex;

	FILE *file = fopen(FileName, "rt");

	if (!file)
	{
		printf("Failed to open %s\n", FileName);

		return vertex;
	}

	char str[256];

	while (true)
	{
		if (fscanf(file, "%s", str) == EOF)
		{
			break;
		}

		// 頂点座標行 "v x y z"

		if (strcmp(str, "v") == 0)
		{
			float x, y, z;

			fscanf(file, "%f %f %f", &x, &y, &z);

			vertex.emplace_back(x, y, z);

			printf("v: (%.3f, %.3f, %.3f)\n", x, y, z);
		}
	}

	fclose(file);

	return vertex;
}
