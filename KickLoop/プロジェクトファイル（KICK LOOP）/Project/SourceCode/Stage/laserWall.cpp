#include "main.h"
#include "laserWall.h"

#include "stageBlock.h"
#include "scene.h"
#include "player.h"

#include "manager.h"
#include "gameTime.h"

#include <cmath>

void LaserWall::Init()
{
	m_LaserLines.clear();
	m_LaserLinePool.clear();

	ResetForReuse();
}

void LaserWall::Uninit()
{
	DestroyAllLineBlocks();
}

void LaserWall::Update()
{
	if (!Active())
	{
		return;
	}

	Player* player =
		GetPlayer();

	m_AnimationTime += 1.0f / 60.0f;

	if (m_TimePenaltyCurrentCooldownFrame > 0)
	{
		m_TimePenaltyCurrentCooldownFrame--;
	}

	if (m_UseAppearAnimation)
	{
		if (player != nullptr)
		{
			CheckStartAppearAnimationByDistance(
				player->GetPosition()
			);
		}

		if (m_HasStartedAppearAnimation)
		{
			m_AppearAnimationTime += 1.0f / 60.0f;
		}
	}
	else
	{
		m_HasStartedAppearAnimation = true;
		m_AppearAnimationTime = 999.0f;
	}

	ApplyEffectToPlayer(
		player
	);

	ApplyAnimatedLaserLines();
}

void LaserWall::Draw()
{
}


void LaserWall::ResetForReuse()
{
	ReturnAllLinesToPool();

	m_HitCenter = Vector3(25.0f, 8.0f, 20.0f);
	m_HitSize = Vector3(10.0f, 7.0f, 1.0f);

	m_LaserThickness = 0.18f;
	m_LaserDepth = 0.25f;

	m_VerticalLineCount = 4;
	m_HorizontalLineCount = 4;

	m_TexturePath = "asset\\texture\\siro.png";

	m_ColorR = 1.0f;
	m_ColorG = 0.0f;
	m_ColorB = 0.0f;
	m_ColorA = 1.0f;

	m_AnimationTime = 0.0f;
	m_AppearAnimationTime = 0.0f;

	m_UseAppearAnimation = true;
	m_HasStartedAppearAnimation = false;

	m_AppearStartDistance = 35.0f;

	m_BaseAppearDuration = 0.35f;
	m_LineAppearInterval = 0.08f;

	m_EnablePatternAnimation = false;

	m_PatternMoveTime = 2.0f;
	m_PatternCoolTime = 1.5f;
	m_DiagonalAngle = 0.785398f;

	m_UseTimePenaltyEffect = true;
	m_TimePenalty = 2.0f;

	m_TimePenaltyCooldownFrame = 60;
	m_TimePenaltyCurrentCooldownFrame = 0;

	GameObject::SetActive(false);
	SetVisible(false);

	m_PlaneType = LaserWallPlaneType::VerticalXY;
}

void LaserWall::SleepForPool()
{
	ReturnAllLinesToPool();

	GameObject::SetActive(false);
	SetVisible(false);

	m_TimePenaltyCurrentCooldownFrame = 0;

	m_AnimationTime = 0.0f;
	m_AppearAnimationTime = 0.0f;
	m_HasStartedAppearAnimation = false;
}


