#include "main.h"
#include "renderer.h"
#include "particle.h"
#include "camera.h"
#include "manager.h"
#include "scene.h"
#include "texture.h"

#include <vector>
#include <algorithm>

#ifdef _DEBUG
#include <Windows.h>
#endif

void Particle::Init()
{
	VERTEX_3D vertex[4]{};

	vertex[0].Position = XMFLOAT3(-1.0f, 1.0f, 0.0f);
	vertex[0].Normal = XMFLOAT3(0.0f, 1.0f, 0.0f);
	vertex[0].Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	vertex[0].TexCoord = XMFLOAT2(0.0f, 0.0f);

	vertex[1].Position = XMFLOAT3(1.0f, 1.0f, 0.0f);
	vertex[1].Normal = XMFLOAT3(0.0f, 1.0f, 0.0f);
	vertex[1].Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	vertex[1].TexCoord = XMFLOAT2(1.0f, 0.0f);

	vertex[2].Position = XMFLOAT3(-1.0f, -1.0f, 0.0f);
	vertex[2].Normal = XMFLOAT3(0.0f, 1.0f, 0.0f);
	vertex[2].Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	vertex[2].TexCoord = XMFLOAT2(0.0f, 1.0f);

	vertex[3].Position = XMFLOAT3(1.0f, -1.0f, 0.0f);
	vertex[3].Normal = XMFLOAT3(0.0f, 1.0f, 0.0f);
	vertex[3].Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	vertex[3].TexCoord = XMFLOAT2(1.0f, 1.0f);

	D3D11_BUFFER_DESC vertexBufferDesc{};
	vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	vertexBufferDesc.ByteWidth = sizeof(VERTEX_3D) * 4;
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexBufferDesc.CPUAccessFlags = 0;

	D3D11_SUBRESOURCE_DATA vertexBufferData{};
	vertexBufferData.pSysMem = vertex;

	Renderer::GetDevice()->CreateBuffer(
		&vertexBufferDesc,
		&vertexBufferData,
		&m_VertexBuffer
	);

	m_Texture = Texture::Load("asset\\texture\\Particle.png");

	std::vector<ParticleGpuData> initialData;
	initialData.resize(m_MaxCount);

	for (int i = 0; i < m_MaxCount; i++)
	{
		initialData[i].PositionLife = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
		initialData[i].VelocityMaxLife = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
		initialData[i].ColorSize = XMFLOAT4(1.0f, 1.0f, 1.0f, 0.1f);
		initialData[i].Param = XMFLOAT4(0.0f, 0.0f, 1.0f, 0.0f);
	}

	D3D11_BUFFER_DESC particleBufferDesc{};
	particleBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	particleBufferDesc.ByteWidth = sizeof(ParticleGpuData) * m_MaxCount;
	particleBufferDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
	particleBufferDesc.CPUAccessFlags = 0;
	particleBufferDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
	particleBufferDesc.StructureByteStride = sizeof(ParticleGpuData);

	D3D11_SUBRESOURCE_DATA particleBufferData{};
	particleBufferData.pSysMem = initialData.data();

	Renderer::GetDevice()->CreateBuffer(
		&particleBufferDesc,
		&particleBufferData,
		&m_ParticleBuffer
	);

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format = DXGI_FORMAT_UNKNOWN;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
	srvDesc.Buffer.FirstElement = 0;
	srvDesc.Buffer.NumElements = m_MaxCount;

	Renderer::GetDevice()->CreateShaderResourceView(
		m_ParticleBuffer,
		&srvDesc,
		&m_ParticleSRV
	);

	D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc{};
	uavDesc.Format = DXGI_FORMAT_UNKNOWN;
	uavDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
	uavDesc.Buffer.FirstElement = 0;
	uavDesc.Buffer.NumElements = m_MaxCount;

	Renderer::GetDevice()->CreateUnorderedAccessView(
		m_ParticleBuffer,
		&uavDesc,
		&m_ParticleUAV
	);

	D3D11_BUFFER_DESC constantBufferDesc{};
	constantBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	constantBufferDesc.ByteWidth = sizeof(UpdateConstantBuffer);
	constantBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	constantBufferDesc.CPUAccessFlags = 0;

	Renderer::GetDevice()->CreateBuffer(
		&constantBufferDesc,
		nullptr,
		&m_UpdateConstantBuffer
	);

	constantBufferDesc.ByteWidth = sizeof(RenderConstantBuffer);

	Renderer::GetDevice()->CreateBuffer(
		&constantBufferDesc,
		nullptr,
		&m_RenderConstantBuffer
	);

	Renderer::CreateVertexShader(
		&m_VertexShader,
		&m_VertexLayout,
		"shader\\gpuParticleVS.cso"
	);

	Renderer::CreatePixelShader(
		&m_PixelShader,
		"shader\\gpuParticlePS.cso"
	);

	Renderer::CreateComputeShader(
		&m_ComputeShader,
		"shader\\gpuParticleCS.cso"
	);

	m_EmitCursor = 0;
	m_DrawFrame = 0;
	m_Time = 0.0f;
	m_EmitRequests.clear();
	m_EmitRequests.reserve(512);
}

