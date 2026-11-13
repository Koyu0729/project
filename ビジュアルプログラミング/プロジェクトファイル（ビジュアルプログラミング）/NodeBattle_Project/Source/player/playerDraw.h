#pragma once

#include "main.h"

class Player;
class ModelRenderer;

// プレイヤーと武器の描画
class PlayerDraw
{
	// 操作対象のプレイヤー（ここでは削除しない）
	Player &m_Player;

	// 担当する実行状態・リソース
	ID3D11InputLayout *m_VertexLayout = nullptr;

	ID3D11VertexShader *m_VertexShader = nullptr;

	ID3D11PixelShader *m_PixelShader = nullptr;

	ModelRenderer *m_ModelRenderer = nullptr;

  public:
	explicit PlayerDraw(Player &player);

	// 描画に使うモデルとシェーダーを用意する
	void Init();

	void Uninit();
	// 画面に描く処理
	void Draw();
};