void LaserWall::Create(
	Scene* scene,
	Vector3 center,
	Vector3 size,
	bool startVisible
)
{
	if (scene == nullptr)
	{
		return;
	}

	ReturnAllLinesToPool();

	m_HitCenter = center;
	m_HitSize = size;

	if (m_HitSize.x <= 0.001f)
	{
		m_HitSize.x = 0.001f;
	}

	if (m_HitSize.y <= 0.001f)
	{
		m_HitSize.y = 0.001f;
	}

	if (m_HitSize.z <= 0.001f)
	{
		m_HitSize.z = 0.001f;
	}

	m_AnimationTime = 0.0f;
	m_AppearAnimationTime = 0.0f;

	m_TimePenaltyCurrentCooldownFrame = 0;

	if (startVisible)
	{
		m_UseAppearAnimation = false;
		m_HasStartedAppearAnimation = true;
		m_AppearAnimationTime = 999.0f;
	}
	else
	{
		m_UseAppearAnimation = true;
		m_HasStartedAppearAnimation = false;
		m_AppearAnimationTime = 0.0f;
	}

	GameObject::SetActive(true);
	SetVisible(true);

	float left = -m_HitSize.x;
	float right = m_HitSize.x;

	float bottom = -m_HitSize.y;
	float top = m_HitSize.y;

	float z = 0.0f;

	float longLength =
		sqrtf(
			m_HitSize.x * m_HitSize.x +
			m_HitSize.y * m_HitSize.y
		) * 3.0f;

	CreateLaserLineLocal(
		scene,
		Vector3(left, bottom, z),
		Vector3(right, bottom, z),
		LaserWallMoveType::Frame,
		LaserWallGridLineType::None,
		0.0f,
		m_BaseAppearDuration
	);

	CreateLaserLineLocal(
		scene,
		Vector3(right, bottom, z),
		Vector3(right, top, z),
		LaserWallMoveType::Frame,
		LaserWallGridLineType::None,
		m_LineAppearInterval,
		m_BaseAppearDuration
	);

	CreateLaserLineLocal(
		scene,
		Vector3(right, top, z),
		Vector3(left, top, z),
		LaserWallMoveType::Frame,
		LaserWallGridLineType::None,
		m_LineAppearInterval * 2.0f,
		m_BaseAppearDuration
	);

	CreateLaserLineLocal(
		scene,
		Vector3(left, top, z),
		Vector3(left, bottom, z),
		LaserWallMoveType::Frame,
		LaserWallGridLineType::None,
		m_LineAppearInterval * 3.0f,
		m_BaseAppearDuration
	);

	if (m_VerticalLineCount > 0)
	{
		float verticalSpacing =
			(m_HitSize.x * 2.0f) /
			static_cast<float>(m_VerticalLineCount + 1);

		for (int i = 1; i <= m_VerticalLineCount; i++)
		{
			float x =
				left +
				verticalSpacing *
				static_cast<float>(i);

			float delay =
				0.35f +
				static_cast<float>(i - 1) *
				m_LineAppearInterval;

			CreateLaserLineFromCenter(
				scene,
				Vector3(x, 0.0f, z),
				Vector3(0.0f, 1.0f, 0.0f),
				longLength,
				LaserWallMoveType::GridLine,
				LaserWallGridLineType::Vertical,
				delay,
				m_BaseAppearDuration
			);
		}
	}

	if (m_HorizontalLineCount > 0)
	{
		float horizontalSpacing =
			(m_HitSize.y * 2.0f) /
			static_cast<float>(m_HorizontalLineCount + 1);

		for (int i = 1; i <= m_HorizontalLineCount; i++)
		{
			float y =
				bottom +
				horizontalSpacing *
				static_cast<float>(i);

			float delay =
				0.35f +
				static_cast<float>(i - 1) *
				m_LineAppearInterval;

			CreateLaserLineFromCenter(
				scene,
				Vector3(0.0f, y, z),
				Vector3(1.0f, 0.0f, 0.0f),
				longLength,
				LaserWallMoveType::GridLine,
				LaserWallGridLineType::Horizontal,
				delay,
				m_BaseAppearDuration
			);
		}
	}

	ApplyAnimatedLaserLines();
}


void LaserWall::DestroyLines()
{
	ReturnAllLinesToPool();
}

StageBlock* LaserWall::GetLaserLineFromPool(Scene* scene)
{
	StageBlock* block = nullptr;

	if (!m_LaserLinePool.empty())
	{
		block =
			m_LaserLinePool.back();

		m_LaserLinePool.pop_back();
	}
	else
	{
		if (scene == nullptr)
		{
			return nullptr;
		}

		block =
			scene->AddGameObject<StageBlock>(2);
	}

	if (block == nullptr)
	{
		return nullptr;
	}

	block->SetActive(false);
	block->SetVisible(false);

	return block;
}

void LaserWall::ReturnLaserLineToPool(StageBlock* block)
{
	if (block == nullptr)
	{
		return;
	}

	block->SetActive(false);
	block->SetVisible(false);

	m_LaserLinePool.push_back(
		block
	);
}

void LaserWall::ReturnAllLinesToPool()
{
	for (LaserWallLineData& line : m_LaserLines)
	{
		ReturnLaserLineToPool(
			line.block
		);

		line.block = nullptr;
	}

	m_LaserLines.clear();
}