void Particle::Uninit()
{
	ReleaseResource();

	m_EmitCursor = 0;
	m_DrawFrame = 0;
	m_Time = 0.0f;
	m_EmitRequests.clear();
}

// 粒子をGPUで更新して発生要求を処理する
void Particle::Update()
{
	m_Time += 1.0f;

	DispatchUpdate(
		1.0f,
		nullptr
	);

	for (const EmitRequest& request : m_EmitRequests)
	{
		DispatchUpdate(
			0.0f,
			&request
		);
	}
#ifdef _DEBUG
	DebugPrintParticleCount();
#endif // DEBUG

	m_EmitRequests.clear();

	if (m_DrawFrame > 0)
	{
		m_DrawFrame--;
	}
}

void Particle::Draw()
{
	if (m_DrawFrame <= 0)
	{
		return;
	}

	if (m_VertexBuffer == nullptr ||
		m_ParticleSRV == nullptr ||
		m_VertexShader == nullptr ||
		m_PixelShader == nullptr)
	{
		return;
	}

	Scene* scene = Manager::GetScene();

	if (scene == nullptr)
	{
		return;
	}

	Camera* camera = scene->GetGameObject<Camera>();

	if (camera == nullptr)
	{
		return;
	}

	XMMATRIX view = camera->GetViewMatrix();
	XMMATRIX projection = camera->GetProjectionMatrix();
	XMMATRIX invView = XMMatrixInverse(nullptr, view);

	XMFLOAT3 cameraRight(invView.r[0].m128_f32[0], invView.r[0].m128_f32[1], invView.r[0].m128_f32[2]);
	XMFLOAT3 cameraUp(invView.r[1].m128_f32[0], invView.r[1].m128_f32[1], invView.r[1].m128_f32[2]);

	RenderConstantBuffer cb{};
	cb.ViewProjection = XMMatrixTranspose(view * projection);
	cb.CameraRight = XMFLOAT4(cameraRight.x, cameraRight.y, cameraRight.z, 0.0f);
	cb.CameraUp = XMFLOAT4(cameraUp.x, cameraUp.y, cameraUp.z, 0.0f);

	ID3D11DeviceContext* context = Renderer::GetDeviceContext();

	if (context == nullptr)
	{
		return;
	}

	context->UpdateSubresource(
		m_RenderConstantBuffer,
		0,
		nullptr,
		&cb,
		0,
		0
	);

	UINT stride = sizeof(VERTEX_3D);
	UINT offset = 0;

	context->IASetInputLayout(m_VertexLayout);

	context->IASetVertexBuffers(
		0,
		1,
		&m_VertexBuffer,
		&stride,
		&offset
	);

	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	context->VSSetShader(
		m_VertexShader,
		nullptr,
		0
	);

	context->VSSetConstantBuffers(
		5,
		1,
		&m_RenderConstantBuffer
	);

	context->VSSetShaderResources(
		0,
		1,
		&m_ParticleSRV
	);

	context->PSSetShader(
		m_PixelShader,
		nullptr,
		0
	);

	if (m_Texture != nullptr)
	{
		context->PSSetShaderResources(
			0,
			1,
			&m_Texture
		);
	}

	Renderer::SetAddBlendEnable(true);
	Renderer::SetDepthEnable(false);

	DrawParticles();

	ID3D11ShaderResourceView* nullSRV = nullptr;

	context->VSSetShaderResources(
		0,
		1,
		&nullSRV
	);

	context->PSSetShaderResources(
		0,
		1,
		&nullSRV
	);

	ID3D11Buffer* nullBuffer = nullptr;

	context->VSSetConstantBuffers(
		5,
		1,
		&nullBuffer
	);

	Renderer::SetDepthEnable(true);
	Renderer::SetAddBlendEnable(false);
}

