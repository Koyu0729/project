#include "ifRangeIO.h"
#include "player.h"
#include <sstream>

// 条件を調べて、「はい」か「いいえ」を決める
bool IfRangeIO::Branch()
{
	GameObject3D *target;

	if (m_GameChara)
	{
		target = m_GameChara->GetTargetObject();
	}
	else
	{
		target = nullptr;
	}

	bool result = false;

	if (target)
	{
		Vector3 offset = target->GetPosition() - m_GameChara->GetPosition();

		offset.y = 0;

		const float distance = offset.length();

		result = distance <= m_Settings.distance;

		std::wostringstream text;

		text.precision(2);

		const wchar_t *answerText;

		if (result)
		{
			answerText = L"はい";
		}
		else
		{
			answerText = L"いいえ";
		}

		text << L"距離 " << std::fixed << distance << L" → " << (answerText);

		m_ExecutionDetail = text.str();
	}
	else
	{
		m_ExecutionDetail = L"敵なし → いいえ";
	}

	if (m_GameChara)
	{
		m_GameChara->SetIfRangeIO(result);
	}

	return result;
}
