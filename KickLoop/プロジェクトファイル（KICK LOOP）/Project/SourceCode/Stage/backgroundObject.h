#pragma once

#include "main.h"
#include "gameObject.h"

#include <string>

class ModelRenderer;

// 背景用のOBJモデルを描画する基本クラス。
class BackgroundObject : public GameObject
{
protected:
	ID3D11InputLayout* m_VertexLayout = nullptr;
	ID3D11VertexShader* m_VertexShader = nullptr;
	ID3D11PixelShader* m_PixelShader = nullptr;

	ModelRenderer* m_ModelRenderer = nullptr;

	// 現在読み込んでいるOBJモデルのパス。
	std::string m_ModelPath = "";

public:
	void Init() override;
	void Uninit() override;
	void Update() override;
	void Draw() override;

	void LoadModel(const char* path);
	void LoadModel(const std::string& path);

	void AddMoveZ(float move);

	void SetActive(bool active);
	bool IsActive() const;

	bool HasModel() const;
};
