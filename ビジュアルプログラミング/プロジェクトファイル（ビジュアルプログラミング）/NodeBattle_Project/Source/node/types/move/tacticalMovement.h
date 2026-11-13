#pragma once
#include "singleOutputNode.h"

// 敵との距離を調節する、または敵の横の位置まで移動する
class TacticalMovement : public SingleOutputNode
{
	// 回り込みを始めたときの敵の向き（追尾で目標が回り続けるのを防ぐ）
	float m_EnemyHeading = 0;

  public:
	// 距離を保つ時間と、回り込みの制限時間を用意する
	TacticalMovement(int id, NodeType type);

	// 敵の横を決める基準の向きを、開始時に保存する
	void Begin() override;

	// 次につながるノードを返す
	NodeBase *Execute() override;

	// 相手の位置に合わせて移動し、到着または制限時間で次へ進む
	NodeStatus Tick(float dt) override;
};
