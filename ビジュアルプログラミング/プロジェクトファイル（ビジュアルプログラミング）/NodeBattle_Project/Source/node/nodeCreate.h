#pragma once

#include "nodeList.h"
#include "vector2.h"

class NodeEditor;

class NodeManager;

class NodeView;

class NodeBase;

class NodeCreate
{
  private:
  public:
	// 種類に対応する実行用ノードを生成する
	static NodeBase *CreateLogic(NodeCategory category, NodeType type, int createId);

	// ノード本体と、画面の見た目を結びつける
	static NodeView *CreateView(NodeBase *logic, Vector2 releasedPos);
};
