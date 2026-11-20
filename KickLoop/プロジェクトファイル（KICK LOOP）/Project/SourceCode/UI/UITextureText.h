#pragma once

#include "UIQuadBase.h"

class UITextureText : public UIQuadBase
{
public:
	void Init() override;
	void Draw() override;

	// 文字画像の切り出し範囲と画面上の表示位置・大きさを設定する
	void SetAtlasRect(
		const UIRect& rect,
		const XMFLOAT4& pixels,
		const XMFLOAT2& textureSize
	);
};
