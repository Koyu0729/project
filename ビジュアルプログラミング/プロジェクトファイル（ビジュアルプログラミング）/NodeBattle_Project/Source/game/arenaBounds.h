#pragma once

#include "vector3.h"
#include <algorithm>

// 床・四方の壁・移動制限で共通に使うフィールドの寸法
namespace ArenaBounds
{
	// 読み込んだ座標は、この範囲までを正常値として扱う
	constexpr float CoordinateLimit = 10000.0f;

	constexpr float HalfSize = 10.0f;

	constexpr float CenterZ = 2.0f;

	constexpr float WallHalfWidth = 0.35f;

	constexpr float WallHeight = 2.5f;

	constexpr float PlayerRadius = 0.3f;

	// 壁の内側から体がはみ出さない位置に戻す（異常な座標も回復する）
	inline Vector3 Constrain(const Vector3 &position)
	{
		const float limit = HalfSize - PlayerRadius;

		// 座標が許容範囲内か確認して、壁の内側に収める
		float boundedX;

		if (position.x >= -CoordinateLimit && position.x <= CoordinateLimit)
		{
			boundedX = (std::max)(-limit, (std::min)(limit, position.x));
		}
		else
		{
			boundedX = 0;
		}

		float boundedY;

		if (position.y >= -CoordinateLimit && position.y <= CoordinateLimit)
		{
			boundedY = position.y;
		}
		else
		{
			boundedY = 1.5f;
		}

		float boundedZ;

		if (position.z >= -CoordinateLimit && position.z <= CoordinateLimit)
		{
			boundedZ = (std::max)(CenterZ - limit, (std::min)(CenterZ + limit, position.z));
		}
		else
		{
			boundedZ = CenterZ;
		}

		return Vector3(boundedX,
					   boundedY,
					   boundedZ);
	}

	// 左・右・手前・奥の壁を床の縁に配置する
	inline Vector3 WallPosition(int side)
	{
		const float edge = HalfSize + WallHalfWidth;

		// 左右の壁はX方向、手前と奥の壁はZ方向へ移動する
		float wallX;

		if (side < 2)
		{
			if (side == 0)
			{
				wallX = -edge;
			}
			else
			{
				wallX = edge;
			}
		}
		else
		{
			wallX = 0;
		}

		float wallOffsetZ;

		if (side < 2)
		{
			wallOffsetZ = 0;
		}
		else if (side == 2)
		{
			wallOffsetZ = -edge;
		}
		else
		{
			wallOffsetZ = edge;
		}

		return Vector3(wallX, WallHeight * 0.5f,
					   CenterZ + wallOffsetZ);
	}

	// 壁の当たり判定と見た目の大きさをそろえる
	inline Vector3 WallScale(int side)
	{
		if (side < 2)
		{
			return Vector3(WallHalfWidth, WallHeight * 0.5f, HalfSize + WallHalfWidth * 2);
		}
		else
		{
			return Vector3(HalfSize, WallHeight * 0.5f, WallHalfWidth);
		}
	}
}
