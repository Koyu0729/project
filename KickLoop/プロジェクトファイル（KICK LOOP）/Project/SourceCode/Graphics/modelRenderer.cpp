
#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <shlwapi.h>
#pragma comment(lib, "shlwapi.lib")

#include "main.h"
#include "renderer.h"
#include "modelRenderer.h"
#include "texture.h"

#include <memory>
#include <stdexcept>
#include <wrl/client.h>


std::unordered_map<std::string, MODEL*> ModelRenderer::m_ModelPool;


static std::unique_ptr<FILE, decltype(&fclose)> OpenModelFile(const char* fileName)
{
	std::unique_ptr<FILE, decltype(&fclose)> file(fopen(fileName, "rt"), fclose);
	if (!file)
	{
		throw std::runtime_error(std::string("Could not open model file: ") + fileName);
	}
	return file;
}


void ModelRenderer::Draw()
{

	// 頂点バッファ設定
	UINT stride = sizeof(VERTEX_3D);
	UINT offset = 0;
	Renderer::GetDeviceContext()->IASetVertexBuffers(0, 1, &m_Model->VertexBuffer, &stride, &offset);

	// インデックスバッファ設定
	Renderer::GetDeviceContext()->IASetIndexBuffer(m_Model->IndexBuffer, DXGI_FORMAT_R32_UINT, 0);

	// プリミティブトポロジ設定
	Renderer::GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);


	for( unsigned int i = 0; i < m_Model->SubsetNum; i++ )
	{
		// マテリアル設定
		Renderer::SetMaterial(m_Model->SubsetArray[i].Material.Material );

		// テクスチャ設定
		if(m_Model->SubsetArray[i].Material.Texture)
			Renderer::GetDeviceContext()->PSSetShaderResources( 0, 1, &m_Model->SubsetArray[i].Material.Texture );

		// ポリゴン描画
		Renderer::GetDeviceContext()->DrawIndexed(m_Model->SubsetArray[i].IndexNum, m_Model->SubsetArray[i].StartIndex, 0 );
	}

}

void ModelRenderer::SetUVFlip(bool flipU, bool flipV)
{
	m_FlipU = flipU;

	m_FlipV = flipV;
}

void ModelRenderer::Preload(const char *FileName)
{
	auto& cached = m_ModelPool[FileName];
	if (cached != nullptr)
	{
		return;
	}

	auto model = std::make_unique<MODEL>();

	LoadModel(
		FileName,
		model.get(),
		true,
		true
	);

	cached = model.release();

}


// 読み込み済みモデルをすべて解放する
void ModelRenderer::UnloadAll()
{
	for (std::pair<const std::string, MODEL*> pair : m_ModelPool)
	{
		if (pair.second == nullptr)
		{
			continue;
		}

		pair.second->VertexBuffer->Release();
		pair.second->IndexBuffer->Release();

		delete[] pair.second->SubsetArray;

		delete pair.second;
	}

	m_ModelPool.clear();
}


void ModelRenderer::Load(const char *FileName)
{
	auto& cached = m_ModelPool[FileName];
	if (cached != nullptr)
	{
		m_Model = cached;
		return;
	}

	auto model = std::make_unique<MODEL>();

	LoadModel(
		FileName,
		model.get(),
		m_FlipU,
		m_FlipV
	);

	cached = model.release();
	m_Model = cached;

}