bool Particle::IsReady() const
{
	return m_EmitRequests.capacity() > 0
		&& m_VertexBuffer != nullptr
		&& m_ParticleBuffer != nullptr
		&& m_ParticleSRV != nullptr
		&& m_ParticleUAV != nullptr
		&& m_UpdateConstantBuffer != nullptr
		&& m_RenderConstantBuffer != nullptr
		&& m_VertexShader != nullptr
		&& m_PixelShader != nullptr
		&& m_ComputeShader != nullptr;
}

// 粒子の更新条件をComputeShaderへ渡す
void Particle::DispatchUpdate(
	float deltaFrame,
	const EmitRequest* emitRequest
)
{
	if (m_ComputeShader == nullptr ||
		m_ParticleBuffer == nullptr ||
		m_ParticleUAV == nullptr ||
		m_UpdateConstantBuffer == nullptr)
	{
		return;
	}

	UpdateConstantBuffer cb{};
	cb.EmitPosition = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	cb.EmitParam = XMFLOAT4(0.0f, 0.0f, 0.0f, static_cast<float>(m_MaxCount));
	cb.TimeParam = XMFLOAT4(m_Time, deltaFrame, 0.0f, 0.0f);
	cb.LifeSizeParam = XMFLOAT4(45.0f, 80.0f, 0.12f, 0.35f);
	cb.VelocityParam = XMFLOAT4(0.05f, 0.25f, -0.05f, 0.18f);
	cb.GravityParam = XMFLOAT4(-0.003f, 0.0f, 0.0f, 0.0f);

	if (emitRequest != nullptr)
	{
		int emitCount = std::max(0, emitRequest->Count);
		emitCount = std::min(emitCount, m_MaxCount);

		int emitStart = m_EmitCursor;
		int emitMode = static_cast<int>(emitRequest->Mode);

		cb.EmitPosition = XMFLOAT4(emitRequest->Position.x, emitRequest->Position.y, emitRequest->Position.z, 1.0f);

		switch (emitRequest->Mode)
		{
		case EmitMode::FireWork:
			cb.LifeSizeParam = XMFLOAT4(20.0f, 35.0f, 0.18f, 0.45f);
			cb.VelocityParam = XMFLOAT4(0.05f, 0.25f, -0.05f, 0.18f);
			cb.GravityParam = XMFLOAT4(-0.003f, 0.0f, 0.0f, 0.0f);
			break;

		case EmitMode::FloatUp:
			cb.LifeSizeParam = XMFLOAT4(80.0f, 150.0f, 0.50f, 1.10f);
			cb.VelocityParam = XMFLOAT4(0.01f, 0.05f, 0.03f, 0.10f);
			cb.GravityParam = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
			break;

		case EmitMode::Spark:
			cb.LifeSizeParam = XMFLOAT4(12.0f, 32.0f, 0.035f, 0.11f);
			cb.VelocityParam = XMFLOAT4(0.0f, 0.0f, 0.00f, 0.00f);
			cb.GravityParam = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
			break;

		case EmitMode::Landing:
			cb.LifeSizeParam = XMFLOAT4(10.0f, 26.0f, 0.04f, 0.13f);
			cb.VelocityParam = XMFLOAT4(0.12f, 0.45f, 0.00f, 0.06f);
			cb.GravityParam = XMFLOAT4(-0.010f, 0.0f, 0.0f, 0.0f);
			break;

		case EmitMode::Rain:
			cb.LifeSizeParam = XMFLOAT4(100.0f, 200.0f, 0.08f, 0.18f);
			cb.VelocityParam = XMFLOAT4(0.02f, 0.08f, -0.75f, -0.55f);
			cb.GravityParam = XMFLOAT4(-0.005f, 0.0f, 0.0f, 0.0f);
			break;

		default:
			cb.LifeSizeParam = XMFLOAT4(30.0f, 60.0f, 0.08f, 0.18f);
			cb.VelocityParam = XMFLOAT4(0.0f, 0.0f, 0.00f, 0.00f);
			cb.GravityParam = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
			break;
		}

		cb.EmitParam = XMFLOAT4(static_cast<float>(emitCount), static_cast<float>(emitStart), static_cast<float>(emitMode), static_cast<float>(m_MaxCount));

		m_EmitCursor += emitCount;
		m_EmitCursor %= m_MaxCount;
	}

	ID3D11DeviceContext* context = Renderer::GetDeviceContext();

	if (context == nullptr)
	{
		return;
	}

	context->UpdateSubresource(
		m_UpdateConstantBuffer,
		0,
		nullptr,
		&cb,
		0,
		0
	);

	context->CSSetShader(
		m_ComputeShader,
		nullptr,
		0
	);

	context->CSSetConstantBuffers(
		0,
		1,
		&m_UpdateConstantBuffer
	);

	context->CSSetUnorderedAccessViews(
		0,
		1,
		&m_ParticleUAV,
		nullptr
	);

	UINT groupCount = (m_MaxCount + 255) / 256;

	context->Dispatch(
		groupCount,
		1,
		1
	);

	ID3D11UnorderedAccessView* nullUAV = nullptr;

	context->CSSetUnorderedAccessViews(
		0,
		1,
		&nullUAV,
		nullptr
	);

	context->CSSetShader(
		nullptr,
		nullptr,
		0
	);
}

