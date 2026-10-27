#pragma once

#include "UIQuadBase.h"

class UISpriteSheetBase : public UIQuadBase
{
protected:
	int m_DivideX = 1;
	int m_DivideY = 1;

public:
	void SetSpriteSheet(int divideX, int divideY);

protected:
	void DrawSpriteIndex(
		int index,
		float x,
		float y,
		float w,
		float h
	);

	void DrawSpriteIndex(
		int index,
		float x,
		float y,
		float w,
		float h,
		float alpha
	);

	void DrawSpriteIndex(
		ID3D11ShaderResourceView* texture,
		int index,
		int divideX,
		int divideY,
		float x,
		float y,
		float w,
		float h
	);

	void DrawSpriteIndex(
		ID3D11ShaderResourceView* texture,
		int index,
		int divideX,
		int divideY,
		float x,
		float y,
		float w,
		float h,
		float alpha
	);
};