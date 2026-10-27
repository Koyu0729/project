#include "ifFindObject.h"
#include "player.h"

// 条件を調べて、「はい」か「いいえ」を決める
bool IfFindObject::Branch()
{
	bool found = m_GameChara && m_GameChara->HasVisibleTarget();

	if (m_GameChara)
	{
		m_GameChara->SetIfFindObject(found);
	}

	if (found)
	{
		m_ExecutionDetail = L"敵を発見 → はい";
	}
	else
	{
		m_ExecutionDetail = L"敵が見つからない → いいえ";
	}

	return found;
}
