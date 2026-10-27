#pragma once

#include "stageBlock.h"

// emissiveMaskPSへ渡すマスク色とマスク範囲。
struct StageMaskBlockEmissiveBuffer
{
	XMFLOAT4 emissiveColor;

	// x = emissivePower
	// y = maskColorStart
	// z = maskColorEnd
	// w = padding
	XMFLOAT4 emissiveParam;

	// x = maskCoreStart
	// y = maskCoreEnd
	// z = padding
	// w = padding
	XMFLOAT4 maskParam;
};

// StageBlockを継承した、マスク部分の色を変えるステージブロック。
class StageMaskBlock : public StageBlock
{
private:
	ID3D11ShaderResourceView* m_MaskTexture = nullptr;
	ID3D11Buffer* m_EmissiveBuffer = nullptr;

	XMFLOAT4 m_EmissiveColor = XMFLOAT4(0.0f, 0.85f, 1.0f, 1.0f);

	float m_EmissivePower = 1.8f;

	float m_MaskColorStart = 0.08f;
	float m_MaskColorEnd = 0.60f;

	float m_MaskCoreStart = 0.15f;
	float m_MaskCoreEnd = 0.65f;

private:
	float Clamp01(float value) const;

	void NormalizeMaskRange(
		float& start,
		float& end
	) const;

public:
	void Init() override;
	void Uninit() override;
	void Draw() override;

	void ResetForReuse();

	void SetMaskTexture(const char* path);

	void SetEmissiveColor(
		float r,
		float g,
		float b,
		float a
	);

	void SetEmissivePower(float power);

	void SetMaskColorRange(
		float start,
		float end
	);

	void SetMaskCoreRange(
		float start,
		float end
	);

	// StageModuleManagerから渡された色を、マスク部分の色として使う。
	void SetColor(
		float r,
		float g,
		float b,
		float a
	) override;
};
