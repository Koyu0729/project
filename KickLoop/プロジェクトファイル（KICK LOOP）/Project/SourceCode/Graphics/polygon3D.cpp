#include "main.h"
#include "renderer.h"
#include "polygon3D.h"

#include "manager.h"
#include "scene.h"
#include "player.h"

#include <cmath>

void Polygon3D::Init()
{
	CreateQuad();

	m_Position = Vector3(0.0f, 0.0f, 0.0f);
	m_Rotation = Vector3(0.0f, 0.0f, 0.0f);
	m_Scale = Vector3(1.0f, 1.0f, 1.0f);

	m_Size = XMFLOAT2(1.0f, 1.0f);

	m_UseFloating = false;
	m_FloatBaseY = 0.0f;
	m_FloatTimer = 0.0f;
	m_FloatAmplitude = 0.0f;
	m_FloatSpeed = 0.0f;

	m_UseTiltElastic = false;
	m_TiltAngle = 0.0f;
	m_TiltVelocity = 0.0f;
	m_TiltSpring = 0.0f;

	m_LookAtPlayer = false;

	SetTexture("asset\\texture\\kuro.png");

	UpdateVertexBuffer();
}

void Polygon3D::Uninit()
{
	UIQuadBase::Uninit();
}

void Polygon3D::Update()
{
	UpdateLookAtPlayer();
	UpdateFloating();
	UpdateTiltElastic();
}

void Polygon3D::Draw()
{
	XMMATRIX scale = XMMatrixScaling(
		m_Scale.x,
		m_Scale.y,
		m_Scale.z
	);

	XMMATRIX rotation = XMMatrixRotationRollPitchYaw(
		m_Rotation.x,
		m_Rotation.y,
		m_Rotation.z
	);

	XMMATRIX translation = XMMatrixTranslation(
		m_Position.x,
		m_Position.y,
		m_Position.z
	);

	XMMATRIX world =
		scale *
		rotation *
		translation;

	Renderer::SetWorldMatrix(world);

	SetDraw3D();

	DrawQuad();
}

void Polygon3D::SetPosition3D(float x, float y, float z)
{
	m_Position = Vector3(x, y, z);

	if (m_UseFloating)
	{
		m_FloatBaseY = y;
	}
}

void Polygon3D::SetSize(float w, float h)
{
	m_Size = XMFLOAT2(w, h);

	UpdateVertexBuffer();
}

void Polygon3D::SetRect3D(
	float x,
	float y,
	float z,
	float w,
	float h
)
{
	m_Position = Vector3(x, y, z);
	m_Size = XMFLOAT2(w, h);

	if (m_UseFloating)
	{
		m_FloatBaseY = y;
	}

	UpdateVertexBuffer();
}

void Polygon3D::SetRotation(float x, float y, float z)
{
	m_Rotation = Vector3(
		DegreeToRadian(x),
		DegreeToRadian(y),
		DegreeToRadian(z)
	);
}

void Polygon3D::SetRotationRadian(float x, float y, float z)
{
	m_Rotation = Vector3(x, y, z);
}

void Polygon3D::SetColor(float r, float g, float b, float a)
{
	UIQuadBase::SetColor(r, g, b, a);

	UpdateVertexBuffer();
}

void Polygon3D::SetAlpha(float alpha)
{
	UIQuadBase::SetAlpha(alpha);

	UpdateVertexBuffer();
}

void Polygon3D::SetFloating(float amplitude, float speed)
{
	m_UseFloating = true;

	m_FloatBaseY = m_Position.y;
	m_FloatTimer = 0.0f;

	m_FloatAmplitude = amplitude;
	m_FloatSpeed = speed;
}

void Polygon3D::StopFloating()
{
	m_UseFloating = false;

	m_Position.y = m_FloatBaseY;

	m_FloatTimer = 0.0f;
	m_FloatAmplitude = 0.0f;
	m_FloatSpeed = 0.0f;
}

void Polygon3D::StartTiltElastic(float power, float spring)
{
	m_UseTiltElastic = true;

	m_TiltAngle = power;
	m_TiltVelocity = 0.0f;
	m_TiltSpring = spring;

	m_Rotation.z = DegreeToRadian(m_TiltAngle);
}

void Polygon3D::StopTiltElastic()
{
	m_UseTiltElastic = false;

	m_TiltAngle = 0.0f;
	m_TiltVelocity = 0.0f;
	m_TiltSpring = 0.0f;

	m_Rotation.z = 0.0f;
}

void Polygon3D::SetLookAtPlayer(bool enable)
{
	m_LookAtPlayer = enable;
}

void Polygon3D::UpdateLookAtPlayer()
{
	if (!m_LookAtPlayer)
	{
		return;
	}

	Scene* scene = Manager::GetScene();

	if (scene == nullptr)
	{
		return;
	}

	Player* player = scene->GetGameObject<Player>();

	if (player == nullptr)
	{
		return;
	}

	Vector3 playerPosition = player->GetPosition();

	float dx = playerPosition.x - m_Position.x;
	float dz = playerPosition.z - m_Position.z;

	m_Rotation.y = atan2f(dx, dz);
}

void Polygon3D::UpdateFloating()
{
	if (!m_UseFloating)
	{
		return;
	}

	m_FloatTimer += 1.0f / 60.0f;

	m_Position.y =
		m_FloatBaseY +
		sinf(m_FloatTimer * m_FloatSpeed) *
		m_FloatAmplitude;
}

void Polygon3D::UpdateTiltElastic()
{
	if (!m_UseTiltElastic)
	{
		return;
	}

	float force =
		-m_TiltAngle *
		m_TiltSpring;

	m_TiltVelocity += force;
	m_TiltVelocity *= 0.90f;

	m_TiltAngle += m_TiltVelocity;

	m_Rotation.z = DegreeToRadian(m_TiltAngle);

	if (fabsf(m_TiltAngle) < 0.01f &&
		fabsf(m_TiltVelocity) < 0.01f)
	{
		m_TiltAngle = 0.0f;
		m_TiltVelocity = 0.0f;

		m_Rotation.z = 0.0f;

		m_UseTiltElastic = false;
	}
}

void Polygon3D::UpdateVertexBuffer()
{
	VERTEX_3D vertex[4]{};

	float halfW = m_Size.x * 0.5f;
	float halfH = m_Size.y * 0.5f;

	vertex[0].Position = XMFLOAT3(-halfW, halfH, 0.0f);
	vertex[1].Position = XMFLOAT3(halfW, halfH, 0.0f);
	vertex[2].Position = XMFLOAT3(-halfW, -halfH, 0.0f);
	vertex[3].Position = XMFLOAT3(halfW, -halfH, 0.0f);

	vertex[0].TexCoord = XMFLOAT2(0.0f, 0.0f);
	vertex[1].TexCoord = XMFLOAT2(1.0f, 0.0f);
	vertex[2].TexCoord = XMFLOAT2(0.0f, 1.0f);
	vertex[3].TexCoord = XMFLOAT2(1.0f, 1.0f);

	for (int i = 0; i < 4; i++)
	{
		vertex[i].Normal = XMFLOAT3(0.0f, 0.0f, -1.0f);
		vertex[i].Diffuse = m_Color;
	}

	UpdateQuad(vertex);
}

float Polygon3D::DegreeToRadian(float degree)
{
	const float PI = 3.1415926535f;

	return degree * PI / 180.0f;
}