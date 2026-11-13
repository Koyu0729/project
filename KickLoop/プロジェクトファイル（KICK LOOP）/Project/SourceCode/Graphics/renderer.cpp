
#include "main.h"
#include "renderer.h"
#include <fstream>
#include <stdexcept>
#include <vector>
#include <wrl/client.h>


D3D_FEATURE_LEVEL       Renderer::m_FeatureLevel = D3D_FEATURE_LEVEL_11_0;

ID3D11Device*           Renderer::m_Device{};
ID3D11DeviceContext*    Renderer::m_DeviceContext{};
IDXGISwapChain*         Renderer::m_SwapChain{};
ID3D11RenderTargetView* Renderer::m_RenderTargetView{};
ID3D11DepthStencilView* Renderer::m_DepthStencilView{};

ID3D11Buffer*			Renderer::m_WorldBuffer{};
ID3D11Buffer*			Renderer::m_ViewBuffer{};
ID3D11Buffer*			Renderer::m_ProjectionBuffer{};
ID3D11Buffer*			Renderer::m_MaterialBuffer{};
ID3D11Buffer*			Renderer::m_LightBuffer{};


ID3D11DepthStencilState* Renderer::m_DepthStateEnable{};
ID3D11DepthStencilState* Renderer::m_DepthStateDisable{};


ID3D11BlendState*		Renderer::m_BlendState{};
ID3D11BlendState*		Renderer::m_BlendStateATC{};
ID3D11BlendState*		Renderer::m_BlendStateAdd{};


template <typename T>
static void ReleaseResource(T*& resource)
{
	if (resource != nullptr)
	{
		resource->Release();
		resource = nullptr;
	}
}

static void CheckInitResult(HRESULT result, const char* message)
{
	if (FAILED(result))
	{
		throw std::runtime_error(message);
	}
}

