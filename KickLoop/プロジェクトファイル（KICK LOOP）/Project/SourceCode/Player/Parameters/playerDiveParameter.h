#pragma once

struct PlayerDiveParameter
{
	// Ctrlを押した直後に、少し上へ浮かせる速度。
	float floatUpVelocity = 0.42f;

	// 浮き上がり状態を続けるフレーム数。
	int floatUpFrame = 16;

	// 浮き上がり後、下方向へ落とす速度。
	float diveDownVelocity = -2.35f;

	// 急降下時に横方向の勢いをどれだけ残すか。
	// 0.0fに近いほど真下へ落ちる。
	float horizontalBrakeRate = 0.25f;
};
