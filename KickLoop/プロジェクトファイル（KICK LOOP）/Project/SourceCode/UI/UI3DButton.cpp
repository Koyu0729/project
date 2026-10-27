#include "UI3DButton.h"

#include "renderer.h"

#include "manager.h"
#include "scene.h"
#include "camera.h"
#include "mouseInput.h"

#include <cmath>
#include <cstdio>

void UI3DButton::Init()
{
	CreateQuad();

	m_Position = Vector3(
		0.0f,
		0.0f,
		0.0f
	);

	m_Rotation = Vector3(
		0.0f,
		0.0f,
		0.0f
	);

	m_Scale = Vector3(
		1.0f,
		1.0f,
		1.0f
	);

	m_Size = XMFLOAT2(
		1.0f,
		1.0f
	);

	m_HitDistance = 100.0f;

	m_IsHover = false;
	m_IsPressed = false;

	m_HoverFlashTimer = 0.0f;
	m_PressFlashTimer = 0.0f;

	m_HighlightScale = 1.25f;

	SetTexture(
		"asset\\texture\\frameBoxR.png"
	);

	UIQuadBase::SetColor(
		1.0f,
		1.0f,
		1.0f,
		1.0f
	);

	UpdateVertexBuffer();

	SetActive(true);
	SetVisible(true);
}

void UI3DButton::Uninit()
{
	UIQuadBase::Uninit();
}

void UI3DButton::Update()
{
	m_IsPressed = false;

	UpdateHit();

	if (m_IsHover)
	{
		m_HoverFlashTimer +=
			1.0f / 60.0f;
	}
	else
	{
		m_HoverFlashTimer = 0.0f;
	}

	if (m_PressFlashTimer > 0.0f)
	{
		m_PressFlashTimer -=
			1.0f / 60.0f;

		if (m_PressFlashTimer < 0.0f)
		{
			m_PressFlashTimer = 0.0f;
		}
	}
}

void UI3DButton::Draw()
{
	XMMATRIX world =
		BuildWorldMatrix();

	Renderer::SetWorldMatrix(
		world
	);

	if (m_IsHover)
	{
		DrawHighlight();
	}

	DrawMainButton();
}

XMMATRIX UI3DButton::BuildWorldMatrix() const
{
	XMMATRIX scale =
		XMMatrixScaling(
			m_Scale.x,
			m_Scale.y,
			m_Scale.z
		);

	XMMATRIX rotation =
		XMMatrixRotationRollPitchYaw(
			m_Rotation.x,
			m_Rotation.y,
			m_Rotation.z
		);

	XMMATRIX translation =
		XMMatrixTranslation(
			m_Position.x,
			m_Position.y,
			m_Position.z
		);

	return
		scale *
		rotation *
		translation;
}

void UI3DButton::DrawHighlight()
{
	float flash =
		(sinf(m_HoverFlashTimer * 12.0f) + 1.0f) *
		0.5f;

	float alpha =
		0.45f +
		flash *
		0.35f;

	UpdateVertexBufferWithScale(
		m_HighlightScale
	);

	UIQuadBase::SetColor(
		0.0f,
		1.0f,
		1.0f,
		alpha
	);

	SetDraw3D();

	DrawQuad();
}

void UI3DButton::DrawMainButton()
{
	UpdateVertexBuffer();

	if (m_PressFlashTimer > 0.0f)
	{
		UIQuadBase::SetColor(
			1.0f,
			0.2f,
			0.1f,
			1.0f
		);
	}
	else if (m_IsHover)
	{
		UIQuadBase::SetColor(
			0.2f,
			1.0f,
			1.0f,
			1.0f
		);
	}
	else
	{
		UIQuadBase::SetColor(
			1.0f,
			1.0f,
			1.0f,
			1.0f
		);
	}

	SetDraw3D();

	DrawQuad();
}

void UI3DButton::SetPosition(
	const Vector3& position
)
{
	m_Position = position;
}

void UI3DButton::SetPosition3D(
	float x,
	float y,
	float z
)
{
	m_Position =
		Vector3(
			x,
			y,
			z
		);
}

void UI3DButton::SetSize(
	float width,
	float height
)
{
	m_Size =
		XMFLOAT2(
			width,
			height
		);

	UpdateVertexBuffer();
}

void UI3DButton::SetRect3D(
	float x,
	float y,
	float z,
	float width,
	float height
)
{
	m_Position =
		Vector3(
			x,
			y,
			z
		);

	m_Size =
		XMFLOAT2(
			width,
			height
		);

	UpdateVertexBuffer();
}

void UI3DButton::SetRotation(
	float x,
	float y,
	float z
)
{
	m_Rotation =
		Vector3(
			DegreeToRadian(x),
			DegreeToRadian(y),
			DegreeToRadian(z)
		);
}

void UI3DButton::SetRotationRadian(
	float x,
	float y,
	float z
)
{
	m_Rotation =
		Vector3(
			x,
			y,
			z
		);
}

void UI3DButton::SetScale(
	float x,
	float y,
	float z
)
{
	m_Scale =
		Vector3(
			x,
			y,
			z
		);
}

