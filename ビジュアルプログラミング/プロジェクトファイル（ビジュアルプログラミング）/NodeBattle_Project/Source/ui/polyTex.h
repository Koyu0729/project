#pragma once

#include "gameObject2D.h"

class PolyTex : public GameObject2D
{
  private:
	ID3D11Buffer *m_VertexBuffer;

	ID3D11InputLayout *m_VertexLayout = nullptr;

	ID3D11VertexShader *m_VertexShader = nullptr;

	ID3D11PixelShader *m_PixelShader = nullptr;

	ID3D11ShaderResourceView *m_Texture = nullptr;

	XMFLOAT4 m_Color = {1, 1, 1, 1};

  public:
	// 初期化処理を行う
	void Init() override;

	// 終了処理を行う
	void Uninit() override;

	// 毎フレームの処理
	void Update() override;

	// 画面に描く処理
	void Draw() override;

	PolyTex *SetTexRule(
		Vector2 position = Vector2(0.0f, 0.0f),
		float rotation = 0.0f,
		Vector2 scale = Vector2(0.0f, 0.0f),
		const char *FileName = "asset\\texture\\test.png");

	void SetColor(const XMFLOAT4 &color)
	{
		m_Color = color;
	}
};
