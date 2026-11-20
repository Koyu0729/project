#include "stageData.h"

// 内部番号は既存データに合わせる。ステージ情報の追加・変更はこの一覧で行う
const StageDefinition StageData::m_Stages[] =
	{
		{0, StageType::Free, 0,
		 L"自由対戦",
		 L"射撃に盾、盾に回り込み、正面からの接近に射撃。距離に合わせて武器を選ぼう。",
		 "nodeSaveData/Tutorial3.json"},

		{1, StageType::Tutorial, 1,
		 L"チュートリアル1：はじめての槍",
		 L"近づく → 敵を向く → 槍で突く。連続実行で倒そう！",
		 "nodeSaveData/Stage1.json"},

		{2, StageType::Tutorial, 2,
		 L"チュートリアル2：構えを見てかわす",
		 L"黄色い構えが合図！横へ移動してかわし、休み中に突こう。",
		 "nodeSaveData/Stage2.json"},

		{3, StageType::Tutorial, 3,
		 L"チュートリアル3：壁を回り込もう",
		 L"壁は通れない！横へ移動 → 前へ進むで回り込んで突こう。",
		 "nodeSaveData/Stage3.json"},

		{4, StageType::Practice, 1,
		 L"実践ステージ1：動く敵との対戦",
		 L"突き → 回り込み → 距離のifで追撃する敵。自分の作戦で挑戦しよう！",
		 ""},

		{5, StageType::Tutorial, 4,
		 L"チュートリアル4：盾で射撃を防ぐ",
		 L"射撃の黄色い構えを見たら盾！正面を向いて防ぎ、盾を下ろして接近しよう。",
		 "nodeSaveData/Stage4.json"},

		{6, StageType::Tutorial, 5,
		 L"チュートリアル5：射撃をかわして接近",
		 L"射撃は構えた向きに飛ぶ！横へかわして接近。距離1.4未満なら敵は撃てない。",
		 "nodeSaveData/Stage5.json"},

		{7, StageType::Tutorial, 6,
		 L"チュートリアル6：盾の横を狙おう",
		 L"盾は正面120度だけ！敵の横へ回り込む → 敵を向く → 剣で斬る。",
		 "nodeSaveData/Stage6.json"},

		{8, StageType::Tutorial, 7,
		 L"チュートリアル7：槍の距離で剣に勝つ",
		 L"剣は速いけれど短い！距離2を保って槍で突けば、敵の剣は届かない。",
		 "nodeSaveData/Stage7.json"},

		{9, StageType::Tutorial, 8,
		 L"チュートリアル8：相性を使い分けよう",
		 L"遠い敵には盾や回り込み、近づいたら剣。盾の残量を見て休ませよう。",
		 "nodeSaveData/Stage8.json"},

		{10, StageType::Tutorial, 9,
		 L"チュートリアル9：射撃の距離で槍に勝つ",
		 L"槍の届かない距離4を保って射撃！敵が盾を使うなら回り込みへ切り替えよう。",
		 "nodeSaveData/Stage9.json"},

		{11, StageType::Practice, 2,
		 L"実践ステージ2：距離を取る射撃手",
		 L"近づくと離れ、構えを見て横へ避ける射撃手。盾と接近のタイミングを考えよう！",
		 ""},

		{12, StageType::Practice, 3,
		 L"実践ステージ3：盾と剣と槍の使い分け",
		 L"攻撃に反応して盾を構え、近くは剣・中距離は槍を使う敵。横から狙おう！",
		 ""}};

// 一覧にあるステージの総数
const int StageData::m_StageCount = sizeof(m_Stages) / sizeof(m_Stages[0]);

// 一覧から、種類ごとのステージ数を数える
int StageData::CountStages(StageType type)
{
	int count = 0;

	for (const StageDefinition &stage : m_Stages)
	{
		if (stage.type == type)
		{
			++count;
		}
	}

	return count;
}

// 内部番号に対応するステージ情報を探す
const StageDefinition &StageData::FindStageDefinition(int stageID)
{
	for (const StageDefinition &stage : m_Stages)
	{
		if (stage.stageID == stageID)
		{
			return stage;
		}
	}

	// 登録されていない番号は自由対戦として扱う
	return m_Stages[0];
}

// 種類とメニュー番号から、内部番号を探す
int StageData::FindStageID(StageType type, int menuNumber)
{
	for (const StageDefinition &stage : m_Stages)
	{
		if (stage.type == type && stage.menuNumber == menuNumber)
		{
			return stage.stageID;
		}
	}

	return m_Stages[0].stageID;
}

// 自由対戦に対応する先頭の番号
int StageData::GetFirstStageID()
{
	return m_Stages[0].stageID;
}

// 一覧の末尾にあるステージの番号
int StageData::GetLastStageID()
{
	return m_Stages[m_StageCount - 1].stageID;
}
