#pragma once

#include "main.h"
#include "renderer.h"
#include "gameObject.h"

// ステージ上に配置する基本ブロック。
// 描画、AABB/OBB当たり判定、接触面の法線、壁アクション可否をまとめて持つ。
class StageBlock : public GameObject
{
protected:
	ID3D11Buffer* m_VertexBuffer = nullptr;
	ID3D11InputLayout* m_VertexLayout = nullptr;
	ID3D11Buffer* m_IndexBuffer = nullptr;
	ID3D11VertexShader* m_VertexShader = nullptr;
	ID3D11PixelShader* m_PixelShader = nullptr;
	ID3D11ShaderResourceView* m_Texture = nullptr;

	Vector3 m_Size = Vector3(1.0f, 1.0f, 1.0f);
	XMFLOAT4 m_Color = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);

	XMFLOAT2 m_UVScroll = XMFLOAT2(0.0f, 0.0f);
	XMFLOAT2 m_UVScrollDirection = XMFLOAT2(1.0f, 0.0f);
	XMFLOAT2 m_UVRepeat = XMFLOAT2(1.0f, 1.0f);

	bool m_IsUVFlipZ = false;
	bool m_IsUVScroll = true;

	bool m_IsRainbow = false;
	float m_RainbowSpeed = 1.0f;
	float m_Alpha = 1.0f;

	// ブロックごとに、壁キック、壁スライド、よじ登りの可否を切り替える。
	int m_WallKickGroupId = -1;
	bool m_CanAddWallKickGauge = true;

	bool m_CanWallKick = true;
	bool m_CanWallSlide = true;
	bool m_CanClimb = true;

	// 回転した足場や斜面だけOBBを使う。
	bool m_UseOBBHit = false;

	bool m_IsGoalTopBlock = false;

protected:
	void BuildVertexData(VERTEX_3D* vertex);
	void NormalizeSize();

public:
	void Init() override;
	void Uninit() override;
	void Update() override;
	void Draw() override;

	void SetTexture(const char* path);

	void SetUVFlipZ(bool isFlip);
	void AddUVScroll(float scroll);
	void UpdateVertexUV();

	void SetUVScrollDirection(XMFLOAT2 direction);
	void SetUVScroll(bool isScroll);
	void SetUVRepeat(XMFLOAT2 repeat);

	void SetSize(Vector3 size);
	Vector3 GetSize() const;

	Vector3 GetMin() const;
	Vector3 GetMax() const;

	virtual void SetColor(
		float r,
		float g,
		float b,
		float a
	);

	void SetRainbow(bool isRainbow);
	void SetRainbowSpeed(float speed);
	void SetAlpha(float alpha);
	XMFLOAT3 GetRainbowColor(float time) const;

	bool IsHit(
		Vector3 position,
		Vector3 size
	);

	bool IsHitAABB(
		Vector3 position,
		Vector3 size
	) const;

	bool IsHitOBB(
		Vector3 position,
		Vector3 size
	) const;

	bool GetHitNormal(
		Vector3 position,
		Vector3 size,
		Vector3& outNormal
	);

	bool IsHitTopFace(
		Vector3 position,
		Vector3 size,
		Vector3& outNormal
	);

	void SetUseOBBHit(bool useOBBHit);
	void SetBlockRotation(Vector3 rotation);

	// OBBのローカル+Y面を斜面として扱い、接地位置と法線を返す。
	bool GetSlopeRideNormal(
		Vector3 position,
		Vector3 size,
		Vector3& outNormal
	);

	bool GetSlopeRideSurfaceY(
		Vector3 position,
		Vector3 size,
		float& outSurfaceY,
		Vector3& outNormal
	);

	void SetWallKickGroupId(int groupId)
	{
		m_WallKickGroupId = groupId;
	}

	int GetWallKickGroupId() const
	{
		return m_WallKickGroupId;
	}

	void SetCanAddWallKickGauge(bool canAdd)
	{
		m_CanAddWallKickGauge = canAdd;
	}

	bool CanAddWallKickGauge() const
	{
		return m_CanAddWallKickGauge;
	}

	void SetCanWallKick(bool canWallKick)
	{
		m_CanWallKick = canWallKick;
	}

	bool CanWallKick() const
	{
		return m_CanWallKick;
	}

	void SetCanWallSlide(bool canWallSlide)
	{
		m_CanWallSlide = canWallSlide;
	}

	bool CanWallSlide() const
	{
		return m_CanWallSlide;
	}

	void SetCanClimb(bool canClimb)
	{
		m_CanClimb = canClimb;
	}

	bool CanClimb() const
	{
		return m_CanClimb;
	}

	void SetGoalTopBlock(bool isGoalTopBlock)
	{
		m_IsGoalTopBlock = isGoalTopBlock;
	}

	bool IsGoalTopBlock() const
	{
		return m_IsGoalTopBlock;
	}
};
