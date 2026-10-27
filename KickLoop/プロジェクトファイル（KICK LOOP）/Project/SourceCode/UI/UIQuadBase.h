#pragma once

#include "main.h"
#include "gameObject.h"
#include "renderer.h"

class UIQuadBase : public GameObject
{
private:
	ID3D11Buffer* m_VertexBuffer = nullptr;
	ID3D11InputLayout* m_VertexLayout = nullptr;
	ID3D11VertexShader* m_VertexShader = nullptr;
	ID3D11PixelShader* m_PixelShader = nullptr;
	ID3D11ShaderResourceView* m_Texture = nullptr;

protected:
	XMFLOAT4 m_Color = XMFLOAT4(1, 1, 1, 1);

public:
	void Uninit() override;

	void SetTexture(const char* path);
	void SetColor(float r, float g, float b, float a);
	void SetAlpha(float alpha);
	float GetAlpha() const { return m_Color.w; }

protected:
	void CreateQuad();
	void UpdateQuad(const VERTEX_3D vertex[4]);

	void SetDraw2D();
	void SetDraw3D();

	void DrawQuad();
};