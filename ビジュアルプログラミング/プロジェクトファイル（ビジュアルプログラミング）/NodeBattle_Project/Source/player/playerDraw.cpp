#include "main.h"
#include "player.h"
#include "playerDraw.h"
#include "renderer.h"
#include "modelRenderer.h"
#include "manager.h"
#include "stageObstacle.h"
// 操作するプレイヤーを受け取る
PlayerDraw::PlayerDraw(Player &player)
	: m_Player(player)
{
}

// 描画に使うモデルとシェーダーを用意する
void PlayerDraw::Init()
{
	m_ModelRenderer = new ModelRenderer();

	m_ModelRenderer->Load("asset\\model\\box.obj");

	Renderer::CreateVertexShader(&m_VertexShader, &m_VertexLayout,
								 "shader\\unlitTextureVS.cso");

	Renderer::CreatePixelShader(&m_PixelShader,
								"shader\\unlitTexturePS.cso");
}

// 作成した描画リソースを解放する
void PlayerDraw::Uninit()
{
	delete m_ModelRenderer;
	m_ModelRenderer = nullptr;

	if (m_VertexLayout)
	{
		m_VertexLayout->Release();
		m_VertexLayout = nullptr;
	}

	if (m_VertexShader)
	{
		m_VertexShader->Release();
		m_VertexShader = nullptr;
	}

	if (m_PixelShader)
	{
		m_PixelShader->Release();
		m_PixelShader = nullptr;
	}
}

// 画面に描く処理
void PlayerDraw::Draw()
{
	// 入力レイアウト設定
	Renderer::GetDeviceContext()->IASetInputLayout(m_VertexLayout);

	// シェーダ設定
	Renderer::GetDeviceContext()->VSSetShader(m_VertexShader, NULL, 0);

	Renderer::GetDeviceContext()->PSSetShader(m_PixelShader, NULL, 0);

	// 3Dマトリクス設定
	XMMATRIX world, scale, rot, trans;

	scale = XMMatrixScaling(m_Player.GetScale().x, m_Player.GetScale().y, m_Player.GetScale().z);

	rot = XMMatrixRotationRollPitchYaw(m_Player.GetRotation().x, m_Player.GetRotation().y , m_Player.GetRotation().z);

	trans = XMMatrixTranslation(m_Player.GetPosition().x, m_Player.GetPosition().y, m_Player.GetPosition().z);

	world = scale * rot * trans;

	Renderer::SetWorldMatrix(world);

	// 自分は赤、敵は青にして、練習でも対戦でも区別する
	XMFLOAT4 playerColor;

	if (m_Player.GetTeam() == Team::TeamA)
	{
		playerColor = XMFLOAT4(1, 0.2f, 0.15f, 1);
	}
	else
	{
		playerColor = XMFLOAT4(0.15f, 0.4f, 1, 1);
	}

	m_ModelRenderer->DrawTinted(playerColor);

	// 射撃の構えを黄色い銃身で示し、横へ避ける合図にする
	if (m_Player.GetAttackController().IsRangePending())
	{
		Vector3 forward(std::sin(m_Player.GetAttackController().GetRangeHeading()), 0, std::cos(m_Player.GetAttackController().GetRangeHeading()));

		Vector3 center = m_Player.GetAttackController().GetRangeOrigin() + forward * 0.5f + Vector3(0, 0.25f, 0);

		world = XMMatrixScaling(0.12f, 0.12f, 0.3f) * XMMatrixRotationY(m_Player.GetAttackController().GetRangeHeading()) * XMMatrixTranslation(center.x, center.y, center.z);

		Renderer::SetWorldMatrix(world);

		m_ModelRenderer->DrawTinted(XMFLOAT4(1, 0.9f, 0.1f, 1));
	}

	// 盾を構えている間は、体の正面に板を表示する
	if (m_Player.IsGuarding())
	{
		Vector3 center = m_Player.GetPosition() + m_Player.GetForward() * 0.55f + Vector3(0, 0.2f, 0);

		world = XMMatrixScaling(0.45f, 0.5f, 0.08f) * XMMatrixRotationY(m_Player.GetRotation().y) * XMMatrixTranslation(center.x, center.y, center.z);

		Renderer::SetWorldMatrix(world);

		m_ModelRenderer->DrawTinted(XMFLOAT4(0.55f, 0.7f, 0.8f, 1));
	}

	if (m_Player.GetAttackController().IsSwordEnabled() || m_Player.GetAttackController().IsMeleePending())
	{

		// 槍の軸と長さ（攻撃中は開始時の向きを使う）
		float heading = m_Player.GetMeleeSwingHeading();

		// 武器の向きから前方向のベクトルを作成する
		Vector3 forward(std::sin(heading), 0, std::cos(heading));

		Vector3 origin;

		if (m_Player.GetAttackController().IsMeleePending())
		{
			origin = m_Player.GetAttackController().GetMeleeOrigin();
		}
		else
		{
			origin = m_Player.GetPosition();
		}

		// 持ち手を少し右に寄せて、その向きの正面へ突く
		Vector3 right(std::cos(heading), 0, -std::sin(heading));

		origin += right * 0.22f;

		float length;

		if (m_Player.GetAttackController().IsMeleePending())
		{
			length = m_Player.GetMeleeExtension();
		}
		else
		{
			length = 1.2f;
		}

		// 槍の表示も壁に当たったところまでにして、突き抜けて見えないようにする
		if (Manager::GetScene())
		{
			Vector3 start = origin + forward * 0.35f;

			for (auto *wall : Manager::GetScene()->GetGameObjects<StageObstacle>())
			{
				if (!wall->GetVisible())
				{
					continue;
				}

				float fraction = 1;

				if (wall->BlocksPath(start, start + forward * length, 0, &fraction))
				{
					length *= fraction;
				}
			}
		}

		// 攻撃中と通常時で、武器の高さと色を切り替える
		float weaponHeight;

		if (m_Player.GetAttackController().IsMeleePending())
		{
			weaponHeight = 0.35f;
		}
		else
		{
			weaponHeight = 0.25f;
		}

		Vector3 center = origin + forward * (0.35f + length * 0.5f) + Vector3(0, weaponHeight, 0);

		// 剣は幅のある短い刃、槍は細長い軸として表示する
		float width;

		if (m_Player.GetAttackController().GetMeleeType() == NodeType::Attack_Sword)
		{
			width = 0.13f;
		}
		else
		{
			width = 0.07f;
		}

		world = XMMatrixScaling(width, 0.07f, length * 0.5f) * XMMatrixRotationY(heading) * XMMatrixTranslation(center.x, center.y, center.z);

		Renderer::SetWorldMatrix(world);

		XMFLOAT4 weaponColor;

		if (m_Player.GetAttackController().IsMeleePending())
		{
			weaponColor = XMFLOAT4(1, 0.65f, 0.15f, 1);
		}
		else
		{
			weaponColor = XMFLOAT4(0.6f, 0.8f, 1, 1);
		}

		m_ModelRenderer->DrawTinted(weaponColor);

		Vector3 guard = origin + forward * 0.4f + Vector3(0, 0.25f, 0);

		world = XMMatrixScaling(0.25f, 0.08f, 0.07f) * XMMatrixRotationY(heading) * XMMatrixTranslation(guard.x, guard.y, guard.z);

		Renderer::SetWorldMatrix(world);

		m_ModelRenderer->DrawTinted(XMFLOAT4(0.8f, 0.55f, 0.15f, 1));
	}
}
