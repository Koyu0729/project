#include "UITextureText.h"

void UITextureText::Init()
{
	CreateQuad();
}

void UITextureText::Draw()
{
	DrawQuad2D();
}

void UITextureText::SetAtlasRect(
	const UIRect& rect,
	const XMFLOAT4& pixels,
	const XMFLOAT2& textureSize
)
{
	// 画像上の左・上・右・下の座標を UV に変換する。
	UpdateRectQuad(rect, XMFLOAT4(
		pixels.x / textureSize.x,
		pixels.y / textureSize.y,
		pixels.z / textureSize.x,
		pixels.w / textureSize.y
	));
}