void LaserWall::DestroyAllLineBlocks()
{
	for (LaserWallLineData& line : m_LaserLines)
	{
		if (line.block == nullptr)
		{
			continue;
		}

		line.block->SetActive(false);
		line.block->SetVisible(false);
		line.block->SetDestroy();
		line.block = nullptr;
	}

	m_LaserLines.clear();

	for (StageBlock* block : m_LaserLinePool)
	{
		if (block == nullptr)
		{
			continue;
		}

		block->SetActive(false);
		block->SetVisible(false);
		block->SetDestroy();
	}

	m_LaserLinePool.clear();
}


StageBlock* LaserWall::CreateLaserLineLocal(
	Scene* scene,
	Vector3 localStart,
	Vector3 localEnd,
	LaserWallMoveType moveType,
	LaserWallGridLineType gridLineType,
	float appearDelay,
	float appearDuration
)
{
	if (scene == nullptr)
	{
		return nullptr;
	}

	StageBlock* laser =
		GetLaserLineFromPool(scene);

	if (laser == nullptr)
	{
		return nullptr;
	}

	laser->SetTexture(
		m_TexturePath.c_str()
	);

	laser->SetUVScroll(false);
	laser->SetRainbow(false);

	laser->SetColor(
		m_ColorR,
		m_ColorG,
		m_ColorB,
		m_ColorA
	);

	laser->SetAlpha(
		m_ColorA
	);

	laser->SetActive(
		Active()
	);

	laser->SetVisible(
		Visible()
	);

	LaserWallLineData line;

	line.block = laser;
	line.localStart = localStart;
	line.localEnd = localEnd;
	line.moveType = moveType;
	line.gridLineType = gridLineType;
	line.appearDelay = appearDelay;
	line.appearDuration = appearDuration;

	m_LaserLines.push_back(
		line
	);

	return laser;
}

StageBlock* LaserWall::CreateLaserLineFromCenter(
	Scene* scene,
	Vector3 localCenter,
	Vector3 direction,
	float halfLength,
	LaserWallMoveType moveType,
	LaserWallGridLineType gridLineType,
	float appearDelay,
	float appearDuration
)
{
	float directionLength =
		sqrtf(
			direction.x * direction.x +
			direction.y * direction.y
		);

	if (directionLength <= 0.001f)
	{
		return nullptr;
	}

	direction.x /= directionLength;
	direction.y /= directionLength;
	direction.z = 0.0f;

	Vector3 localStart =
		Vector3(
			localCenter.x - direction.x * halfLength,
			localCenter.y - direction.y * halfLength,
			localCenter.z
		);

	Vector3 localEnd =
		Vector3(
			localCenter.x + direction.x * halfLength,
			localCenter.y + direction.y * halfLength,
			localCenter.z
		);

	return CreateLaserLineLocal(
		scene,
		localStart,
		localEnd,
		moveType,
		gridLineType,
		appearDelay,
		appearDuration
	);
}


void LaserWall::ApplyLaserBlockTransform(
	StageBlock* laser,
	Vector3 start,
	Vector3 end
)
{
	if (m_PlaneType == LaserWallPlaneType::HorizontalXZ)
	{
		ApplyHorizontalLaserBlockTransform(
			laser,
			start,
			end
		);

		return;
	}

	if (m_PlaneType == LaserWallPlaneType::SideYZ)
	{
		ApplySideLaserBlockTransform(
			laser,
			start,
			end
		);

		return;
	}

	ApplyVerticalLaserBlockTransform(
		laser,
		start,
		end
	);
}

void LaserWall::ApplyVerticalLaserBlockTransform(
	StageBlock* laser,
	Vector3 start,
	Vector3 end
)
{
	if (laser == nullptr)
	{
		return;
	}

	float dx =
		end.x -
		start.x;

	float dy =
		end.y -
		start.y;

	float length =
		sqrtf(
			dx * dx +
			dy * dy
		);

	if (length <= 0.001f)
	{
		laser->SetActive(false);
		laser->SetVisible(false);
		return;
	}

	Vector3 center =
		Vector3(
			(start.x + end.x) * 0.5f,
			(start.y + end.y) * 0.5f,
			(start.z + end.z) * 0.5f
		);

	float angleZ =
		atan2f(
			dy,
			dx
		);

	laser->SetActive(
		Active()
	);

	laser->SetVisible(
		Visible()
	);

	laser->SetPosition(center);

	laser->SetSize(
		Vector3(
			length * 0.5f,
			m_LaserThickness,
			m_LaserThickness
		)
	);

	laser->SetRotation(
		Vector3(
			0.0f,
			0.0f,
			angleZ
		)
	);
}

