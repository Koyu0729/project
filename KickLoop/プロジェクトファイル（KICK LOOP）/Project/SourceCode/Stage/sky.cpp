#include "main.h"
#include "renderer.h"
#include "sky.h"
#include "modelRenderer.h"
#include "input.h"
#include "camera.h"
#include "manager.h"
#include "scene.h"

void Sky::Init()
{
	m_ModelRenderer = new ModelRenderer();
	m_ModelRenderer->Load("asset\\model\\sky.obj");

	Renderer::CreateVertexShader(&m_VertexShader, &m_VertexLayout,
		"shader\\unlitTextureVS.cso");

	Renderer::CreatePixelShader(&m_PixelShader,
		"shader\\unlitTexturePS.cso");

	m_Scale = Vector3{1000.0f, 1000.0f, 1000.0f };
}

void Sky::Uninit()
{
	delete m_ModelRenderer;
	m_ModelRenderer = nullptr;

	if (m_VertexLayout != nullptr)
	{
		m_VertexLayout->Release();
		m_VertexLayout = nullptr;
	}
	if (m_VertexShader != nullptr)
	{
		m_VertexShader->Release();
		m_VertexShader = nullptr;
	}
	if (m_PixelShader != nullptr)
	{
		m_PixelShader->Release();
		m_PixelShader = nullptr;
	}
}

void Sky::Update()
{
	Camera* camera = Manager::GetScene()->GetGameObject<Camera>();

	m_Position = camera->GetPosition();
}

void Sky::Draw()
{
	//入力レイアウト設定
	Renderer::GetDeviceContext()->IASetInputLayout(m_VertexLayout);

	//シェーダ設定
	Renderer::GetDeviceContext()->VSSetShader(m_VertexShader, NULL, 0);
	Renderer::GetDeviceContext()->PSSetShader(m_PixelShader, NULL, 0);

	// 3Dマトリクス設定
	XMMATRIX world, scale, rot, trans;
	scale = XMMatrixScaling(m_Scale.x, m_Scale.y, m_Scale.z);
	rot = XMMatrixRotationRollPitchYaw(m_Rotation.x, m_Rotation.y + XM_PI, m_Rotation.z);
	trans = XMMatrixTranslation(m_Position.x, m_Position.y, m_Position.z);
	world = scale * rot * trans;
	Renderer::SetWorldMatrix(world);

	m_ModelRenderer->Draw();
}