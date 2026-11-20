#pragma once

#include "UIQuadBase.h"

class UISpriteSheetBase : public UIQuadBase
{
protected:
	// スプライトシートの分割数
	int m_DivideX = 1;  //横
	int m_DivideY = 1;  //縦

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