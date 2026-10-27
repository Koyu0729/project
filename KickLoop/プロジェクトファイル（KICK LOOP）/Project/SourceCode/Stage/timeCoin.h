#pragma once

#include <string>

#include "main.h"
#include "gameObject.h"

class ModelRenderer;

// 取得すると制限時間を増やすステージギミック。
class TimeCoin : public GameObject
{
private:
	ID3D11InputLayout* m_VertexLayout = nullptr;
	ID3D11VertexShader* m_VertexShader = nullptr;
	ID3D11PixelShader* m_PixelShader = nullptr;

	ModelRenderer* m_ModelRenderer = nullptr;

	std::string m_ModelPath = "asset\\model\\TimeCoin.obj";

	// 取得判定用の中心座標と半径サイズ。
	Vector3 m_HitCenter = Vector3(0.0f, 0.0f, 0.0f);
	Vector3 m_HitSize = Vector3(0.7f, 0.7f, 0.7f);

	// 取得時に増やす制限時間。
	float m_AddTime = 5.0f;

	// 表示用モデルの回転速度。
	float m_RotationSpeedY = 0.04f;

public:
	void Init() override;
	void Uninit() override;
	void Update() override;
	void Draw() override;

	void Create(
		Vector3 position,
		Vector3 size
	);

	void SetAddTime(float addTime);

	Vector3 GetHitCenter() const;
	Vector3 GetHitSize() const;

private:
	void LoadModel();

	void UpdateHit();
	bool IsHitPlayer() const;

	void NormalizeHitSize();

	void RecordCollectedTimeCoin();
};
