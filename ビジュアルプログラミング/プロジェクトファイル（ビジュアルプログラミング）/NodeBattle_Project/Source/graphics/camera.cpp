#include "main.h"
#include "renderer.h"
#include "camera.h"
#include "player.h"
#include "manager.h"
#include "scene.h"
#include "input.h"
#include <algorithm>

// 初期化処理を行う
void Camera::Init()
{
	m_Position = Vector3(0, 10, 0);

	m_Target = Vector3(0, 0, 0);
}

// 終了処理を行う
void Camera::Uninit()
{
}

// 毎フレームの処理
void Camera::Update()
{
	if (Input::GetKeyTrigger(VK_F1))
	{
		SetViewNumber(1);
	}

	if (Input::GetKeyTrigger(VK_F2))
	{
		SetViewNumber(2);
	}

	if (Input::GetKeyTrigger(VK_F3))
	{
		SetViewNumber(3);
	}

	auto *scene = Manager::GetScene();

	if (!scene)
	{
		return;
	}

	auto *player = scene->GetGameObject<Player>(L"player");

	if (!player)
	{
		return;
	}

	Vector3 position = player->GetPosition();

	auto *enemy = player->GetTargetObject();
	// テストで敵の表示フラグを切っても、両キャラクターが画面に収まる視点を保つ。

	if (!enemy)
	{
		enemy = scene->GetGameObject<Player>(L"testTarget");
	}

	if (!enemy)
	{
		enemy = scene->GetGameObject<Player>(L"enemy");
	}

	// 両方のプレイヤーが入るように、真ん中を注視点にする
	Vector3 center;

	if (enemy)
	{
		center = (position + enemy->GetPosition()) * 0.5f;
	}
	else
	{
		center = position;
	}

	float separation;

	if (enemy)
	{
		separation = (position - enemy->GetPosition()).length();
	}
	else
	{
		separation = 0;
	}

	if (m_ViewNumber == 1)
	{
		Vector3 forward = player->GetForward();

		forward.y = 0;

		forward.normalize();

		m_Target = position + Vector3(0, 0.3f, 0);

		m_Position = m_Target - forward * 6 + Vector3(0, 3, 0);
	}
	else if (m_ViewNumber == 2)
	{
		m_Target = center + Vector3(0, 0.3f, 0);

		float distance = (std::max)(7.0f, separation * 1.2f);

		m_Position = m_Target + Vector3(distance, distance * 0.5f, 0);
	}
	else
	{
		m_Target = center + Vector3(0, 0.3f, 0);

		m_Position = m_Target + Vector3(0, (std::max)(8.0f, 7.0f + separation), 0);
	}
}

// 画面に描く処理
void Camera::Draw()
{
	m_Projection = XMMatrixPerspectiveFovLH(1.0f, (float)SCREEN_WIDTH / (float)SCREEN_HEIGHT, 0.1f, 1000.0f);

	Renderer::SetProjectionMatrix(m_Projection);

	// 真上のカメラでも、上下の向きが決まるようにする
	XMFLOAT3 up;

	if (m_ViewNumber == 3)
	{
		up = XMFLOAT3(0, 0, 1);
	}
	else
	{
		up = XMFLOAT3(0, 1, 0);
	}

	m_View = XMMatrixLookAtLH(XMLoadFloat3((XMFLOAT3 *)&m_Position), XMLoadFloat3((XMFLOAT3 *)&m_Target), XMLoadFloat3(&up));

	Renderer::SetViewMatrix(m_View);
}
