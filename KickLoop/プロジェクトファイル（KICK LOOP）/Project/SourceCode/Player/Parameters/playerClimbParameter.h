#pragma once

struct PlayerClimbParameter
{
	bool enable = true;

	// よじ登りにかかるフレーム数。
	int climbFrameMax = 6;

	// よじ登り完了時、ブロック上端からどれだけ上へ出すか。
	float endYOffset = 7.0f;

	// ブロック外側へ押し出す距離。
	float forwardDistance = -5.85f;

	// 補間をなめらかにするか。
	bool useSmoothStep = true;

	// 体の高さチェック用オフセット。
	float bodyCheckOffsetY = 0.8f;

	// ブロック上端より少し下でも、よじ登りを許可する範囲。
	float climbMinOverTop = -1.25f;

	// ブロック上端より上に出すぎている場合は、よじ登り対象外にする範囲。
	float climbMaxOverTop = 2.0f;

	// よじ登り候補を探す疑似Ray判定のサイズと距離。
	float rayCheckWidth = 0.5f;
	float rayCheckHeightExpand = 0.8f;
	float rayCheckDepth = 1.5f;
	float rayMaxDistance = 14.0f;

	// Rayを進める刻み幅。
	// 小さいほど精度は上がるが、判定回数は増える。
	float rayStep = 0.2f;
};
