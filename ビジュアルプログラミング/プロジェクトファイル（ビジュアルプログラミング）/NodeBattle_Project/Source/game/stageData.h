#pragma once
#include "stageDefinition.h"

// ステージの一覧と、番号・情報の検索を管理する
class StageData
{
  private:
	// 内部番号の小さい順に並べたステージ一覧と、その件数
	static const StageDefinition m_Stages[];
	static const int m_StageCount;

  public:
	// 種類ごとのメニュー項目数を数える
	static int CountStages(StageType type);

	// 内部番号からステージ情報を取得する。未登録なら自由対戦を返す
	static const StageDefinition &FindStageDefinition(int stageID);

	// 種類とメニュー番号から内部番号を探す
	static int FindStageID(StageType type, int menuNumber);

	// 選択できる内部番号の範囲
	static int GetFirstStageID();
	static int GetLastStageID();
};