void LaserWall::ApplyHorizontalLaserBlockTransform(
	StageBlock* laser,
	Vector3 start,
	Vector3 end
)
{
	if (laser == nullptr)
	{
		return;
	}

	float dx =
		end.x -
		start.x;

	float dz =
		end.z -
		start.z;

	float length =
		sqrtf(
			dx * dx +
			dz * dz
		);

	if (length <= 0.001f)
	{
		laser->SetActive(false);
		laser->SetVisible(false);
		return;
	}

	Vector3 center =
		Vector3(
			(start.x + end.x) * 0.5f,
			(start.y + end.y) * 0.5f,
			(start.z + end.z) * 0.5f
		);

	float angleY =
		atan2f(
			-dz,
			dx
		);

	laser->SetActive(
		Active()
	);

	laser->SetVisible(
		Visible()
	);

	laser->SetPosition(center);

	laser->SetSize(
		Vector3(
			length * 0.5f,
			m_LaserDepth,
			m_LaserThickness
		)
	);

	laser->SetRotation(
		Vector3(
			0.0f,
			angleY,
			0.0f
		)
	);
}

void LaserWall::ApplySideLaserBlockTransform(
	StageBlock* laser,
	Vector3 start,
	Vector3 end
)
{
	if (laser == nullptr)
	{
		return;
	}

	float dy =
		end.y -
		start.y;

	float dz =
		end.z -
		start.z;

	float length =
		sqrtf(
			dy * dy +
			dz * dz
		);

	if (length <= 0.001f)
	{
		laser->SetActive(false);
		laser->SetVisible(false);
		return;
	}

	Vector3 center =
		Vector3(
			(start.x + end.x) * 0.5f,
			(start.y + end.y) * 0.5f,
			(start.z + end.z) * 0.5f
		);

	float angleX =
		atan2f(
			dz,
			dy
		);

	laser->SetActive(
		Active()
	);

	laser->SetVisible(
		Visible()
	);

	laser->SetPosition(center);

	laser->SetSize(
		Vector3(
			m_LaserThickness,
			length * 0.5f,
			m_LaserThickness
		)
	);

	laser->SetRotation(
		Vector3(
			angleX,
			0.0f,
			1.57079632679f
		)
	);
}


Vector3 LaserWall::RotateLocalPoint(
	Vector3 point,
	float angle
) const
{
	float c =
		cosf(angle);

	float s =
		sinf(angle);

	Vector3 result;

	result.x =
		point.x * c -
		point.y * s;

	result.y =
		point.x * s +
		point.y * c;

	result.z =
		point.z;

	return result;
}

Vector3 LaserWall::ConvertLocalToWorldPoint(
	Vector3 localPoint
) const
{
	if (m_PlaneType == LaserWallPlaneType::HorizontalXZ)
	{
		return Vector3(
			m_HitCenter.x + localPoint.x,
			m_HitCenter.y,
			m_HitCenter.z + localPoint.y
		);
	}

	if (m_PlaneType == LaserWallPlaneType::SideYZ)
	{
		return Vector3(
			m_HitCenter.x,
			m_HitCenter.y + localPoint.y,
			m_HitCenter.z + localPoint.x
		);
	}

	return Vector3(
		m_HitCenter.x + localPoint.x,
		m_HitCenter.y + localPoint.y,
		m_HitCenter.z + localPoint.z
	);
}