// 粒子バッファを使って一括描画する
void Particle::DrawParticles()
{
	Renderer::GetDeviceContext()->DrawInstanced(
		4,
		m_MaxCount,
		0,
		0
	);
}

// 粒子の発生要求を記録する
void Particle::AddParticleRequest(
	Vector3 position,
	int count,
	EmitMode mode
)
{
	if (m_EmitRequests.capacity() == 0)
	{
#ifdef _DEBUG
		OutputDebugStringA("[AddParticleRequest RETURN] Particle is not initialized. capacity == 0\n");
#endif
		return;
	}

	if (count <= 0)
	{
		return;
	}

	if (m_EmitRequests.size() >= 512)
	{
		return;
	}

	EmitRequest request{};
	request.Position = position;
	request.Count = count;
	request.Mode = mode;

	m_EmitRequests.push_back(request);

	m_DrawFrame = 180;
}

void Particle::SetTexture(const char* fileName)
{
	if (m_Texture != nullptr)
	{
		m_Texture = nullptr;
	}

	m_Texture = Texture::Load(fileName);
}

// 花火用の粒子を出す
void Particle::EmitFireWork(Vector3 position, int count)
{
#ifdef _DEBUG
	printf(
		"EmitFireWork count=%d max=%d request=%zu\n",
		count,
		m_MaxCount,
		m_EmitRequests.size()
	);
#endif
	AddParticleRequest(
		position,
		count,
		EmitMode::FireWork
	);
}

void Particle::EmitFloatUp(Vector3 position, int count)
{
	AddParticleRequest(
		position,
		count,
		EmitMode::FloatUp
	);
}

void Particle::EmitSpark(Vector3 position, int count)
{
	AddParticleRequest(
		position,
		count,
		EmitMode::Spark
	);
}

// 着地用の粒子を出す
void Particle::EmitLanding(Vector3 position, int count)
{
	AddParticleRequest(
		position,
		count,
		EmitMode::Landing
	);
}

void Particle::EmitRain(Vector3 position, int count)
{
	AddParticleRequest(
		position,
		count,
		EmitMode::Rain
	);
}

void Particle::Clear()
{
	std::vector<ParticleGpuData> clearData;
	clearData.resize(m_MaxCount);

	for (int i = 0; i < m_MaxCount; i++)
	{
		clearData[i].PositionLife = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
		clearData[i].VelocityMaxLife = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
		clearData[i].ColorSize = XMFLOAT4(1.0f, 1.0f, 1.0f, 0.1f);
		clearData[i].Param = XMFLOAT4(0.0f, 0.0f, 1.0f, 0.0f);
	}

	if (m_ParticleBuffer != nullptr)
	{
		Renderer::GetDeviceContext()->UpdateSubresource(
			m_ParticleBuffer,
			0,
			nullptr,
			clearData.data(),
			0,
			0
		);
	}

	m_EmitCursor = 0;
	m_DrawFrame = 0;
	m_EmitRequests.clear();
}

void Particle::ReleaseResource()
{
	if (m_VertexBuffer != nullptr)
	{
		m_VertexBuffer->Release();
		m_VertexBuffer = nullptr;
	}

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

	if (m_ComputeShader != nullptr)
	{
		m_ComputeShader->Release();
		m_ComputeShader = nullptr;
	}

	if (m_Texture != nullptr)
	{
		m_Texture = nullptr;
	}

	if (m_ParticleBuffer != nullptr)
	{
		m_ParticleBuffer->Release();
		m_ParticleBuffer = nullptr;
	}

	if (m_ParticleSRV != nullptr)
	{
		m_ParticleSRV->Release();
		m_ParticleSRV = nullptr;
	}

	if (m_ParticleUAV != nullptr)
	{
		m_ParticleUAV->Release();
		m_ParticleUAV = nullptr;
	}

	if (m_UpdateConstantBuffer != nullptr)
	{
		m_UpdateConstantBuffer->Release();
		m_UpdateConstantBuffer = nullptr;
	}

	if (m_RenderConstantBuffer != nullptr)
	{
		m_RenderConstantBuffer->Release();
		m_RenderConstantBuffer = nullptr;
	}
}

