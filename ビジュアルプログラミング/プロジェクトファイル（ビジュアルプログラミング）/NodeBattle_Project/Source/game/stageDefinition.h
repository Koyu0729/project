#pragma once
#include "stageType.h"

// ステージ1つ分の番号・表示・サンプルをまとめる
struct StageDefinition
{
	// 内部番号、用途、メニュー上の番号
	int stageID;
	StageType type;
	int menuNumber;

	// 画面に表示する名前と説明
	const wchar_t *name;
	const wchar_t *hint;

	// 実践は空欄、それ以外はノードのサンプルを指定する
	const char *samplePath;
};