void Renderer::Init()
{
	// デバイス(DirectXをコントロールする)、スワップチェーン作成
	DXGI_SWAP_CHAIN_DESC swapChainDesc{};
	swapChainDesc.BufferCount = 1;
	swapChainDesc.BufferDesc.Width = SCREEN_WIDTH;
	swapChainDesc.BufferDesc.Height = SCREEN_HEIGHT;
	swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.BufferDesc.RefreshRate.Numerator = 60;
	swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.OutputWindow = GetWindow();
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.SampleDesc.Quality = 0;
	swapChainDesc.Windowed = TRUE;

	HRESULT hr = D3D11CreateDeviceAndSwapChain( NULL,
										D3D_DRIVER_TYPE_HARDWARE,
										NULL,
										0,
										NULL,
										0,
										D3D11_SDK_VERSION,
										&swapChainDesc,
										&m_SwapChain,
										&m_Device,
										&m_FeatureLevel,
										&m_DeviceContext );
	CheckInitResult(hr, "Could not initialize the Direct3D 11 device.");


	// レンダーターゲットビュー作成
	Microsoft::WRL::ComPtr<ID3D11Texture2D> renderTarget;
	CheckInitResult(m_SwapChain->GetBuffer(0, IID_PPV_ARGS(renderTarget.GetAddressOf())),
		"Could not get the back buffer.");
	CheckInitResult(m_Device->CreateRenderTargetView(renderTarget.Get(), nullptr, &m_RenderTargetView),
		"Could not create the render target view.");


	// デプスステンシルバッファ作成 メモリ上の本体
	Microsoft::WRL::ComPtr<ID3D11Texture2D> depthStencil;
	D3D11_TEXTURE2D_DESC textureDesc{};
	textureDesc.Width = swapChainDesc.BufferDesc.Width;
	textureDesc.Height = swapChainDesc.BufferDesc.Height;
	textureDesc.MipLevels = 1;
	textureDesc.ArraySize = 1;
	textureDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	textureDesc.SampleDesc = swapChainDesc.SampleDesc;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	textureDesc.CPUAccessFlags = 0;
	textureDesc.MiscFlags = 0;
	CheckInitResult(m_Device->CreateTexture2D(&textureDesc, nullptr, depthStencil.GetAddressOf()),
		"Could not create the depth stencil buffer.");

	// デプスステンシルビュー作成 バッファにアクセスするための窓口
	D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc{};
	depthStencilViewDesc.Format = textureDesc.Format;
	depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	depthStencilViewDesc.Flags = 0;
	CheckInitResult(m_Device->CreateDepthStencilView(depthStencil.Get(), &depthStencilViewDesc, &m_DepthStencilView),
		"Could not create the depth stencil view.");


	m_DeviceContext->OMSetRenderTargets(1, &m_RenderTargetView, m_DepthStencilView);


	// ビューポート設定 描画する場所の設定
	D3D11_VIEWPORT viewport;
	viewport.Width = (FLOAT)SCREEN_WIDTH;
	viewport.Height = (FLOAT)SCREEN_HEIGHT;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	m_DeviceContext->RSSetViewports( 1, &viewport );


	// ラスタライザステート設定
	D3D11_RASTERIZER_DESC rasterizerDesc{};
	rasterizerDesc.FillMode = D3D11_FILL_SOLID;
	rasterizerDesc.CullMode = D3D11_CULL_BACK; 
	rasterizerDesc.DepthClipEnable = TRUE; 
	rasterizerDesc.MultisampleEnable = FALSE; 

	Microsoft::WRL::ComPtr<ID3D11RasterizerState> rs;
	CheckInitResult(m_Device->CreateRasterizerState(&rasterizerDesc, rs.GetAddressOf()),
		"Could not create the rasterizer state.");

	m_DeviceContext->RSSetState(rs.Get());


	// ブレンドステート設定 合成の設定
	D3D11_BLEND_DESC blendDesc{};
	blendDesc.AlphaToCoverageEnable = FALSE;
	blendDesc.IndependentBlendEnable = FALSE;
	blendDesc.RenderTarget[0].BlendEnable = TRUE;
	blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
	blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

	CheckInitResult(m_Device->CreateBlendState( &blendDesc, &m_BlendState ),
		"Could not create a blend state.");

	blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_ONE;
	CheckInitResult(m_Device->CreateBlendState( &blendDesc, &m_BlendStateAdd ),
		"Could not create a blend state.");

	blendDesc.AlphaToCoverageEnable = TRUE;
	CheckInitResult(m_Device->CreateBlendState( &blendDesc, &m_BlendStateATC ),
		"Could not create a blend state.");

	float blendFactor[4] = {0.0f, 0.0f, 0.0f, 0.0f};
	m_DeviceContext->OMSetBlendState(m_BlendState, blendFactor, 0xffffffff );


	// デプスステンシルステート設定
	D3D11_DEPTH_STENCIL_DESC depthStencilDesc{};
	depthStencilDesc.DepthEnable = TRUE;
	depthStencilDesc.DepthWriteMask	= D3D11_DEPTH_WRITE_MASK_ALL;
	depthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
	depthStencilDesc.StencilEnable = FALSE;

	CheckInitResult(m_Device->CreateDepthStencilState( &depthStencilDesc, &m_DepthStateEnable ),
		"Could not create a depth stencil state."); // 深度テストと書き込みを有効にする

	depthStencilDesc.DepthWriteMask	= D3D11_DEPTH_WRITE_MASK_ZERO;
	CheckInitResult(m_Device->CreateDepthStencilState( &depthStencilDesc, &m_DepthStateDisable ),
		"Could not create a depth stencil state."); // 深度テストを有効にしたまま、書き込みを止める

	m_DeviceContext->OMSetDepthStencilState( m_DepthStateEnable, NULL );


	// サンプラーステート設定 テクスチャからどうやって色を取り出すか
	D3D11_SAMPLER_DESC samplerDesc{};
	samplerDesc.Filter = D3D11_FILTER_ANISOTROPIC;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.MaxAnisotropy = 4;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

	Microsoft::WRL::ComPtr<ID3D11SamplerState> samplerState;
	CheckInitResult(m_Device->CreateSamplerState(&samplerDesc, samplerState.GetAddressOf()),
		"Could not create the sampler state.");

	m_DeviceContext->PSSetSamplers(0, 1, samplerState.GetAddressOf());


	// 定数バッファ生成
	D3D11_BUFFER_DESC bufferDesc{};
	bufferDesc.ByteWidth = sizeof(XMFLOAT4X4);
	bufferDesc.Usage = D3D11_USAGE_DEFAULT;
	bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bufferDesc.CPUAccessFlags = 0;
	bufferDesc.MiscFlags = 0;
	bufferDesc.StructureByteStride = sizeof(float);

	CheckInitResult(m_Device->CreateBuffer( &bufferDesc, NULL, &m_WorldBuffer ),
		"Could not create a constant buffer.");
	m_DeviceContext->VSSetConstantBuffers( 0, 1, &m_WorldBuffer);

	CheckInitResult(m_Device->CreateBuffer( &bufferDesc, NULL, &m_ViewBuffer ),
		"Could not create a constant buffer.");
	m_DeviceContext->VSSetConstantBuffers( 1, 1, &m_ViewBuffer );

	CheckInitResult(m_Device->CreateBuffer( &bufferDesc, NULL, &m_ProjectionBuffer ),
		"Could not create a constant buffer.");
	m_DeviceContext->VSSetConstantBuffers( 2, 1, &m_ProjectionBuffer );


	bufferDesc.ByteWidth = sizeof(MATERIAL);

	CheckInitResult(m_Device->CreateBuffer( &bufferDesc, NULL, &m_MaterialBuffer ),
		"Could not create a constant buffer.");
	m_DeviceContext->VSSetConstantBuffers( 3, 1, &m_MaterialBuffer );
	m_DeviceContext->PSSetConstantBuffers( 3, 1, &m_MaterialBuffer );


	bufferDesc.ByteWidth = sizeof(LIGHT);

	CheckInitResult(m_Device->CreateBuffer( &bufferDesc, NULL, &m_LightBuffer ),
		"Could not create a constant buffer.");
	m_DeviceContext->VSSetConstantBuffers( 4, 1, &m_LightBuffer );
	m_DeviceContext->PSSetConstantBuffers( 4, 1, &m_LightBuffer );


	// ライト初期化
	LIGHT light{};
	light.Enable = true;
	light.Direction = XMFLOAT4(1.0f, -1.0f, 0.5f, 0.0f);
	light.Ambient = XMFLOAT4(0.1f, 0.1f, 0.1f, 1.0f);
	light.Diffuse = XMFLOAT4(1.5f, 1.5f, 1.5f, 1.0f);
	SetLight(light);


	// マテリアル初期化
	MATERIAL material{};
	material.Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	material.Ambient = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	SetMaterial(material);


}


