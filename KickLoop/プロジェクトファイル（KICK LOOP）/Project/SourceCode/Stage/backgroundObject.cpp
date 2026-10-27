#include "backgroundObject.h"

#include "renderer.h"
#include "modelRenderer.h"

void BackgroundObject::Init()
{
	if (m_ModelRenderer != nullptr)
	{
		delete m_ModelRenderer;
		m_ModelRenderer = nullptr;
	}

	m_ModelRenderer = new ModelRenderer();

	Renderer::CreateVertexShader(
		&m_VertexShader,
		&m_VertexLayout,
		"shader\\unlitTextureVS.cso"
	);

	Renderer::CreatePixelShader(
		&m_PixelShader,
		"shader\\unlitTexturePS.cso"
	);

	GameObject::SetActive(true);
	SetVisible(true);
}

void BackgroundObject::Uninit()
{
	if (m_ModelRenderer != nullptr)
	{
		delete m_ModelRenderer;
		m_ModelRenderer = nullptr;
	}

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

	m_ModelPath.clear();
}

void BackgroundObject::Update()
{
}

void BackgroundObject::Draw()
{
	if (!Active())
	{
		return;
	}

	if (!Visible())
	{
		return;
	}

	if (m_ModelRenderer == nullptr)
	{
		return;
	}

	if (m_ModelPath.empty())
	{
		return;
	}

	Renderer::GetDeviceContext()->IASetInputLayout(m_VertexLayout);

	Renderer::GetDeviceContext()->VSSetShader(
		m_VertexShader,
		nullptr,
		0
	);

	Renderer::GetDeviceContext()->PSSetShader(
		m_PixelShader,
		nullptr,
		0
	);

	XMMATRIX scale = XMMatrixScaling(m_Scale.x, m_Scale.y, m_Scale.z);
	XMMATRIX rotation = XMMatrixRotationRollPitchYaw(m_Rotation.x, m_Rotation.y, m_Rotation.z);
	XMMATRIX translation = XMMatrixTranslation(m_Position.x, m_Position.y, m_Position.z);
	XMMATRIX world = scale * rotation * translation;

	Renderer::SetWorldMatrix(world);

	m_ModelRenderer->Draw();
}

void BackgroundObject::LoadModel(const char* path)
{
	if (path == nullptr)
	{
		return;
	}

	if (m_ModelRenderer == nullptr)
	{
		return;
	}

	m_ModelPath = path;

	if (m_ModelPath.empty())
	{
		return;
	}

	m_ModelRenderer->Load(m_ModelPath.c_str());
}

void BackgroundObject::LoadModel(const std::string& path)
{
	LoadModel(path.c_str());
}

void BackgroundObject::AddMoveZ(float move)
{
	m_Position.z -= move;
}

void BackgroundObject::SetActive(bool active)
{
	GameObject::SetActive(active);
}

bool BackgroundObject::IsActive() const
{
	return Active();
}

bool BackgroundObject::HasModel() const
{
	if (m_ModelRenderer == nullptr)
	{
		return false;
	}

	if (m_ModelPath.empty())
	{
		return false;
	}

	return true;
}
