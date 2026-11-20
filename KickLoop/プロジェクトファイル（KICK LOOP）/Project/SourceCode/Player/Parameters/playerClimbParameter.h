#pragma once

struct PlayerClimbParameter
{
	bool enable = true;

	// よじ登りにかかるフレーム数。
	int climbFrameMax = 6;

	// よじ登り完了時、ブロック上端からどれだけ上へ出すか。
	float endYOffset = 7.0f;

	// ブロック中心から外向きを正とする距離。負値で内側へ寄せる。
	float forwardDistance = -5.85f;

	// 補間をなめらかにするか。
	bool useSmoothStep = true;

	// 体の高さチェック用オフセット。
	float bodyCheckOffsetY = 0.8f;

	// ブロック上端より少し下でも、よじ登りを許可する範囲。
	float climbMinOverTop = -1.25f;

	// ブロック上端より上に出すぎている場合は、よじ登り対象外にする範囲。
	float climbMaxOverTop = 2.0f;

	// よじ登れる足場を探すときの判定範囲
	float rayCheckWidth = 0.5f;         // X（左右の半幅）
	float rayCheckHeightExpand = 0.8f;  // Y（プレイヤーの当たり判定の半高に足す分）
	float rayCheckDepth = 1.5f;         // Z（奥行きの半分）
	float rayMaxDistance = 14.0f;

	// Rayを進める刻み幅。
	// 小さいほど精度は上がるが、判定回数は増える。
	float rayStep = 0.2f;
};