void UI3DButton::SetHitDistance(
	float distance
)
{
	m_HitDistance = distance;
}

bool UI3DButton::IsHover() const
{
	return m_IsHover;
}

bool UI3DButton::IsPressed() const
{
	return m_IsPressed;
}

void UI3DButton::UpdateHit()
{
	Scene* scene =
		Manager::GetScene();

	if (scene == nullptr)
	{
		m_IsHover = false;
		return;
	}

	Camera* camera =
		scene->GetGameObject<Camera>();

	if (camera == nullptr)
	{
		m_IsHover = false;
		return;
	}

	m_IsHover =
		IsHitByCameraRay(
			camera
		);

	if (m_IsHover && Mouse::LeftClick())
	{
		m_IsPressed = true;

		m_PressFlashTimer =
			0.15f;
	}
}

bool UI3DButton::IsHitByCameraRay(
	Camera* camera
)
{
	Vector3 rayOrigin =
		camera->GetPosition();

	Vector3 rayDirection =
		camera->GetForward();

	XMVECTOR worldRayOrigin =
		XMVectorSet(
			rayOrigin.x,
			rayOrigin.y,
			rayOrigin.z,
			1.0f
		);

	XMVECTOR worldRayDirection =
		XMVectorSet(
			rayDirection.x,
			rayDirection.y,
			rayDirection.z,
			0.0f
		);

	worldRayDirection =
		XMVector3Normalize(
			worldRayDirection
		);

	XMMATRIX world =
		BuildWorldMatrix();

	XMVECTOR determinant;

	XMMATRIX inverseWorld =
		XMMatrixInverse(
			&determinant,
			world
		);

	XMVECTOR localRayOrigin =
		XMVector3TransformCoord(
			worldRayOrigin,
			inverseWorld
		);

	XMVECTOR localRayDirection =
		XMVector3TransformNormal(
			worldRayDirection,
			inverseWorld
		);

	localRayDirection =
		XMVector3Normalize(
			localRayDirection
		);

	XMFLOAT3 localOrigin{};
	XMFLOAT3 localDirection{};

	XMStoreFloat3(
		&localOrigin,
		localRayOrigin
	);

	XMStoreFloat3(
		&localDirection,
		localRayDirection
	);

	if (fabsf(localDirection.z) < 0.0001f)
	{
		return false;
	}

	// ボタンはローカル空間の z = 0 平面。
	float t =
		-localOrigin.z /
		localDirection.z;

	if (t < 0.0f)
	{
		return false;
	}

	if (t > m_HitDistance)
	{
		return false;
	}

	float hitX =
		localOrigin.x +
		localDirection.x *
		t;

	float hitY =
		localOrigin.y +
		localDirection.y *
		t;

	float halfWidth =
		m_Size.x *
		0.5f;

	float halfHeight =
		m_Size.y *
		0.5f;

	bool hit =
		hitX >= -halfWidth &&
		hitX <= halfWidth &&
		hitY >= -halfHeight &&
		hitY <= halfHeight;

#ifdef _DEBUG
	if (hit)
	{
		printf(
			"UI3DButton HIT local(%.2f, %.2f) size(%.2f, %.2f)\n",
			hitX,
			hitY,
			m_Size.x,
			m_Size.y
		);
	}
#endif

	return hit;
}

void UI3DButton::UpdateVertexBuffer()
{
	UpdateVertexBufferWithScale(
		1.0f
	);
}

void UI3DButton::UpdateVertexBufferWithScale(
	float scaleRate
)
{
	VERTEX_3D vertex[4]{};

	float halfWidth =
		m_Size.x *
		0.5f *
		scaleRate;

	float halfHeight =
		m_Size.y *
		0.5f *
		scaleRate;

	vertex[0].Position =
		XMFLOAT3(
			-halfWidth,
			halfHeight,
			0.0f
		);

	vertex[1].Position =
		XMFLOAT3(
			halfWidth,
			halfHeight,
			0.0f
		);

	vertex[2].Position =
		XMFLOAT3(
			-halfWidth,
			-halfHeight,
			0.0f
		);

	vertex[3].Position =
		XMFLOAT3(
			halfWidth,
			-halfHeight,
			0.0f
		);

	vertex[0].TexCoord =
		XMFLOAT2(
			0.0f,
			0.0f
		);

	vertex[1].TexCoord =
		XMFLOAT2(
			1.0f,
			0.0f
		);

	vertex[2].TexCoord =
		XMFLOAT2(
			0.0f,
			1.0f
		);

	vertex[3].TexCoord =
		XMFLOAT2(
			1.0f,
			1.0f
		);

	for (int i = 0; i < 4; i++)
	{
		vertex[i].Normal =
			XMFLOAT3(
				0.0f,
				0.0f,
				-1.0f
			);

		vertex[i].Diffuse =
			m_Color;
	}

	UpdateQuad(
		vertex
	);
}

float UI3DButton::DegreeToRadian(
	float degree
)
{
	const float PI =
		3.1415926535f;

	return degree *
		PI /
		180.0f;
}