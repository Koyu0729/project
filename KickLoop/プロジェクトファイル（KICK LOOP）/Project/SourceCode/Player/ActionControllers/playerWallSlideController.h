#pragma once

#include "vector3.h"

class Camera;
class StageBlockManager;
struct PlayerRuntimeState;
struct PlayerWallContact;

// 接触結果を記録し、壁滑りの開始・維持・解除を扱う。
class PlayerWallSlideController
{
private:
	Camera* m_Camera = nullptr;
	StageBlockManager* m_StageBlockManager = nullptr;

public:
	void SetSceneReferences(Camera* camera, StageBlockManager* stageBlockManager);
	void ApplyContact(PlayerRuntimeState& state, const PlayerWallContact& wall);
	void ClearLock(PlayerRuntimeState& state);

	// 壁との接触と視線に応じて、壁滑り中の横位置を固定または解除する
	void Update(
		Vector3& position,
		Vector3& move,
		Vector3 hitSize,
		PlayerRuntimeState& state,
		PlayerWallContact& wall
	);

private:
	void StartLock(Vector3 position, PlayerRuntimeState& state, const PlayerWallContact& wall);
	// 滑っている壁との接触を調べ、接触先と壁の法線を更新する
	bool UpdateLockedContact(Vector3 position, Vector3 hitSize, PlayerRuntimeState& state);
	// 視線の向きから壁スライドの固定を解除するか判定する
	bool ShouldReleaseByCamera(const PlayerRuntimeState& state) const;
	Vector3 GetCameraForwardXZ() const;
};
