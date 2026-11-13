#include "main.h"
#include "renderer.h"
#include "bullet.h"
#include "modelRenderer.h"
#include "input.h"
#include "camera.h"
#include "manager.h"
#include "scene.h"
#include "player.h"
#include "nodeRuntime.h"
#include "explosion.h"
#include "stageObstacle.h"
#include "billboardSprite.h"

// プレイヤーの移動量の変数
constexpr float MOVE_SPEED = 0.1f;

// 弾の見た目とシェーダを用意する
void Bullet::Init()
{
	m_Sprite = new BillboardSprite();
	m_Sprite->Init("asset\\texture\\particle.png");
}

// 弾の描画リソースを解放する
void Bullet::Uninit()
{
	m_Sprite->Uninit();
	delete m_Sprite;
	m_Sprite = nullptr;
}

// 弾を進めて、壁や相手に当たったか調べる
void Bullet::Update()
{
	if (m_Owner && m_Owner->GetNodeRuntime() && m_Owner->GetNodeRuntime()->GetState() == RuntimeState::Paused)
	{
		return;
	}
	// 移動
	m_Lifetime += GetFrameDeltaSeconds();

	if (m_Lifetime >= 4.0f)
	{
		SetDestroy();
		return;
	}
	Vector3 previous = m_Position;

	m_Position += m_Velocity * (GetFrameDeltaSeconds() * 60.0f);

	// 弾は移動した線全体で壁を調べ、壁に当たったら消す
	if (Manager::GetScene())
	{
		for (auto *wall : Manager::GetScene()->GetGameObjects<StageObstacle>())
		{
			if (wall->GetVisible() && wall->BlocksPath(previous, m_Position))
			{
				SetDestroy();

				return;
			}
		}
	}

	if (!m_Owner)
	{
		return;
	}

	GameObject3D *target = m_Owner->GetTargetObject();

	if (!target)
	{
		return;
	}

	// フレーム間に通った線で体に当たったか調べ、速い弾の通り抜けを防ぐ
	Vector3 movement = m_Position - previous;

	float lengthSquared = Vector3::dot(movement, movement);

	float fraction;

	if (lengthSquared > 0.000001f)
	{
		fraction = (std::max)(0.0f, (std::min)(1.0f, Vector3::dot(target->GetPosition() - previous, movement) / lengthSquared));
	}
	else
	{
		fraction = 0;
	}

	Vector3 nearest = previous + movement * fraction;

	if ((nearest - target->GetPosition()).length() <= CombatBalance::BulletRadius)
	{
		Player *targetPlayer = dynamic_cast<Player *>(target);

		if (targetPlayer && targetPlayer != m_Owner && (m_Owner->GetTeam() == Team::Team_Null || targetPlayer->GetTeam() != m_Owner->GetTeam()))
		{
			targetPlayer->TakeDamage(CombatBalance::BulletDamage, &previous);

			auto *effect = Manager::GetScene()->AddGameObject3D<Explosion>(1);

			effect->SetPosition(nearest + Vector3(0, 0.2f, 0));
			effect->SetOwner(m_Owner);

			SetDestroy();
		}
	}
}

// 弾を小さな発光画像として描き、どのカメラでも丸く見せる
void Bullet::Draw()
{
	Camera *camera;

	if (Manager::GetScene())
	{
		camera = Manager::GetScene()->GetGameObject<Camera>();
	}
	else
	{
		camera = nullptr;
	}

	XMFLOAT4 colorValue;

	if (m_Owner && m_Owner->GetTeam() == Team::TeamB)
	{
		colorValue = XMFLOAT4(0.3f, 0.75f, 1, 1);
	}
	else
	{
		colorValue = XMFLOAT4(1, 0.65f, 0.2f, 1);
	}

	const XMFLOAT4 color = colorValue;

	m_Sprite->Draw(camera, m_Position, 0.16f, color);
}
