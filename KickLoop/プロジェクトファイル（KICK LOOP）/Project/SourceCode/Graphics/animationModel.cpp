#include "main.h"
#include "renderer.h"
#include "animationModel.h"

#include <memory>
#include <stdexcept>
#include <wrl/client.h>

void AnimationModel::Draw()
{
	// プリミティブトポロジ設定
	Renderer::GetDeviceContext()->IASetPrimitiveTopology(
		D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	
	// マテリアル設定
	MATERIAL material;
	ZeroMemory(&material, sizeof(material));
	material.Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	material.Ambient = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	material.TextureEnable = true;
	Renderer::SetMaterial(material);

	for (unsigned int m = 0; m < m_AiScene->mNumMeshes; m++)
	{
		aiMesh* mesh = m_AiScene->mMeshes[m];


		// マテリアル設定
		aiString texture;
		aiColor3D diffuse(1.0f, 1.0f, 1.0f);
		float opacity = 1.0f;

		aiMaterial* aimaterial = m_AiScene->mMaterials[mesh->mMaterialIndex];

		if (aimaterial != nullptr)
		{
			aimaterial->GetTexture(aiTextureType_DIFFUSE, 0, &texture);

			if (AI_SUCCESS != aimaterial->Get(AI_MATKEY_COLOR_DIFFUSE, diffuse))
			{
				diffuse = aiColor3D(1.0f, 1.0f, 1.0f);
			}

			if (AI_SUCCESS != aimaterial->Get(AI_MATKEY_OPACITY, opacity))
			{
				opacity = 1.0f;
			}
		}
		aimaterial->GetTexture(aiTextureType_DIFFUSE, 0, &texture);
		aimaterial->Get(AI_MATKEY_COLOR_DIFFUSE, diffuse);
		aimaterial->Get(AI_MATKEY_OPACITY, opacity);

		if (texture == aiString(""))
		{
			material.TextureEnable = false;
		}
		else
		{
			Renderer::GetDeviceContext()->PSSetShaderResources(0, 1, &m_Texture[texture.data]);
			material.TextureEnable = true;
		}

		material.Diffuse = XMFLOAT4(diffuse.r, diffuse.g, diffuse.b, opacity);
		material.Ambient = material.Diffuse;
		Renderer::SetMaterial(material);


		// 頂点バッファ設定
		UINT stride = sizeof(VERTEX_3D);
		UINT offset = 0;
		Renderer::GetDeviceContext()->IASetVertexBuffers(0, 1, &m_VertexBuffer[m], &stride, &offset);

		// インデックスバッファ設定
		Renderer::GetDeviceContext()->IASetIndexBuffer(m_IndexBuffer[m], DXGI_FORMAT_R32_UINT, 0);

		// ポリゴン描画
		Renderer::GetDeviceContext()->DrawIndexed(mesh->mNumFaces * 3, 0, 0);
	}
}

void AnimationModel::Load( const char *FileName )
{
	const std::string modelPath( FileName );

	m_AiScene = aiImportFile(FileName, aiProcessPreset_TargetRealtime_MaxQuality | aiProcess_ConvertToLeftHanded);
	if (m_AiScene == nullptr)
	{
		throw std::runtime_error("Could not load FBX model: " + modelPath + "\n" + aiGetErrorString());
	}

	if (!m_AiScene->HasMeshes() || m_AiScene->mRootNode == nullptr)
	{
		throw std::runtime_error("FBX model has no mesh or root node: " + modelPath);
	}

	m_VertexBuffer = new ID3D11Buffer*[m_AiScene->mNumMeshes]{};
	m_IndexBuffer = new ID3D11Buffer*[m_AiScene->mNumMeshes]{};


	//変形後頂点配列生成
	m_DeformVertex = new std::vector<DEFORM_VERTEX>[m_AiScene->mNumMeshes];

	//再帰的にボーン生成
	CreateBone(m_AiScene->mRootNode);


	for (unsigned int m = 0; m < m_AiScene->mNumMeshes; m++)
	{
		aiMesh* mesh = m_AiScene->mMeshes[m];

		if (!mesh->HasPositions() || !mesh->HasNormals() || !mesh->HasTextureCoords(0) || !mesh->HasFaces())
		{
			throw std::runtime_error("FBX mesh is missing geometry or UVs: " + modelPath);
		}

		// 頂点バッファ生成
		{
			std::vector<VERTEX_3D> vertex(mesh->mNumVertices);

			for (unsigned int v = 0; v < mesh->mNumVertices; v++)
			{
				vertex[v].Position = XMFLOAT3(mesh->mVertices[v].x, mesh->mVertices[v].y, mesh->mVertices[v].z);
				vertex[v].Normal = XMFLOAT3(mesh->mNormals[v].x, mesh->mNormals[v].y, mesh->mNormals[v].z);
				vertex[v].TexCoord = XMFLOAT2( mesh->mTextureCoords[0][v].x, mesh->mTextureCoords[0][v].y);
				vertex[v].Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
			}

			D3D11_BUFFER_DESC bd;
			ZeroMemory(&bd, sizeof(bd));
			bd.Usage = D3D11_USAGE_DYNAMIC;
			bd.ByteWidth = sizeof(VERTEX_3D) * mesh->mNumVertices;
			bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
			bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

			D3D11_SUBRESOURCE_DATA sd;
			ZeroMemory(&sd, sizeof(sd));
			sd.pSysMem = vertex.data();

			if (FAILED(Renderer::GetDevice()->CreateBuffer(&bd, &sd, &m_VertexBuffer[m])))
			{
				throw std::runtime_error("Could not create FBX vertex buffer: " + modelPath);
			}
		}


		// インデックスバッファ生成
		{
			std::vector<unsigned int> index(mesh->mNumFaces * 3);

			for (unsigned int f = 0; f < mesh->mNumFaces; f++)
			{
				const aiFace* face = &mesh->mFaces[f];

				if (face->mNumIndices != 3)
				{
					throw std::runtime_error("FBX mesh contains a non-triangle face: " + modelPath);
				}

				index[f * 3 + 0] = face->mIndices[0];
				index[f * 3 + 1] = face->mIndices[1];
				index[f * 3 + 2] = face->mIndices[2];
			}

			D3D11_BUFFER_DESC bd;
			ZeroMemory(&bd, sizeof(bd));
			bd.Usage = D3D11_USAGE_DEFAULT;
			bd.ByteWidth = sizeof(unsigned int) * mesh->mNumFaces * 3;
			bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
			bd.CPUAccessFlags = 0;

			D3D11_SUBRESOURCE_DATA sd;
			ZeroMemory(&sd, sizeof(sd));
			sd.pSysMem = index.data();

			if (FAILED(Renderer::GetDevice()->CreateBuffer(&bd, &sd, &m_IndexBuffer[m])))
			{
				throw std::runtime_error("Could not create FBX index buffer: " + modelPath);
			}
		}


		//変形後頂点データ初期化
		for (unsigned int v = 0; v < mesh->mNumVertices; v++)
		{
			DEFORM_VERTEX deformVertex;
			deformVertex.Position = mesh->mVertices[v];
			deformVertex.Normal = mesh->mNormals[v];
			deformVertex.BoneNum = 0;

			for (unsigned int b = 0; b < 4; b++)
			{
				deformVertex.BoneName[b] = "";
				deformVertex.BoneWeight[b] = 0.0f;
			}

			m_DeformVertex[m].push_back(deformVertex);
		}


		//ボーンデータ初期化
		for (unsigned int b = 0; b < mesh->mNumBones; b++)
		{
			aiBone* bone = mesh->mBones[b];

			m_Bone[bone->mName.C_Str()].OffsetMatrix = bone->mOffsetMatrix;

			//変形後頂点にボーンデータ格納
			for (unsigned int w = 0; w < bone->mNumWeights; w++)
			{
				aiVertexWeight weight = bone->mWeights[w];

				if (weight.mVertexId >= mesh->mNumVertices)
				{
					throw std::runtime_error("FBX bone weight references a missing vertex: " + modelPath);
				}

				DEFORM_VERTEX& vertex = m_DeformVertex[m][weight.mVertexId];
				int num = vertex.BoneNum;
				if (num >= ARRAYSIZE(vertex.BoneWeight))
				{
					throw std::runtime_error("FBX vertex has too many bone weights: " + modelPath);
				}

				vertex.BoneWeight[num] = weight.mWeight;
				vertex.BoneName[num] = bone->mName.C_Str();
				vertex.BoneNum++;
			}
		}
	}


	//テクスチャ読み込み
	for(int i = 0; i < m_AiScene->mNumTextures; i++)
	{
		aiTexture* aitexture = m_AiScene->mTextures[i];

		// テクスチャ読み込み
		TexMetadata metadata{};
		ScratchImage image;
		if (aitexture->mHeight != 0 ||
			FAILED(LoadFromWICMemory(aitexture->pcData, aitexture->mWidth, WIC_FLAGS_NONE, &metadata, image)))
		{
			throw std::runtime_error("Could not read embedded texture in FBX: " + modelPath +
				" (" + aitexture->mFilename.C_Str() + ")");
		}

		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> texture;
		if (FAILED(CreateShaderResourceView(Renderer::GetDevice(), image.GetImages(),
			image.GetImageCount(), metadata, texture.GetAddressOf())))
		{
			throw std::runtime_error("Could not create embedded texture in FBX: " + modelPath +
				" (" + aitexture->mFilename.C_Str() + ")");
		}

		auto& loadedTexture = m_Texture[aitexture->mFilename.C_Str()];
		if (loadedTexture != nullptr)
		{
			loadedTexture->Release();
		}
		loadedTexture = texture.Detach();
	}


}


void AnimationModel::LoadAnimation( const char *FileName, const char *Name )
{
	std::unique_ptr<const aiScene, decltype(&aiReleaseImport)> animation(
		aiImportFile(FileName, aiProcess_ConvertToLeftHanded), aiReleaseImport);
	if (!animation)
	{
		throw std::runtime_error(std::string("Could not load FBX animation: ") + FileName + "\n" + aiGetErrorString());
	}

	// 最初のクリップの各チャンネルで位置キーと回転キーを確認する
	if (animation->HasAnimations())
	{
		const aiAnimation* clip = animation->mAnimations[0];
		for (unsigned int c = 0; c < clip->mNumChannels; c++)
		{
			const aiNodeAnim* channel = clip->mChannels[c];
			if (channel->mNumRotationKeys == 0 || channel->mNumPositionKeys == 0)
			{
				throw std::runtime_error(std::string("FBX animation is missing rotation or position keys: ") + FileName);
			}
		}
	}

	auto& loadedAnimation = m_Animation[Name];
	if (loadedAnimation != nullptr)
	{
		aiReleaseImport(loadedAnimation);
	}
	loadedAnimation = animation.release();
}


void AnimationModel::CreateBone(aiNode* node)
{
	BONE bone;

	m_Bone[node->mName.C_Str()] = bone;

	for (unsigned int n = 0; n < node->mNumChildren; n++)
	{
		CreateBone(node->mChildren[n]);
	}

}


void AnimationModel::Uninit()
{
	if (m_AiScene != nullptr)
	{
		for (unsigned int m = 0; m < m_AiScene->mNumMeshes; m++)
		{
			if (m_VertexBuffer != nullptr && m_VertexBuffer[m] != nullptr)
			{
				m_VertexBuffer[m]->Release();
			}

			if (m_IndexBuffer != nullptr && m_IndexBuffer[m] != nullptr)
			{
				m_IndexBuffer[m]->Release();
			}
		}
	}

	delete[] m_VertexBuffer;
	delete[] m_IndexBuffer;
	delete[] m_DeformVertex;
	m_VertexBuffer = nullptr;
	m_IndexBuffer = nullptr;
	m_DeformVertex = nullptr;

	for (const auto& pair : m_Texture)
	{
		if (pair.second != nullptr)
		{
			pair.second->Release();
		}
	}
	m_Texture.clear();

	if (m_AiScene != nullptr)
	{
		aiReleaseImport(m_AiScene);
		m_AiScene = nullptr;
	}

	for (const auto& pair : m_Animation)
	{
		aiReleaseImport(pair.second);
	}
	m_Animation.clear();
	m_Bone.clear();
}


void AnimationModel::Update(const char *AnimationName1, int Frame1,
							const char* AnimationName2, int Frame2,
							float BlendRate)
{
	if (m_Animation.count(AnimationName1) == 0)
		return;

	if (!m_Animation[AnimationName1]->HasAnimations())
		return;

	if (m_Animation.count(AnimationName2) == 0)
		return;

	if (!m_Animation[AnimationName2]->HasAnimations())
		return;

	//アニメーションデータからボーンマトリクス算出
	aiAnimation* animation1 = m_Animation[AnimationName1]->mAnimations[0];
	aiAnimation* animation2 = m_Animation[AnimationName2]->mAnimations[0];

	for (auto pair : m_Bone)
	{
		BONE* bone = &m_Bone[pair.first];

		aiNodeAnim* nodeAnim1 = nullptr;

		for (unsigned int c = 0; c < animation1->mNumChannels; c++)
		{
			if (animation1->mChannels[c]->mNodeName == aiString(pair.first))
			{
				nodeAnim1 = animation1->mChannels[c];
				break;
			}
		}

		aiNodeAnim* nodeAnim2 = nullptr;

		for (unsigned int c = 0; c < animation2->mNumChannels; c++)
		{
			if (animation2->mChannels[c]->mNodeName == aiString(pair.first))
			{
				nodeAnim2 = animation2->mChannels[c];
				break;
			}
		}


		aiQuaternion rot1;
		aiVector3D pos1;
		int f;

		if (nodeAnim1)
		{
			f = Frame1 % nodeAnim1->mNumRotationKeys;
			rot1 = nodeAnim1->mRotationKeys[f].mValue;

			f = Frame1 % nodeAnim1->mNumPositionKeys;
			pos1 = nodeAnim1->mPositionKeys[f].mValue;
		}

		aiQuaternion rot2;
		aiVector3D pos2;

		if (nodeAnim2)
		{
			f = Frame2 % nodeAnim2->mNumRotationKeys;
			rot2 = nodeAnim2->mRotationKeys[f].mValue;

			f = Frame2 % nodeAnim2->mNumPositionKeys;
			pos2 = nodeAnim2->mPositionKeys[f].mValue;
		}


		aiVector3D pos;
		pos = pos1 * (1.0f - BlendRate) + pos2 * BlendRate; //線形補間

		aiQuaternion rot;
		aiQuaternion::Interpolate(rot, rot1, rot2, BlendRate); // 球面線形補間

		bone->AnimationMatrix = aiMatrix4x4(aiVector3D(1.0f, 1.0f, 1.0f), rot, pos);

	}

	//再帰的にボーンマトリクスを更新
	aiMatrix4x4 rootMatrix = aiMatrix4x4(aiVector3D(1.0f, 1.0f, 1.0f),
		aiQuaternion((float)AI_MATH_PI, 0.0f, 0.0f), aiVector3D(0.0f, 0.0f, 0.0f));

	UpdateBoneMatrix(m_AiScene->mRootNode, rootMatrix);

	//頂点変換（CPUスキニング）
	for (unsigned int m = 0; m < m_AiScene->mNumMeshes; m++)
	{
		aiMesh* mesh = m_AiScene->mMeshes[m];

		D3D11_MAPPED_SUBRESOURCE ms;
		Renderer::GetDeviceContext()
			->Map(m_VertexBuffer[m], 0, D3D11_MAP_WRITE_DISCARD, 0, &ms);
		VERTEX_3D* vertex = (VERTEX_3D*)ms.pData;

		for (unsigned int v = 0; v < mesh->mNumVertices; v++)
		{
			DEFORM_VERTEX* deformVertex = &m_DeformVertex[m][v];

			aiMatrix4x4 matrix[4];
			matrix[0] = m_Bone[deformVertex->BoneName[0]].Matrix;
			matrix[1] = m_Bone[deformVertex->BoneName[1]].Matrix;
			matrix[2] = m_Bone[deformVertex->BoneName[2]].Matrix;
			matrix[3] = m_Bone[deformVertex->BoneName[3]].Matrix;


			aiMatrix4x4 outMatrix;
			outMatrix = matrix[0] * deformVertex->BoneWeight[0]
						+ matrix[1] * deformVertex->BoneWeight[1]
						+ matrix[2] * deformVertex->BoneWeight[2]
						+ matrix[3] * deformVertex->BoneWeight[3];

			deformVertex->Position = mesh->mVertices[v];
			deformVertex->Position *= outMatrix;

			//法線変換用に移動成分を削除
			outMatrix.a4 = 0.0f;
			outMatrix.b4 = 0.0f;
			outMatrix.c4 = 0.0f;


			deformVertex->Normal = mesh->mNormals[v];
			deformVertex->Normal *= outMatrix;

			//頂点バッファへ書き込み
			vertex[v].Position.x = deformVertex->Position.x;
			vertex[v].Position.y = deformVertex->Position.y;
			vertex[v].Position.z = deformVertex->Position.z;

			vertex[v].Normal.x = deformVertex->Normal.x;
			vertex[v].Normal.y = deformVertex->Normal.y;
			vertex[v].Normal.z = deformVertex->Normal.z;

			vertex[v].TexCoord.x = mesh->mTextureCoords[0][v].x;
			vertex[v].TexCoord.y = mesh->mTextureCoords[0][v].y;

			vertex[v].Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
		}

		Renderer::GetDeviceContext()->Unmap(m_VertexBuffer[m], 0);
	}

	
}


// アニメーションからボーンの変換行列を更新する
void AnimationModel::UpdateBoneMatrix(aiNode* node, aiMatrix4x4 matrix)
{
	BONE* bone = &m_Bone[node->mName.C_Str()];

	aiMatrix4x4 worldMatrix;
	worldMatrix = matrix * bone->AnimationMatrix;

	bone->Matrix = worldMatrix * bone->OffsetMatrix;

	for (unsigned int n = 0; n < node->mNumChildren; n++)
	{
		UpdateBoneMatrix(node->mChildren[n], worldMatrix);
	}

}

