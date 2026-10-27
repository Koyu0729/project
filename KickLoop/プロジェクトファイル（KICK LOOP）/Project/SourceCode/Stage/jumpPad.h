#pragma once

#include <string>

#include "main.h"
#include "gameObject.h"

#include "playerActionRequest.h"

class Scene;
class Player;
class ModelRenderer;

// ジャンプ台用のActionEffectを渡すギミック。
class JumpPad : public GameObject
{
private:
	ModelRenderer* m_ModelRenderer = nullptr;

	std::string m_ModelPath = "asset\\model\\jumpPad.obj";

	Vector3 m_DrawScale = Vector3(1.0f, 1.0f, 1.0f);

	// Y回転に対応したOBB判定用の中心・半径サイズ・角度。
	Vector3 m_HitCenter = Vector3(25.0f, 1.6f, 20.0f);
	Vector3 m_HitSize = Vector3(2.5f, 0.5f, 2.5f);
	float m_RotationY = 0.0f;

	// 接触時にPlayerActionEffectとして渡すジャンプ台の移動量。
	Vector3 m_JumpVelocity = Vector3(0.0f, 1.6f, 5.0f);
	float m_FloatPowerRate = 5.0f;

	int m_CooldownFrame = 20;
	int m_CurrentCooldownFrame = 0;

	// 最後のジャンプ台用。
	// 通常のActionEffectではなく、ゴール床へ向かう専用移動をPlayer側で開始する。
	bool m_UseGoalLaunch = false;
	bool m_IsGoalLaunchStarted = false;

	Vector3 m_GoalTargetPosition = Vector3(25.0f, 180.0f, 5400.0f);

	int m_GoalLaunchFrameMax = 120;
	float m_GoalLaunchHeight = 60.0f;

public:
	void Init() override;
	void Uninit() override;
	void Update() override;
	void Draw() override;

	void ResetForReuse();
	void SleepForPool();

	void Create(
		Scene* scene,
		Vector3 center,
		Vector3 size
	);

	void AddMoveZ(float move);

	bool IsHitPlayer(Player* player);

	void SetHitOBB(
		Vector3 center,
		Vector3 size,
		float rotationY
	);

	void SetHitAABB(
		Vector3 center,
		Vector3 size
	);

	Vector3 GetHitCenter();
	Vector3 GetHitSize();

	float GetRotationY();

	void SetModelPath(std::string modelPath);
	void SetDrawScale(Vector3 drawScale);

	void SetJumpVelocity(Vector3 jumpVelocity);
	Vector3 GetJumpVelocity();

	void SetFloatPowerRate(float floatPowerRate);
	float GetFloatPowerRate();

	void SetCooldownFrame(int cooldownFrame);

	void SetRotationY(float rotationY);

	void SetGoalLaunch(
		bool useGoalLaunch,
		Vector3 targetPosition,
		int frameMax,
		float height
	);

	void SetActive(bool isActive);
	bool IsActive();

private:
	bool CheckOBB(
		Vector3 obbCenter,
		Vector3 obbHalfSize,
		float obbRotationY,
		Vector3 playerCenter,
		Vector3 playerHalfSize
	);

	Vector3 WorldToLocalByYaw(
		Vector3 worldPosition,
		Vector3 center,
		float rotationY
	);

	Vector3 RotateVectorByYaw(
		Vector3 vector,
		float rotationY
	);

	PlayerActionEffect CreateActionEffect();

	void ApplyFloatByVelocity(
		PlayerActionEffect& effect,
		Vector3 velocity
	);

	float CalculateMoveDecay(Vector3 velocity);

	void StartGoalLaunch(Player* player);

	void UpdateDrawScale();
	void LoadModel();
};
