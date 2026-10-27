#pragma once

#include "gameObject3D.h"
#include "renderer.h"
#include "modelRenderer.h"
#include <algorithm>
#include <cmath>
#include "arenaBounds.h"

// ステージ3で、2人の間に置く壁（見た目と当たり判定をまとめる）
class StageObstacle : public GameObject3D
{
	// 編集中や、別のステージでは壁を表示・判定しない
	bool m_Visible = true;

	// 外周の壁はFieldが描き、ここでは当たり判定を受け持つ
	bool m_ArenaBoundary = false;

	// 箱のモデルと、描画に使うシェーダ
	ModelRenderer *m_Model = nullptr;

	ID3D11InputLayout *m_Layout = nullptr;

	ID3D11VertexShader *m_VertexShader = nullptr;

	ID3D11PixelShader *m_PixelShader = nullptr;

  public:
	// 床の四方に合わせて壁の当たり判定を配置する
	// 壁の位置・大きさと、箱の見た目を用意する
	void Init() override
	{
		m_Position = Vector3(0, 1.5f, 2);

		m_Scale = Vector3(0.8f, 0.65f, 0.45f);

		m_Model = new ModelRenderer();

		m_Model->Load("asset\\model\\box.obj");

		Renderer::CreateVertexShader(&m_VertexShader, &m_Layout, "shader\\unlitTextureVS.cso");

		Renderer::CreatePixelShader(&m_PixelShader, "shader\\unlitTexturePS.cso");
	}

	// 壁を解放する
	void Uninit() override
	{
		delete m_Model;

		m_Layout->Release();

		m_VertexShader->Release();

		m_PixelShader->Release();
	}

	// 壁は移動しない
	void Update() override
	{
	}

	// プレイヤーと区別できる灰色の箱を描く
	void Draw() override
	{
		if (!m_Visible || m_ArenaBoundary)
		{
			return;
		}

		Renderer::GetDeviceContext()->IASetInputLayout(m_Layout);

		Renderer::GetDeviceContext()->VSSetShader(m_VertexShader, nullptr, 0);

		Renderer::GetDeviceContext()->PSSetShader(m_PixelShader, nullptr, 0);

		Renderer::SetWorldMatrix(XMMatrixScaling(m_Scale.x, m_Scale.y, m_Scale.z) * XMMatrixTranslation(m_Position.x, m_Position.y, m_Position.z));

		m_Model->DrawTinted(XMFLOAT4(0.65f, 0.65f, 0.7f, 1));
	}

	// 移動や攻撃の線が壁に当たるか調べる（高速移動でも通り抜けない）
	bool BlocksPath(const Vector3 &from, const Vector3 &to, float padding = 0, float *hitFraction = nullptr) const
	{
		float first = 0;

		float last = 1;

		const float origin[] = {from.x, from.y, from.z};

		const float delta[] = {to.x - from.x, to.y - from.y, to.z - from.z};

		const float center[] = {m_Position.x, m_Position.y, m_Position.z};

		const float extent[] = {m_Scale.x + padding, m_Scale.y, m_Scale.z + padding};

		for (int axis = 0; axis < 3; ++axis)
		{
			if (std::fabs(delta[axis]) < 0.000001f)
			{
				if (origin[axis] < center[axis] - extent[axis] || origin[axis] > center[axis] + extent[axis])
				{
					return false;
				}
			}
			else
			{
				float entry = (center[axis] - extent[axis] - origin[axis]) / delta[axis];

				float exit = (center[axis] + extent[axis] - origin[axis]) / delta[axis];

				if (entry > exit)
				{
					std::swap(entry, exit);
				}

				first = (std::max)(first, entry);

				last = (std::min)(last, exit);

				if (first > last)
				{
					return false;
				}
			}
		}

		if (hitFraction)
		{
			*hitFraction = first;
		}

		return true;
	}

	// 壁に当たる移動を止め、空いている方向へは滑って動けるようにする
	Vector3 ResolveMovement(const Vector3 &from, const Vector3 &to) const
	{
		// 外周では縁に沿って動けるようにし、接触後も内側へ戻れるようにする
		if (m_ArenaBoundary)
		{
			return ArenaBounds::Constrain(to);
		}

		if (!BlocksPath(from, to, 0.3f))
		{
			return to;
		}

		Vector3 result = from;

		Vector3 alongX(to.x, to.y, from.z);

		if (!BlocksPath(result, alongX, 0.3f))
		{
			result = alongX;
		}

		Vector3 alongZ(result.x, to.y, to.z);

		if (!BlocksPath(result, alongZ, 0.3f))
		{
			result = alongZ;
		}

		return result;
	}

	void SetArenaSide(int side)
	{
		m_ArenaBoundary = true;
		m_Position = ArenaBounds::WallPosition(side);
		m_Scale = ArenaBounds::WallScale(side);
	}

	bool IsArenaBoundary() const
	{
		return m_ArenaBoundary;
	}

	// 壁の表示と当たり判定を切り替える
	void SetVisible(bool visible)
	{
		m_Visible = visible;
	}

	// 今のシーンで壁を使うか確認する
	bool GetVisible() const
	{
		return m_Visible;
	}
};