float LaserWall::GetPatternRate()
{
	if (!m_EnablePatternAnimation)
	{
		return 0.0f;
	}

	if (m_PatternMoveTime <= 0.001f)
	{
		return 0.0f;
	}

	if (m_PatternCoolTime < 0.0f)
	{
		m_PatternCoolTime = 0.0f;
	}

	float cycleTime =
		m_PatternMoveTime * 2.0f +
		m_PatternCoolTime * 2.0f;

	float timeInCycle =
		fmodf(
			m_AnimationTime,
			cycleTime
		);

	float rate = 0.0f;

	if (timeInCycle < m_PatternMoveTime)
	{
		rate =
			timeInCycle /
			m_PatternMoveTime;
	}
	else if (timeInCycle < m_PatternMoveTime + m_PatternCoolTime)
	{
		rate = 1.0f;
	}
	else if (timeInCycle < m_PatternMoveTime * 2.0f + m_PatternCoolTime)
	{
		float backTime =
			timeInCycle -
			m_PatternMoveTime -
			m_PatternCoolTime;

		rate =
			1.0f -
			(backTime / m_PatternMoveTime);
	}
	else
	{
		rate = 0.0f;
	}

	rate =
		rate *
		rate *
		(3.0f - 2.0f * rate);

	return rate;
}

void LaserWall::CheckStartAppearAnimationByDistance(
	Vector3 playerPosition
)
{
	if (m_HasStartedAppearAnimation)
	{
		return;
	}

	float dx =
		playerPosition.x -
		m_HitCenter.x;

	float dy =
		playerPosition.y -
		m_HitCenter.y;

	float dz =
		playerPosition.z -
		m_HitCenter.z;

	float distance =
		sqrtf(
			dx * dx +
			dy * dy +
			dz * dz
		);

	if (distance <= m_AppearStartDistance)
	{
		m_HasStartedAppearAnimation = true;
		m_AppearAnimationTime = 0.0f;
	}
}

float LaserWall::GetAppearRate(
	const LaserWallLineData& line
) const
{
	if (!m_UseAppearAnimation)
	{
		return 1.0f;
	}

	if (!m_HasStartedAppearAnimation)
	{
		return 0.0f;
	}

	if (line.appearDuration <= 0.001f)
	{
		return 1.0f;
	}

	float rate =
		(m_AppearAnimationTime - line.appearDelay) /
		line.appearDuration;

	if (rate < 0.0f)
	{
		rate = 0.0f;
	}

	if (rate > 1.0f)
	{
		rate = 1.0f;
	}

	rate =
		rate *
		rate *
		(3.0f - 2.0f * rate);

	return rate;
}

float LaserWall::GetAppearEndTime() const
{
	if (!m_UseAppearAnimation)
	{
		return 0.0f;
	}

	float frameEndTime =
		m_LineAppearInterval * 3.0f +
		m_BaseAppearDuration;

	int gridLineCount =
		m_VerticalLineCount +
		m_HorizontalLineCount;

	if (gridLineCount <= 0)
	{
		return frameEndTime;
	}

	float gridEndTime =
		0.35f +
		static_cast<float>(gridLineCount - 1) *
		m_LineAppearInterval +
		m_BaseAppearDuration;

	if (frameEndTime > gridEndTime)
	{
		return frameEndTime;
	}

	return gridEndTime;
}


// レーザーが矩形内に収まるか調べる
bool LaserWall::ClipTest(
	float p,
	float q,
	float& t0,
	float& t1
) const
{
	if (p == 0.0f)
	{
		return q >= 0.0f;
	}

	float r =
		q /
		p;

	if (p < 0.0f)
	{
		if (r > t1)
		{
			return false;
		}

		if (r > t0)
		{
			t0 = r;
		}
	}
	else
	{
		if (r < t0)
		{
			return false;
		}

		if (r < t1)
		{
			t1 = r;
		}
	}

	return true;
}

bool LaserWall::ClipLineToLocalRect(
	Vector3& start,
	Vector3& end
) const
{
	float left = -m_HitSize.x;
	float right = m_HitSize.x;
	float bottom = -m_HitSize.y;
	float top = m_HitSize.y;

	float dx =
		end.x -
		start.x;

	float dy =
		end.y -
		start.y;

	float t0 = 0.0f;
	float t1 = 1.0f;

	if (!ClipTest(-dx, start.x - left, t0, t1))
	{
		return false;
	}

	if (!ClipTest(dx, right - start.x, t0, t1))
	{
		return false;
	}

	if (!ClipTest(-dy, start.y - bottom, t0, t1))
	{
		return false;
	}

	if (!ClipTest(dy, top - start.y, t0, t1))
	{
		return false;
	}

	Vector3 oldStart = start;
	Vector3 oldEnd = end;

	if (t1 < 1.0f)
	{
		end.x =
			oldStart.x +
			dx * t1;

		end.y =
			oldStart.y +
			dy * t1;

		end.z =
			oldStart.z +
			(oldEnd.z - oldStart.z) *
			t1;
	}

	if (t0 > 0.0f)
	{
		start.x =
			oldStart.x +
			dx * t0;

		start.y =
			oldStart.y +
			dy * t0;

		start.z =
			oldStart.z +
			(oldEnd.z - oldStart.z) *
			t0;
	}

	return true;
}


