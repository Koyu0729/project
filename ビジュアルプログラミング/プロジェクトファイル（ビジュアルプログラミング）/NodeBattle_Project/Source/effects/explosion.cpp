#include "main.h"
#include "explosion.h"
#include "billboardSprite.h"
#include "manager.h"
#include "scene.h"
#include "player.h"
#include "nodeRuntime.h"

void Explosion::Init()
{
	m_Sprite = new BillboardSprite();
	m_Sprite->Init("asset\\texture\\Explosion.png");
	m_Elapsed = 0;
	m_Frame = 0;
}

void Explosion::Uninit()
{
	m_Sprite->Uninit();
	delete m_Sprite;
	m_Sprite = nullptr;
}

void Explosion::Update()
{
	if (m_Owner && m_Owner->GetNodeRuntime() && m_Owner->GetNodeRuntime()->GetState() == RuntimeState::Paused)
	{
		return;
	}

	m_Elapsed += GetFrameDeltaSeconds();

	if (m_Elapsed >= 0.4f)
	{
		SetDestroy();

		return;
	}

	m_Frame = static_cast<int>(m_Elapsed * 40);
}

void Explosion::Draw()
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

	m_Sprite->Draw(camera, m_Position, 0.65f, XMFLOAT4(1, 0.8f, 0.4f, 1 - m_Elapsed / 0.4f), m_Frame, 4, 4);
}
