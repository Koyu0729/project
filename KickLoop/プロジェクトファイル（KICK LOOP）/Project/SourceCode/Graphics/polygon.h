#pragma once

#include "gameObject.h"

class Polygon : public GameObject
{
private:
	// 描画用のバッファとシェーダー

	// 描画する頂点データ
	ID3D11Buffer * m_VertexBuffer = NULL;
	ID3D11InputLayout* m_VertexLayout;
	ID3D11VertexShader* m_VertexShader;
	ID3D11PixelShader* m_PixelShader;

public:
	void Init() override;
	void Uninit() override;
	void Update() override;
	void Draw() override;
};
