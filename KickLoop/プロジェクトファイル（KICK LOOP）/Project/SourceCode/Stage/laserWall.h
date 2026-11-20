#pragma once

#include <string>
#include <vector>

#include "main.h"
#include "gameObject.h"
#include "playerActionRequest.h"

class Scene;
class StageBlock;
class Player;


// レーザー線の動き方
enum class LaserWallMoveType
{
	Frame,
	GridLine
};

// 格子線の種類
enum class LaserWallGridLineType
{
	None,
	Vertical,
	Horizontal
};

// 配置面
enum class LaserWallPlaneType
{
	VerticalXY,
	HorizontalXZ,
	SideYZ
};

// LaserWallを構成する1本分の線ブロック情報。
// localStart / localEndはLaserWall中心から見たローカル座標。
struct LaserWallLineData
{
	StageBlock* block = nullptr;

	Vector3 localStart = Vector3(0.0f, 0.0f, 0.0f);
	Vector3 localEnd = Vector3(0.0f, 0.0f, 0.0f);

	LaserWallMoveType moveType = LaserWallMoveType::GridLine;
	LaserWallGridLineType gridLineType = LaserWallGridLineType::None;

	float appearDelay = 0.0f;  // 壁が出現し始めてから、この線が伸び始めるまでの秒数
	float appearDuration = 0.35f;  // この線が伸びきるまでの秒数
};

// 複数のStageBlockを線として並べ、レーザー壁として扱うクラス。
class LaserWall : public GameObject
{
private:
	// 現在使用中の線ブロックと、再利用待ちの内部Pool。
	std::vector<LaserWallLineData> m_LaserLines;
	std::vector<StageBlock*> m_LaserLinePool;

	// レーザー全体の当たり判定中心と半径サイズ。
	Vector3 m_HitCenter = Vector3(25.0f, 8.0f, 20.0f);
	Vector3 m_HitSize = Vector3(10.0f, 7.0f, 1.0f);

	// 線ブロックの太さ、奥行き、格子数、見た目設定。
	float m_LaserThickness = 0.18f;
	float m_LaserDepth = 0.25f;

	// 外枠の内側に置く格子線の本数（壁のローカル座標）
	int m_VerticalLineCount = 4;    // X方向に並べる縦線
	int m_HorizontalLineCount = 4;  // Y方向に並べる横線

	std::string m_TexturePath = "asset\texture\siro.png";

	float m_ColorR = 1.0f;
	float m_ColorG = 0.0f;
	float m_ColorB = 0.0f;
	float m_ColorA = 1.0f;

	// 出現演出と格子線のパターン移動用タイマー。
	float m_AnimationTime = 0.0f;
	float m_AppearAnimationTime = 0.0f;

	bool m_UseAppearAnimation = true;
	bool m_HasStartedAppearAnimation = false;

	float m_AppearStartDistance = 35.0f;  // プレイヤーが壁の中心からこの距離に入ると出現開始

	float m_BaseAppearDuration = 0.35f;
	float m_LineAppearInterval = 0.08f;  // 次の線が伸び始めるまでの間隔（秒）

	bool m_EnablePatternAnimation = false;

	float m_PatternMoveTime = 2.0f;
	float m_PatternCoolTime = 1.5f;
	float m_DiagonalAngle = 0.785398f;

	// 接触時に発生させる時間ペナルティ。
	bool m_UseTimePenaltyEffect = true;

	float m_TimePenalty = 2.0f;

	int m_TimePenaltyCooldownFrame = 60;
	int m_TimePenaltyCurrentCooldownFrame = 0;

	// XY / XZ / YZ のどの面にレーザーを配置するか。
	LaserWallPlaneType m_PlaneType = LaserWallPlaneType::VerticalXY;

private:
	StageBlock* GetLaserLineFromPool(Scene* scene);
	void ReturnLaserLineToPool(StageBlock* block);
	void ReturnAllLinesToPool();
	void DestroyAllLineBlocks();