#ifdef _DEBUG
// 有効な粒子数をデバッグ出力する
void Particle::DebugPrintParticleCount()
{
	auto GetModeName = [](EmitMode mode) -> const char*
		{
			switch (mode)
			{
			case EmitMode::FireWork:
				return "FireWork";

			case EmitMode::FloatUp:
				return "FloatUp";

			case EmitMode::Spark:
				return "Spark";

			case EmitMode::Landing:
				return "Landing";

			case EmitMode::Rain:
				return "Rain";

			default:
				return "Unknown";
			}
		};

	int fireWorkRequestCount = 0;
	int floatUpRequestCount = 0;
	int sparkRequestCount = 0;
	int landingRequestCount = 0;
	int rainRequestCount = 0;

	for (const EmitRequest& request : m_EmitRequests)
	{
		switch (request.Mode)
		{
		case EmitMode::FireWork:
			fireWorkRequestCount++;
			break;

		case EmitMode::FloatUp:
			floatUpRequestCount++;
			break;

		case EmitMode::Spark:
			sparkRequestCount++;
			break;

		case EmitMode::Landing:
			landingRequestCount++;
			break;

		case EmitMode::Rain:
			rainRequestCount++;
			break;

		default:
			break;
		}
	}

	const char* lastModeName = "NoRequest";

	if (!m_EmitRequests.empty())
	{
		lastModeName =
			GetModeName(
				m_EmitRequests.back().Mode
			);
	}

	if (m_ParticleBuffer == nullptr)
	{
		return;
	}

	ID3D11Device* device =
		Renderer::GetDevice();

	ID3D11DeviceContext* context =
		Renderer::GetDeviceContext();

	if (device == nullptr || context == nullptr)
	{
		return;
	}

	D3D11_BUFFER_DESC desc{};
	m_ParticleBuffer->GetDesc(
		&desc
	);

	D3D11_BUFFER_DESC stagingDesc =
		desc;

	stagingDesc.Usage = D3D11_USAGE_STAGING;
	stagingDesc.BindFlags = 0;
	stagingDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
	stagingDesc.MiscFlags = 0;

	ID3D11Buffer* stagingBuffer = nullptr;

	HRESULT hr =
		device->CreateBuffer(
			&stagingDesc,
			nullptr,
			&stagingBuffer
		);

	if (FAILED(hr) || stagingBuffer == nullptr)
	{
		return;
	}

	context->CopyResource(
		stagingBuffer,
		m_ParticleBuffer
	);

	D3D11_MAPPED_SUBRESOURCE mapped{};

	hr =
		context->Map(
			stagingBuffer,
			0,
			D3D11_MAP_READ,
			0,
			&mapped
		);

	if (FAILED(hr))
	{
		stagingBuffer->Release();
		return;
	}

	ParticleGpuData* particles =
		reinterpret_cast<ParticleGpuData*>(mapped.pData);

	int activeCount = 0;
	int deadCount = 0;
	int lowLifeCount = 0;

	for (int i = 0; i < m_MaxCount; i++)
	{
		if (particles[i].Param.x > 0.5f)
		{
			activeCount++;

			if (particles[i].PositionLife.w <= 10.0f)
			{
				lowLifeCount++;
			}
		}
		else
		{
			deadCount++;
		}
	}

	context->Unmap(
		stagingBuffer,
		0
	);

	stagingBuffer->Release();

	printf(
		"[ParticleDebug:%s] active=%d dead=%d lowLife=%d max=%d cursor=%d request=%zu FW=%d Float=%d Spark=%d Landing=%d Rain=%d\n",
		lastModeName,
		activeCount,
		deadCount,
		lowLifeCount,
		m_MaxCount,
		m_EmitCursor,
		m_EmitRequests.size(),
		fireWorkRequestCount,
		floatUpRequestCount,
		sparkRequestCount,
		landingRequestCount,
		rainRequestCount
	);
}
#endif