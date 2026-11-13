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

	void Update(
		Vector3& position,
		Vector3& move,
		Vector3 hitSize,
		PlayerRuntimeState& state,
		PlayerWallContact& wall
	);

private:
	void StartLock(Vector3 position, PlayerRuntimeState& state, const PlayerWallContact& wall);
	bool UpdateLockedContact(Vector3 position, Vector3 hitSize, PlayerRuntimeState& state);
	bool ShouldReleaseByCamera(const PlayerRuntimeState& state) const;
	Vector3 GetCameraForwardXZ() const;
};
