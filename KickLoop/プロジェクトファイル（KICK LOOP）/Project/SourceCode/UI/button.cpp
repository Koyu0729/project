#include "main.h"
#include "button.h"
#include "mouseInput.h"

void Button::Init()
{
	ShowCursor(TRUE);
}

void Button::Update()
{
}

bool Button::IsPushedOnce()
{
	if (!m_Enabled) return false;

	Vector3 pos = Mouse::Pos();

	bool inside =
		pos.x >= m_X && pos.x <= m_X + m_Width &&
		pos.y >= m_Y && pos.y <= m_Y + m_Height;

	bool pushed = inside && Mouse::LeftClick();

	if (pushed)
	{
		m_Enabled = false;   // ƒƒ“ƒVƒ‡ƒbƒg§ŒÀ
	}

	return pushed;
}