// 出現率と動きの設定をレーザーに反映する
void LaserWall::ApplyAnimatedLaserLines()
{
	float patternRate =
		GetPatternRate();

	float verticalAngle =
		-m_DiagonalAngle *
		patternRate;

	float horizontalAngle =
		-m_DiagonalAngle *
		patternRate;

	for (LaserWallLineData& line : m_LaserLines)
	{
		if (line.block == nullptr)
		{
			continue;
		}

		Vector3 start;
		Vector3 end;

		if (line.moveType == LaserWallMoveType::Frame)
		{
			start = line.localStart;
			end = line.localEnd;
		}
		else if (line.moveType == LaserWallMoveType::GridLine)
		{
			float angle = 0.0f;

			if (line.gridLineType == LaserWallGridLineType::Vertical)
			{
				angle = verticalAngle;
			}
			else if (line.gridLineType == LaserWallGridLineType::Horizontal)
			{
				angle = horizontalAngle;
			}

			start =
				RotateLocalPoint(
					line.localStart,
					angle
				);

			end =
				RotateLocalPoint(
					line.localEnd,
					angle
				);

			if (!ClipLineToLocalRect(
				start,
				end
			))
			{
				line.block->SetActive(false);
				line.block->SetVisible(false);
				continue;
			}
		}
		else
		{
			start = line.localStart;
			end = line.localEnd;
		}

		float appearRate =
			GetAppearRate(
				line
			);

		Vector3 drawEnd =
			Vector3(
				start.x + (end.x - start.x) * appearRate,
				start.y + (end.y - start.y) * appearRate,
				start.z + (end.z - start.z) * appearRate
			);

		Vector3 worldStart =
			ConvertLocalToWorldPoint(
				start
			);

		Vector3 worldEnd =
			ConvertLocalToWorldPoint(
				drawEnd
			);

		ApplyLaserBlockTransform(
			line.block,
			worldStart,
			worldEnd
		);
	}
}


void LaserWall::AddMoveZ(float move)
{
	m_HitCenter.z -= move;

	ApplyAnimatedLaserLines();
}

bool LaserWall::IsHitPlayer(Player* player)
{
	if (player == nullptr)
	{
		return false;
	}

	return IsHitAABB(
		player->GetPosition(),
		player->GetHitSize()
	);
}

Vector3 LaserWall::GetCurrentHitSize() const
{
	if (m_PlaneType == LaserWallPlaneType::HorizontalXZ)
	{
		return Vector3(
			m_HitSize.x,
			m_HitSize.z,
			m_HitSize.y
		);
	}

	if (m_PlaneType == LaserWallPlaneType::SideYZ)
	{
		return Vector3(
			m_HitSize.z,
			m_HitSize.y,
			m_HitSize.x
		);
	}

	return m_HitSize;
}

bool LaserWall::IsHitAABB(
	Vector3 position,
	Vector3 size
)
{
	if (!Active())
	{
		return false;
	}

	if (m_UseAppearAnimation)
	{
		if (!m_HasStartedAppearAnimation)
		{
			return false;
		}

		if (m_AppearAnimationTime < GetAppearEndTime())
		{
			return false;
		}
	}

	return CheckAABB(
		m_HitCenter,
		GetCurrentHitSize(),
		position,
		size
	);
}