void ModelRenderer::LoadModel( const char *FileName, MODEL *Model, bool flipU, bool flipV)
{

	MODEL_OBJ modelObj;

	LoadObj(
		FileName,
		&modelObj,
		flipU,
		flipV
	);

	std::unique_ptr<VERTEX_3D[]> vertices(modelObj.VertexArray);
	std::unique_ptr<unsigned int[]> indices(modelObj.IndexArray);
	std::unique_ptr<SUBSET[]> subsets(modelObj.SubsetArray);

	// 頂点バッファ生成
	Microsoft::WRL::ComPtr<ID3D11Buffer> vertexBuffer;
	{
		D3D11_BUFFER_DESC bd;
		ZeroMemory( &bd, sizeof(bd) );
		bd.Usage = D3D11_USAGE_DEFAULT;
		bd.ByteWidth = sizeof( VERTEX_3D ) * modelObj.VertexNum;
		bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		bd.CPUAccessFlags = 0;

		D3D11_SUBRESOURCE_DATA sd;
		ZeroMemory( &sd, sizeof(sd) );
		sd.pSysMem = modelObj.VertexArray;

		if (FAILED(Renderer::GetDevice()->CreateBuffer(&bd, &sd, vertexBuffer.GetAddressOf())))
		{
			throw std::runtime_error(std::string("Could not create OBJ vertex buffer: ") + FileName);
		}
	}


	// インデックスバッファ生成
	Microsoft::WRL::ComPtr<ID3D11Buffer> indexBuffer;
	{
		D3D11_BUFFER_DESC bd;
		ZeroMemory( &bd, sizeof(bd) );
		bd.Usage = D3D11_USAGE_DEFAULT;
		bd.ByteWidth = sizeof( unsigned int ) * modelObj.IndexNum;
		bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
		bd.CPUAccessFlags = 0;

		D3D11_SUBRESOURCE_DATA sd;
		ZeroMemory( &sd, sizeof(sd) );
		sd.pSysMem = modelObj.IndexArray;

		if (FAILED(Renderer::GetDevice()->CreateBuffer(&bd, &sd, indexBuffer.GetAddressOf())))
		{
			throw std::runtime_error(std::string("Could not create OBJ index buffer: ") + FileName);
		}
	}

	// サブセット設定
	for (unsigned int i = 0; i < modelObj.SubsetNum; i++)
	{
		auto& material = subsets[i].Material;
		material.Texture = Texture::Load(material.TextureName);
		material.Material.TextureEnable = material.Texture != nullptr;
	}

	Model->VertexBuffer = vertexBuffer.Detach();
	Model->IndexBuffer = indexBuffer.Detach();
	Model->SubsetArray = subsets.release();
	Model->SubsetNum = modelObj.SubsetNum;
}


