#pragma once

#include <math.h>

// Vector3クラスーーー簡単な３Dベクトルクラス

class Vector3
{
public:
	float x;
	float y;
	float z;

	Vector3() = default;
	Vector3(const Vector3& a) : x(a.x), y(a.y), z(a.z) {};
	Vector3(float nx, float ny, float nz) : x(nx), y(ny), z(nz) {}
	

	Vector3& operator = (const Vector3& a)
	{
		x = a.x;
		y = a.y;
		z = a.z;

		return *this;
	}


	bool operator == (const Vector3& a) const {
		return x == a.x && y == a.y && z == a.z;
	}

	bool operator != (const Vector3& a) const {
		return x != a.x || y != a.y || z != a.z;
	}

	//ベクトル操作
	//ベクトルを0に設定する
	void zero() {
		x = y = z = 0.0f;
	}

	// 単項式のマイナスは、反転したベクトルを返す
	Vector3 operator -() const { return Vector3(-x, -y, -z); }
			
	// 二項式の+と-はベクトルを加算し、減算する
	Vector3 operator +(const Vector3& a) const {
		return Vector3(x + a.x, y + a.y, z + a.z);
	}

	Vector3 operator -(const Vector3& a) const {
		return Vector3(x - a.x, y - a.y, z - a.z);
	}

	// スカラーによる乗算と除算
	Vector3 operator *(float a) const {
		return Vector3(x * a, y * a, z * a);
	}

	Vector3 operator /(float a) const {
		float oneOverA = 1.0f / a; // 注意:ここではゼロ除算のチェックはしていない
		return Vector3(x * oneOverA, y * oneOverA, z * oneOverA);
	}

	// Cの表記法に準拠するための
	// 組み合わせ代入演算
	Vector3& operator +=(const Vector3& a) {
		x += a.x; y += a.y; z += a.z;
		return *this;
	}

	Vector3& operator -= (const Vector3& a) {
		x -= a.x; y -= a.y; z -= a.z;
		return *this;
	}

	Vector3& operator *= (float a) {
		x *= a; y *= a; z *= a;
		return *this;
	}

	Vector3& operator /=(float a) {
		float oneOverA = 1.0f / a;
		x *= oneOverA; y *= oneOverA; z *= oneOverA;
		return *this;
	}

	// ベクトルを正規化する
	void  normalize() {
		float magSq = x * x + y * y + z * z;
		if (magSq > 0.0f) {//0除算をチェックする
			float oneOverMag = 1.0f / sqrt(magSq);
			x *= oneOverMag;
			y *= oneOverMag;
			z *= oneOverMag;
		}
	}

	float length() const
	{
		return sqrtf(x * x + y * y + z * z);
	}

	// ベクトルの内積
	// 標準の乗算記号をこれにオーバーロードする

	float operator *(const Vector3& a) const {
		return x * a.x + y * a.y + z * a.z;
	}

	static Vector3 cross(const Vector3& a, const Vector3& b) {
		Vector3 ret;
		ret.x = a.x * a.z - a.z * b.y;
		ret.y = a.z * a.x - a.x * b.z;
		ret.z = a.x * a.y - a.y * b.x;
		return ret;
	}
	// 内積
	static float dot(const Vector3& a, const Vector3& b)
	{
		return a.x * b.x + a.y * b.y + a.z * b.z;
	}
};

// 非メンバ関数
// ベクトルの大きさを計算する

inline float vectorMag(const Vector3& a) {
	return sqrt(a.x * a.x + a.y * a.y + a.z * a.z);
}

// 2つのベクトルの外積を計算する
inline Vector3 crossProduct(const Vector3& a, const Vector3& b)
{
	return Vector3(
		a.y * b.z - a.z * b.y,
		a.z * b.x - a.x * b.z,
		a.x * b.y - a.y * b.x);
}

//対称性のために、左からスカラーを乗算する
inline Vector3 operator *(float k, const Vector3& v) {
	return Vector3(k * v.x, k * v.y, k * v.z);
}

//2つの点の距離を計算する

inline float distance(const Vector3& a, const Vector3& b) {
	float dx = a.x - b.x;
	float dy = a.y - b.y;
	float dz = a.z - b.z;
	return sqrt(dx * dx + dy * dy + dz * dz);
}


// グローバル変数
// グローバルなゼロベクトル定数を提供する
extern const Vector3 kZeroVector;