bool LaserWall::CheckAABB(
	Vector3 centerA,
	Vector3 sizeA,
	Vector3 centerB,
	Vector3 sizeB
) const
{
	Vector3 minA =
		Vector3(
			centerA.x - sizeA.x,
			centerA.y - sizeA.y,
			centerA.z - sizeA.z
		);

	Vector3 maxA =
		Vector3(
			centerA.x + sizeA.x,
			centerA.y + sizeA.y,
			centerA.z + sizeA.z
		);

	Vector3 minB =
		Vector3(
			centerB.x - sizeB.x,
			centerB.y - sizeB.y,
			centerB.z - sizeB.z
		);

	Vector3 maxB =
		Vector3(
			centerB.x + sizeB.x,
			centerB.y + sizeB.y,
			centerB.z + sizeB.z
		);

	if (maxA.x <= minB.x)
	{
		return false;
	}

	if (minA.x >= maxB.x)
	{
		return false;
	}

	if (maxA.y <= minB.y)
	{
		return false;
	}

	if (minA.y >= maxB.y)
	{
		return false;
	}

	if (maxA.z <= minB.z)
	{
		return false;
	}

	if (minA.z >= maxB.z)
	{
		return false;
	}

	return true;
}

void LaserWall::SetHitAABB(
	Vector3 center,
	Vector3 size
)
{
	m_HitCenter = center;
	m_HitSize = size;

	if (m_HitSize.x <= 0.001f)
	{
		m_HitSize.x = 0.001f;
	}

	if (m_HitSize.y <= 0.001f)
	{
		m_HitSize.y = 0.001f;
	}

	if (m_HitSize.z <= 0.001f)
	{
		m_HitSize.z = 0.001f;
	}

	ApplyAnimatedLaserLines();
}

Vector3 LaserWall::GetHitCenter() const
{
	return m_HitCenter;
}

Vector3 LaserWall::GetHitSize() const
{
	return m_HitSize;
}


void LaserWall::SetActive(bool isActive)
{
	GameObject::SetActive(
		isActive
	);

	for (LaserWallLineData& line : m_LaserLines)
	{
		if (line.block == nullptr)
		{
			continue;
		}

		line.block->SetActive(
			isActive
		);

		line.block->SetVisible(
			isActive
		);
	}
}

bool LaserWall::IsActive() const
{
	return Active();
}

void LaserWall::SetTexturePath(
	const std::string& texturePath
)
{
	m_TexturePath =
		texturePath;

	for (LaserWallLineData& line : m_LaserLines)
	{
		if (line.block == nullptr)
		{
			continue;
		}

		line.block->SetTexture(
			m_TexturePath.c_str()
		);
	}
}

void LaserWall::SetAlpha(float alpha)
{
	m_ColorA = alpha;

	for (LaserWallLineData& line : m_LaserLines)
	{
		if (line.block == nullptr)
		{
			continue;
		}

		line.block->SetAlpha(
			alpha
		);
	}
}

void LaserWall::SetColor(
	float r,
	float g,
	float b,
	float a
)
{
	m_ColorR = r;
	m_ColorG = g;
	m_ColorB = b;
	m_ColorA = a;

	for (LaserWallLineData& line : m_LaserLines)
	{
		if (line.block == nullptr)
		{
			continue;
		}

		line.block->SetColor(
			r,
			g,
			b,
			a
		);

		line.block->SetAlpha(
			a
		);
	}
}


void LaserWall::SetLaserThickness(
	float thickness
)
{
	if (thickness <= 0.001f)
	{
		thickness = 0.001f;
	}

	m_LaserThickness =
		thickness;

	ApplyAnimatedLaserLines();
}

void LaserWall::SetLaserDepth(
	float depth
)
{
	if (depth <= 0.001f)
	{
		depth = 0.001f;
	}

	m_LaserDepth =
		depth;

	ApplyAnimatedLaserLines();
}

void LaserWall::SetGridLineCount(
	int verticalCount,
	int horizontalCount
)
{
	if (verticalCount < 0)
	{
		verticalCount = 0;
	}

	if (horizontalCount < 0)
	{
		horizontalCount = 0;
	}

	m_VerticalLineCount =
		verticalCount;

	m_HorizontalLineCount =
		horizontalCount;
}


void LaserWall::SetStartAnimation(
	bool useStartAnimation,
	float appearStartDistance
)
{
	m_UseAppearAnimation =
		useStartAnimation;

	if (appearStartDistance < 0.0f)
	{
		appearStartDistance = 0.0f;
	}

	m_AppearStartDistance =
		appearStartDistance;

	if (m_UseAppearAnimation)
	{
		m_HasStartedAppearAnimation = false;
		m_AppearAnimationTime = 0.0f;
	}
	else
	{
		m_HasStartedAppearAnimation = true;
		m_AppearAnimationTime = 999.0f;
	}
}

