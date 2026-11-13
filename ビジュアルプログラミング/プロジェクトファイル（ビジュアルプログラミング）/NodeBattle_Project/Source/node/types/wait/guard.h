#pragma once
#include "singleOutputNode.h"

// 設定した時間だけ盾を構えるノード（開始と終了は共通の実行処理で切り替える）
class Guard : public SingleOutputNode
{
  public:
	// 防御ノードの種類を設定する
	explicit Guard(int id);

	// 防御を終えたあとに続くノードを返す
	NodeBase *Execute() override;
};