//モデルを読み込む
void ModelRenderer::LoadObj( const char *FileName, MODEL_OBJ *ModelObj, bool flipU, bool flipV)
{

	char dir[MAX_PATH];
	strcpy (dir, FileName );
	PathRemoveFileSpec(dir);


	unsigned int	positionNum = 0;
	unsigned int	normalNum = 0;
	unsigned int	texcoordNum = 0;
	unsigned int	vertexNum = 0;
	unsigned int	indexNum = 0;
	unsigned int	in = 0;
	unsigned int	subsetNum = 0;

	std::unique_ptr<MODEL_MATERIAL[]> materialArray;
	unsigned int	materialNum = 0;

	char str[256];
	char *s;
	int c;


	auto modelFile = OpenModelFile(FileName);
	FILE* file = modelFile.get();


	//要素数カウント
	while (fscanf(file, "%255s", str) == 1)
	{

		if( strcmp( str, "v" ) == 0 )
		{
			positionNum++;
		}
		else if( strcmp( str, "vn" ) == 0 )
		{
			normalNum++;
		}
		else if( strcmp( str, "vt" ) == 0 )
		{
			texcoordNum++;
		}
		else if( strcmp( str, "usemtl" ) == 0 )
		{
			subsetNum++;
		}
		else if( strcmp( str, "f" ) == 0 )
		{
			in = 0;

			do
			{
				if (fscanf(file, "%255s", str) != 1)
				{
					throw std::runtime_error(std::string("Could not read OBJ face: ") + FileName);
				}
				vertexNum++;
				in++;
				c = fgetc( file );
			}
			while( c != '\n' && c != '\r' && c != EOF );

			//四角は三角に分割
			if( in == 4 )
				in = 6;

			indexNum += in;
		}
	}


	if (ferror(file) || positionNum == 0 || normalNum == 0 || indexNum == 0 || subsetNum == 0)
	{
		throw std::runtime_error(std::string("OBJ model has no drawable mesh: ") + FileName);
	}

	//メモリ確保
	auto positionArray = std::make_unique<XMFLOAT3[]>(positionNum);
	auto normalArray = std::make_unique<XMFLOAT3[]>(normalNum);
	auto texcoordArray = std::make_unique<XMFLOAT2[]>(texcoordNum);

	auto vertices = std::make_unique<VERTEX_3D[]>(vertexNum);
	auto indices = std::make_unique<unsigned int[]>(indexNum);
	auto subsets = std::make_unique<SUBSET[]>(subsetNum);


	auto readIndex = [FileName](const char* value, unsigned int count)
	{
		if (value != nullptr)
		{
			char* end = nullptr;
			const long index = strtol(value, &end, 10);
			if (*end == '\0' && index > 0 && static_cast<unsigned long>(index) <= count)
			{
				return index - 1;
			}
		}

		throw std::runtime_error(std::string("Invalid vertex reference in OBJ: ") + FileName);
	};

	//要素読込
	XMFLOAT3 *position = positionArray.get();
	XMFLOAT3 *normal = normalArray.get();
	XMFLOAT2 *texcoord = texcoordArray.get();

	unsigned int vc = 0;
	unsigned int ic = 0;
	unsigned int sc = 0;


	fseek( file, 0, SEEK_SET );

	while (fscanf(file, "%255s", str) == 1)
	{

		if( strcmp( str, "mtllib" ) == 0 )
		{
			//マテリアルファイル
			fscanf( file, "%255s", str );

			const std::string path = std::string(dir) + "\\" + str;
			MODEL_MATERIAL* materials = nullptr;
			LoadMaterial(path.c_str(), &materials, &materialNum);
			materialArray.reset(materials);
		}
		else if( strcmp( str, "o" ) == 0 )
		{
			//オブジェクト名
			fscanf( file, "%255s", str );
		}
		else if( strcmp( str, "v" ) == 0 )
		{
			//頂点座標
			fscanf( file, "%f", &position->x );
			fscanf( file, "%f", &position->y );
			fscanf( file, "%f", &position->z );
			position++;
		}
		else if( strcmp( str, "vn" ) == 0 )
		{
			//法線
			fscanf( file, "%f", &normal->x );
			fscanf( file, "%f", &normal->y );
			fscanf( file, "%f", &normal->z );
			normal++;
		}
		else if (strcmp(str, "vt") == 0)
		{
			// テクスチャ座標
			fscanf(file, "%f", &texcoord->x);
			fscanf(file, "%f", &texcoord->y);

			if (flipU)
			{
				texcoord->x = 1.0f - texcoord->x;
			}

			if (flipV)
			{
				texcoord->y = 1.0f - texcoord->y;
			}

			texcoord++;
		}
		else if( strcmp( str, "usemtl" ) == 0 )
		{
			//マテリアル
			fscanf( file, "%255s", str );

			if( sc != 0 )
				subsets[ sc - 1 ].IndexNum = ic - subsets[ sc - 1 ].StartIndex;

			subsets[ sc ].StartIndex = ic;


			unsigned int material = 0;
			for (; material < materialNum; material++)
			{
				if (strcmp(str, materialArray[material].Name) == 0)
				{
					break;
				}
			}

			if (material == materialNum)
			{
				throw std::runtime_error(std::string("Missing material in OBJ: ") + FileName + " (" + str + ")");
			}

			subsets[sc].Material = materialArray[material];
			sc++;
			
		}
		else if( strcmp( str, "f" ) == 0 )
		{
			//面
			in = 0;

			do
			{
				if (fscanf(file, "%255s", str) != 1 || strchr(str, '/') == nullptr)
				{
					throw std::runtime_error(std::string("Could not read OBJ vertex: ") + FileName);
				}

				s = strtok( str, "/" );	
				vertices[vc].Position = positionArray[readIndex(s, positionNum)];
				if( s[ strlen( s ) + 1 ] != '/' )
				{
					//テクスチャ座標が存在しない場合もある
					s = strtok( nullptr, "/" );
					vertices[vc].TexCoord = texcoordArray[readIndex(s, texcoordNum)];
				}
				s = strtok( nullptr, "/" );	
				vertices[vc].Normal = normalArray[readIndex(s, normalNum)];

				vertices[vc].Diffuse = XMFLOAT4( 1.0f, 1.0f, 1.0f, 1.0f );

				indices[ic] = vc;
				ic++;
				vc++;

				in++;
				c = fgetc( file );
			}
			while( c != '\n' && c != '\r' && c != EOF );

			//四角は三角に分割
			if( in == 4 )
			{
				indices[ic] = vc - 4;
				ic++;
				indices[ic] = vc - 2;
				ic++;
			}
		}
	}


	if( sc != 0 )
		subsets[ sc - 1 ].IndexNum = ic - subsets[ sc - 1 ].StartIndex;


	if (ferror(file))
	{
		throw std::runtime_error(std::string("Could not read OBJ model: ") + FileName);
	}

	ModelObj->VertexArray = vertices.release();
	ModelObj->VertexNum = vertexNum;
	ModelObj->IndexArray = indices.release();
	ModelObj->IndexNum = indexNum;
	ModelObj->SubsetArray = subsets.release();
	ModelObj->SubsetNum = subsetNum;
}