void LaserWall::SetAppearAnimation(
	bool useAppearAnimation,
	float baseAppearDuration,
	float lineAppearInterval
)
{
	m_UseAppearAnimation =
		useAppearAnimation;

	if (baseAppearDuration <= 0.001f)
	{
		baseAppearDuration = 0.001f;
	}

	if (lineAppearInterval < 0.0f)
	{
		lineAppearInterval = 0.0f;
	}

	m_BaseAppearDuration =
		baseAppearDuration;

	m_LineAppearInterval =
		lineAppearInterval;

	if (m_UseAppearAnimation)
	{
		m_HasStartedAppearAnimation = false;
		m_AppearAnimationTime = 0.0f;
	}
	else
	{
		m_HasStartedAppearAnimation = true;
		m_AppearAnimationTime = 999.0f;
	}
}

void LaserWall::SetPatternAnimation(
	bool enablePatternAnimation
)
{
	m_EnablePatternAnimation =
		enablePatternAnimation;
}

void LaserWall::SetPatternTiming(
	float patternMoveTime,
	float patternCoolTime
)
{
	if (patternMoveTime <= 0.001f)
	{
		patternMoveTime = 0.001f;
	}

	if (patternCoolTime < 0.0f)
	{
		patternCoolTime = 0.0f;
	}

	m_PatternMoveTime =
		patternMoveTime;

	m_PatternCoolTime =
		patternCoolTime;
}

void LaserWall::SetDiagonalAngle(
	float diagonalAngle
)
{
	m_DiagonalAngle =
		diagonalAngle;
}


// 接触したプレイヤーにペナルティを与える
void LaserWall::ApplyEffectToPlayer(
	Player* player
)
{
	if (player == nullptr)
	{
		return;
	}

	if (!IsHitPlayer(player))
	{
		return;
	}

	if (m_UseTimePenaltyEffect)
	{
		ApplyTimePenalty();
	}
}

void LaserWall::ApplyTimePenalty()
{
	if (m_TimePenaltyCurrentCooldownFrame > 0)
	{
		return;
	}

	Scene* scene =
		Manager::GetScene();

	if (scene == nullptr)
	{
		return;
	}

	Time* time =
		scene->GetGameObject<Time>();

	if (time == nullptr)
	{
		return;
	}

	time->ReduceLimitTime(
		m_TimePenalty
	);

	m_TimePenaltyCurrentCooldownFrame =
		m_TimePenaltyCooldownFrame;
}

void LaserWall::SetTimePenaltyEffect(
	bool useTimePenaltyEffect,
	float timePenalty,
	int cooldownFrame
)
{
	m_UseTimePenaltyEffect =
		useTimePenaltyEffect;

	m_TimePenalty =
		timePenalty;

	if (m_TimePenalty < 0.0f)
	{
		m_TimePenalty = 0.0f;
	}

	m_TimePenaltyCooldownFrame =
		cooldownFrame;

	if (m_TimePenaltyCooldownFrame < 1)
	{
		m_TimePenaltyCooldownFrame = 1;
	}

	m_TimePenaltyCurrentCooldownFrame = 0;
}

void LaserWall::SetSlowEffect(
	bool useSlowEffect,
	float slowPenalty,
	int slowFrame
)
{
	(void)slowPenalty;

	m_UseTimePenaltyEffect =
		useSlowEffect;

	m_TimePenalty =
		2.0f;

	m_TimePenaltyCooldownFrame =
		slowFrame;

	if (m_TimePenaltyCooldownFrame < 1)
	{
		m_TimePenaltyCooldownFrame = 1;
	}

	m_TimePenaltyCurrentCooldownFrame = 0;
}

void LaserWall::SetPlaneType(LaserWallPlaneType planeType)
{
	m_PlaneType =
		planeType;

	ApplyAnimatedLaserLines();
}


Player* LaserWall::GetPlayer() const
{
	Scene* scene =
		Manager::GetScene();

	if (scene == nullptr)
	{
		return nullptr;
	}

	return scene->GetGameObject<Player>();
}