	// ローカル座標で線を作る
	StageBlock* CreateLaserLineLocal(
		Scene* scene,
		Vector3 localStart,
		Vector3 localEnd,
		LaserWallMoveType moveType,
		LaserWallGridLineType gridLineType,
		float appearDelay,
		float appearDuration
	);

	// 中心座標から方向と長さで線を作る
	StageBlock* CreateLaserLineFromCenter(
		Scene* scene,
		Vector3 localCenter,
		Vector3 direction,
		float halfLength,
		LaserWallMoveType moveType,
		LaserWallGridLineType gridLineType,
		float appearDelay,
		float appearDuration
	);

	// レーザーの配置面に合わせて両端の間に線を配置する
	void ApplyLaserBlockTransform(
		StageBlock* laser,
		Vector3 start,
		Vector3 end
	);

	// XY平面上の両端に合わせてレーザーの位置・長さ・向きを決める
	void ApplyVerticalLaserBlockTransform(
		StageBlock* laser,
		Vector3 start,
		Vector3 end
	);

	// XZ平面上の両端に合わせてレーザーの位置・長さ・向きを決める
	void ApplyHorizontalLaserBlockTransform(
		StageBlock* laser,
		Vector3 start,
		Vector3 end
	);

	// YZ平面上の両端に合わせてレーザーの位置・長さ・向きを決める
	void ApplySideLaserBlockTransform(
		StageBlock* laser,
		Vector3 start,
		Vector3 end
	);

	// ローカル座標で回転させる
	Vector3 RotateLocalPoint(
		Vector3 point,
		float angle
	) const;

	Vector3 ConvertLocalToWorldPoint(Vector3 localPoint) const;

	// 格子が斜めに傾いて元に戻る動きの進み具合を求める
	float GetPatternRate();

	void CheckStartAppearAnimationByDistance(Vector3 playerPosition);

	float GetAppearRate(const LaserWallLineData& line) const;
	float GetAppearEndTime() const;

	void ApplyAnimatedLaserLines();

	// レーザー線のうち、壁の枠内に収まる区間を求める
	bool ClipLineToLocalRect(
		Vector3& start,
		Vector3& end
	) const;

	// 枠の一辺との交差を調べ、レーザー線が枠内に残る区間を絞る
	bool ClipTest(
		float p,
		float q,
		float& t0,
		float& t1
	) const;

	bool CheckAABB(
		Vector3 centerA,
		Vector3 sizeA,
		Vector3 centerB,
		Vector3 sizeB
	) const;

	void ApplyEffectToPlayer(Player* player);
	void ApplyTimePenalty();

	Vector3 GetCurrentHitSize() const;

	Player* GetPlayer() const;

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
		Vector3 size,
		bool startVisible = true
	);

	void DestroyLines();

	void AddMoveZ(float move);

	bool IsHitPlayer(Player* player);

	bool IsHitAABB(
		Vector3 position,
		Vector3 size
	);

	void SetHitAABB(
		Vector3 center,
		Vector3 size
	);

	Vector3 GetHitCenter() const;
	Vector3 GetHitSize() const;

	void SetActive(bool isActive);
	bool IsActive() const;

	void SetTexturePath(const std::string& texturePath);

	void SetAlpha(float alpha);

	void SetColor(
		float r,
		float g,
		float b,
		float a
	);

	void SetLaserThickness(float thickness);
	void SetLaserDepth(float depth);

	void SetGridLineCount(
		int verticalCount,
		int horizontalCount
	);

	void SetStartAnimation(
		bool useStartAnimation,
		float appearStartDistance
	);

	void SetAppearAnimation(
		bool useAppearAnimation,
		float baseAppearDuration,
		float lineAppearInterval
	);

	void SetPatternAnimation(bool enablePatternAnimation);

	void SetPatternTiming(
		float patternMoveTime,
		float patternCoolTime
	);

	void SetDiagonalAngle(float diagonalAngle);

	void SetTimePenaltyEffect(
		bool useTimePenaltyEffect,
		float timePenalty,
		int cooldownFrame
	);

	void SetSlowEffect(
		bool useSlowEffect,
		float slowPenalty,
		int slowFrame
	);

	void SetPlaneType(LaserWallPlaneType planeType);
};