//マテリアルを読み込む
void ModelRenderer::LoadMaterial( const char *FileName, MODEL_MATERIAL **MaterialArray, unsigned int *MaterialNum )
{

	char dir[MAX_PATH];
	strcpy(dir, FileName);
	PathRemoveFileSpec(dir);


	char str[256];

	auto modelFile = OpenModelFile(FileName);
	FILE* file = modelFile.get();

	unsigned int materialNum = 0;

	//要素数カウント
	while (fscanf(file, "%255s", str) == 1)
	{


		if( strcmp( str, "newmtl" ) == 0 )
		{
			materialNum++;
		}
	}


	//メモリ確保
	if (ferror(file) || materialNum == 0)
	{
		throw std::runtime_error(std::string("Could not read MTL materials: ") + FileName);
	}

	auto materialArray = std::make_unique<MODEL_MATERIAL[]>(materialNum);


	//要素読込
	int mc = -1;

	fseek( file, 0, SEEK_SET );

	while (fscanf(file, "%255s", str) == 1)
	{


		if (mc < 0 && (strcmp(str, "Ka") == 0 || strcmp(str, "Kd") == 0 ||
			strcmp(str, "Ks") == 0 || strcmp(str, "Ns") == 0 || strcmp(str, "d") == 0 ||
			strcmp(str, "map_Kd") == 0))
		{
			throw std::runtime_error(std::string("Missing newmtl in MTL: ") + FileName);
		}

		if( strcmp( str, "newmtl" ) == 0 )
		{
			//マテリアル名
			mc++;
			fscanf( file, "%255s", materialArray[ mc ].Name );
			strcpy( materialArray[ mc ].TextureName, "" );

			materialArray[mc].Material.Emission.x = 0.0f;
			materialArray[mc].Material.Emission.y = 0.0f;
			materialArray[mc].Material.Emission.z = 0.0f;
			materialArray[mc].Material.Emission.w = 0.0f;
		}
		else if( strcmp( str, "Ka" ) == 0 )
		{
			//アンビエント
			fscanf( file, "%f", &materialArray[ mc ].Material.Ambient.x );
			fscanf( file, "%f", &materialArray[ mc ].Material.Ambient.y );
			fscanf( file, "%f", &materialArray[ mc ].Material.Ambient.z );
			materialArray[ mc ].Material.Ambient.w = 1.0f;
		}
		else if( strcmp( str, "Kd" ) == 0 )
		{
			//ディフューズ
			fscanf( file, "%f", &materialArray[ mc ].Material.Diffuse.x );
			fscanf( file, "%f", &materialArray[ mc ].Material.Diffuse.y );
			fscanf( file, "%f", &materialArray[ mc ].Material.Diffuse.z );
			materialArray[ mc ].Material.Diffuse.w = 1.0f;
		}
		else if( strcmp( str, "Ks" ) == 0 )
		{
			//スペキュラ
			fscanf( file, "%f", &materialArray[ mc ].Material.Specular.x );
			fscanf( file, "%f", &materialArray[ mc ].Material.Specular.y );
			fscanf( file, "%f", &materialArray[ mc ].Material.Specular.z );
			materialArray[ mc ].Material.Specular.w = 1.0f;
		}
		else if( strcmp( str, "Ns" ) == 0 )
		{
			//スペキュラ強度
			fscanf( file, "%f", &materialArray[ mc ].Material.Shininess );
		}
		else if( strcmp( str, "d" ) == 0 )
		{
			//アルファ
			fscanf( file, "%f", &materialArray[ mc ].Material.Diffuse.w );
		}
		else if( strcmp( str, "map_Kd" ) == 0 )
		{
			//テクスチャ
			fscanf( file, "%255s", str );

			const std::string path = std::string(dir) + "\\" + str;
			if (path.size() >= sizeof(materialArray[mc].TextureName))
			{
				throw std::runtime_error(std::string("Texture path is too long in MTL: ") + FileName);
			}
			strcpy(materialArray[mc].TextureName, path.c_str());
		}
	}

	if (ferror(file))
	{
		throw std::runtime_error(std::string("Could not read MTL file: ") + FileName);
	}

	*MaterialArray = materialArray.release();
	*MaterialNum = materialNum;
}

