#include "main.h"
#include "field.h"
#include "arenaBounds.h"

// 草の床1個と石の壁4個を配置する
void Field::Init()
{
	m_Cubes[0].SetPosition(Vector3(0, 0, ArenaBounds::CenterZ));
	m_Cubes[0].SetScale(Vector3(ArenaBounds::HalfSize, 0.1f, ArenaBounds::HalfSize));
	m_Cubes[0].SetTexture("asset\\texture\\grass.jpg");
	m_Cubes[0].SetTextureRepeat(Vector3(5, 1, 5));
	m_Cubes[0].SetColor(XMFLOAT4(0.65f, 0.7f, 0.65f, 1));
	m_Cubes[0].Init();

	// 壁の見た目を、移動制限で使うフィールドの大きさに合わせる
	for (int side = 0; side < 4; ++side)
	{
		Vector3 wallScale = ArenaBounds::WallScale(side);

		m_Cubes[side + 1].SetPosition(ArenaBounds::WallPosition(side));
		m_Cubes[side + 1].SetScale(wallScale);
		m_Cubes[side + 1].SetTexture("asset\\texture\\wall.jpg");
		m_Cubes[side + 1].SetTextureRepeat(wallScale);
		m_Cubes[side + 1].SetColor(XMFLOAT4(0.65f, 0.65f, 0.7f, 1));
		m_Cubes[side + 1].Init();
	}
}

// 配置した床と壁を解放する
void Field::Uninit()
{
	for (int index = 0; index < 5; ++index)
	{
		m_Cubes[index].Uninit();
	}
}

// 床と壁は動かさない
void Field::Update()
{
}

// 配置した床と壁を描く
void Field::Draw()
{
	if (!m_Visible)
	{
		return;
	}

	for (int index = 0; index < 5; ++index)
	{
		m_Cubes[index].Draw();
	}
}
