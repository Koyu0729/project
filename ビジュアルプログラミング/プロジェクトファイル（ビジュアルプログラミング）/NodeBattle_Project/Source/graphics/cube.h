#pragma once

#include "gameObject3D.h"
#include <string>

// 位置・大きさ・画像を指定して描く直方体
class Cube : public GameObject3D
{
	// このCubeが所有する描画用のバッファとシェーダー
	ID3D11Buffer *m_VertexBuffer = nullptr;
	ID3D11InputLayout *m_VertexLayout = nullptr;
	ID3D11VertexShader *m_VertexShader = nullptr;
	ID3D11PixelShader *m_PixelShader = nullptr;

	// 画像は共有キャッシュから借り、ここでは解放しない
	ID3D11ShaderResourceView *m_Texture = nullptr;
	std::string m_TexturePath;

	// 各方向の画像の繰り返し回数と色
	Vector3 m_TextureRepeat = {1, 1, 1};
	XMFLOAT4 m_Color = {1, 1, 1, 1};

  public:
	void Init() override;

	void Uninit() override;

	void Update() override;

	void Draw() override;

	// 初期化する前に、画像と繰り返し回数を指定する
	void SetTexture(const std::string &path);

	void SetTextureRepeat(Vector3 repeat);

	void SetColor(XMFLOAT4 color);
};
