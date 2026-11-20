#pragma once

#include <unordered_map>

#include "assimp/cimport.h"
#include "assimp/scene.h"
#include "assimp/postprocess.h"
#include "assimp/matrix4x4.h"
#pragma comment (lib, "assimp-vc143-mt.lib")

#include "component.h"


//変形後頂点構造体
struct DEFORM_VERTEX
{
	aiVector3D Position;
	aiVector3D Normal;
	int				BoneNum;
	std::string		BoneName[4];//ボーン名を保持する
	float			BoneWeight[4];
};

//ボーン構造体
struct BONE
{
	aiMatrix4x4 Matrix;
	aiMatrix4x4 AnimationMatrix;
	aiMatrix4x4 OffsetMatrix;
};

class AnimationModel
{
private:
	const aiScene* m_AiScene = nullptr;
	std::unordered_map<std::string, const aiScene*> m_Animation;

	ID3D11Buffer** m_VertexBuffer = nullptr;
	ID3D11Buffer** m_IndexBuffer = nullptr;

	std::unordered_map<std::string, ID3D11ShaderResourceView*> m_Texture;

	std::vector<DEFORM_VERTEX>* m_DeformVertex = nullptr;
	std::unordered_map<std::string, BONE> m_Bone;

	void CreateBone(aiNode* Node);
	// 親ボーンの変換を子へ引き継ぎ、各ボーンの変形行列を更新する
	void UpdateBoneMatrix(aiNode* Node, aiMatrix4x4 Matrix);

public:
	AnimationModel() = default;
	~AnimationModel() {}

	void Load(const char* FileName);
	void LoadAnimation(const char* FileName, const char* Name);
	void Uninit();
	// 2つのアニメーションを混ぜ合わせてモデルの頂点を変形する
	void Update(
		const char* AnimationName1,
		int Frame1,
		const char* AnimationName2,
		int Frame2,
		float BlendRate
	);
	void Draw();
};