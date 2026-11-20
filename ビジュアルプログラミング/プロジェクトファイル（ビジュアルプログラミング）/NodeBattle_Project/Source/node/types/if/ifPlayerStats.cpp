#include "ifPlayerStats.h"
#include "player.h"
#include <sstream>

// 条件を調べて、「はい」か「いいえ」を決める
bool IfPlayerStats::Branch()
{
	float percentValue;

	if (m_GameChara && m_GameChara->GetStats()->maxHp > 0)
	{
		percentValue = 100.0f * m_GameChara->GetHP() / m_GameChara->GetStats()->maxHp;
	}
	else
	{
		percentValue = 100.0f;
	}

	const float percent = percentValue;

	bool result = m_GameChara && percent <= m_Settings.hpPercent;

	if (m_GameChara)
	{
		m_GameChara->SetIfStats(result);
	}

	std::wostringstream text;

	const wchar_t *answerText;

	if (result)
	{
		answerText = L"はい";
	}
	else
	{
		answerText = L"いいえ";
	}

	text << L"HP " << percent << L"% → " << (answerText);

	m_ExecutionDetail = text.str();

	return result;
}
