#pragma once

#include "gameObject.h"

class Sky : public GameObject
{
private:

	ID3D11InputLayout* m_VertexLayout;
	ID3D11VertexShader* m_VertexShader;
	ID3D11PixelShader* m_PixelShader;

	class ModelRenderer* m_ModelRenderer;

public:
	void Init() override;
	void Uninit() override;
	void Update() override;
	void Draw() override;
};
