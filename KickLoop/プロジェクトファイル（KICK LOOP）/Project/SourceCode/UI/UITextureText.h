#pragma once

#include "UIQuadBase.h"

class UITextureText : public UIQuadBase
{
public:
	void Init() override;
	void Draw() override;

	void SetAtlasRect(
		const UIRect& rect,
		const XMFLOAT4& pixels,
		const XMFLOAT2& textureSize
	);
};
