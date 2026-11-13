#pragma once

#include "gameObject3D.h"

class Player;

class Explosion : public GameObject3D
{
	// 着弾の画像と、秒で進めるアニメーション
	class BillboardSprite *m_Sprite = nullptr;

	float m_Elapsed = 0;

	int m_Frame = 0;

	// ノードの一時停止では着弾の見た目も停止する
	Player *m_Owner = nullptr;

  public:
	// 着弾画像を用意する
	void Init() override;

	// 描画リソースを解放する
	void Uninit() override;

	// 経過時間からコマを選ぶ
	void Update() override;

	// カメラに正対する着弾画像を描く
	void Draw() override;

	void SetOwner(Player *owner)
	{
		m_Owner = owner;
	}
};