void Renderer::Uninit()
{
	ReleaseResource(m_WorldBuffer);
	ReleaseResource(m_ViewBuffer);
	ReleaseResource(m_ProjectionBuffer);
	ReleaseResource(m_LightBuffer);
	ReleaseResource(m_MaterialBuffer);

	if (m_DeviceContext != nullptr)
	{
		m_DeviceContext->ClearState();
	}

	ReleaseResource(m_BlendState);
	ReleaseResource(m_BlendStateATC);
	ReleaseResource(m_BlendStateAdd);
	ReleaseResource(m_DepthStateEnable);
	ReleaseResource(m_DepthStateDisable);
	ReleaseResource(m_DepthStencilView);
	ReleaseResource(m_RenderTargetView);
	ReleaseResource(m_SwapChain);
	ReleaseResource(m_DeviceContext);
	ReleaseResource(m_Device);
}


void Renderer::Begin() // 描画開始
{
	float clearColor[4] = { 0.9f, 0.8f, 0.9f, 1.0f };
	m_DeviceContext->ClearRenderTargetView( m_RenderTargetView, clearColor );
	m_DeviceContext->ClearDepthStencilView( m_DepthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);
}


void Renderer::End() // 描画終了
{
	m_SwapChain->Present( 0, 0 );
}


void Renderer::SetDepthEnable( bool Enable )
{
	if( Enable )
		m_DeviceContext->OMSetDepthStencilState( m_DepthStateEnable, NULL );
	else
		m_DeviceContext->OMSetDepthStencilState( m_DepthStateDisable, NULL );

}


void Renderer::SetATCEnable( bool Enable )
{
	float blendFactor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };

	if (Enable)
		m_DeviceContext->OMSetBlendState(m_BlendStateATC, blendFactor, 0xffffffff);
	else
		m_DeviceContext->OMSetBlendState(m_BlendState, blendFactor, 0xffffffff);

}

void Renderer::SetAddBlendEnable( bool Enable )
{
	float blendFactor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };

	if (Enable)
		m_DeviceContext->OMSetBlendState(m_BlendStateAdd, blendFactor, 0xffffffff);
	else
		m_DeviceContext->OMSetBlendState(m_BlendState, blendFactor, 0xffffffff);

}

void Renderer::SetWorldViewProjection2D()
{
	SetWorldMatrix(XMMatrixIdentity());
	SetViewMatrix(XMMatrixIdentity());

	XMMATRIX projection;
	projection = XMMatrixOrthographicOffCenterLH(0.0f, SCREEN_WIDTH, SCREEN_HEIGHT, 0.0f, 0.0f, 1.0f);
	SetProjectionMatrix(projection);
}


void Renderer::SetWorldMatrix(XMMATRIX WorldMatrix)
{
	XMFLOAT4X4 worldf;
	XMStoreFloat4x4(&worldf, XMMatrixTranspose(WorldMatrix));
	m_DeviceContext->UpdateSubresource(m_WorldBuffer, 0, NULL, &worldf, 0, 0);
}

void Renderer::SetViewMatrix(XMMATRIX ViewMatrix)
{
	XMFLOAT4X4 viewf;
	XMStoreFloat4x4(&viewf, XMMatrixTranspose(ViewMatrix));
	m_DeviceContext->UpdateSubresource(m_ViewBuffer, 0, NULL, &viewf, 0, 0);
}

void Renderer::SetProjectionMatrix(XMMATRIX ProjectionMatrix)
{
	XMFLOAT4X4 projectionf;
	XMStoreFloat4x4(&projectionf, XMMatrixTranspose(ProjectionMatrix));
	m_DeviceContext->UpdateSubresource(m_ProjectionBuffer, 0, NULL, &projectionf, 0, 0);

}


void Renderer::SetMaterial( MATERIAL Material )
{
	m_DeviceContext->UpdateSubresource( m_MaterialBuffer, 0, NULL, &Material, 0, 0 );
}

void Renderer::SetLight( LIGHT Light )
{
	m_DeviceContext->UpdateSubresource(m_LightBuffer, 0, NULL, &Light, 0, 0);
}


static std::vector<char> LoadShaderFile(const char* fileName)
{
	std::ifstream file(fileName, std::ios::binary | std::ios::ate);
	const std::streamoff size = file.tellg();
	if (size <= 0)
	{
		throw std::runtime_error(std::string("Could not open shader or shader is empty: ") + fileName);
	}

	std::vector<char> buffer(static_cast<size_t>(size));
	file.seekg(0, std::ios::beg);
	if (!file.read(buffer.data(), size))
	{
		throw std::runtime_error(std::string("Could not read shader: ") + fileName);
	}

	return buffer;
}


void Renderer::CreateVertexShader(ID3D11VertexShader** VertexShader, ID3D11InputLayout** VertexLayout, const char* FileName)
{
	const auto buffer = LoadShaderFile(FileName);
	Microsoft::WRL::ComPtr<ID3D11VertexShader> shader;
	if (FAILED(m_Device->CreateVertexShader(buffer.data(), buffer.size(), nullptr, shader.GetAddressOf())))
	{
		throw std::runtime_error(std::string("Could not create vertex shader: ") + FileName);
	}

	D3D11_INPUT_ELEMENT_DESC layout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 4 * 3, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 4 * 6, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 4 * 10, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};
	UINT numElements = ARRAYSIZE(layout);

	Microsoft::WRL::ComPtr<ID3D11InputLayout> inputLayout;
	if (FAILED(m_Device->CreateInputLayout(layout, numElements, buffer.data(), buffer.size(), inputLayout.GetAddressOf())))
	{
		throw std::runtime_error(std::string("Could not create shader input layout: ") + FileName);
	}

	*VertexShader = shader.Detach();
	*VertexLayout = inputLayout.Detach();
}


void Renderer::CreatePixelShader(ID3D11PixelShader** PixelShader, const char* FileName)
{
	const auto buffer = LoadShaderFile(FileName);
	if (FAILED(m_Device->CreatePixelShader(buffer.data(), buffer.size(), nullptr, PixelShader)))
	{
		throw std::runtime_error(std::string("Could not create pixel shader: ") + FileName);
	}
}


void Renderer::CreateComputeShader(ID3D11ComputeShader** ComputeShader, const char* FileName)
{
	const auto buffer = LoadShaderFile(FileName);
	if (FAILED(m_Device->CreateComputeShader(buffer.data(), buffer.size(), nullptr, ComputeShader)))
	{
		throw std::runtime_error(std::string("Could not create compute shader: ") + FileName);
	}
}
