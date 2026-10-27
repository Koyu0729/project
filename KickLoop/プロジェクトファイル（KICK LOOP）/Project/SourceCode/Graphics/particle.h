#pragma once

#include "gameObject.h"
#include "vector3.h"

#include <vector>

// GPU上のStructuredBufferで粒子情報を保持し、ComputeShaderで更新するパーティクル。
// 描画時は粒子バッファをVertexShaderへ渡し、インスタンシングでビルボードとして描画する。
class Particle : public GameObject
{
private:
	static const int PARTICLE_MAX = 5000;

private:
	// ComputeShader側へ渡す発生パターン。
	enum class EmitMode
	{
		FireWork = 0,
		FloatUp,
		Spark,
		Landing,
		Rain,
	};

	// StructuredBufferに格納する1粒子分の情報。
	struct ParticleGpuData
	{
		XMFLOAT4 PositionLife;       // xyz = position, w = life
		XMFLOAT4 VelocityMaxLife;    // xyz = velocity, w = maxLife
		XMFLOAT4 ColorSize;          // xyz = color, w = size
		XMFLOAT4 Param;              // x = active, y = swirl, z = drag, w = pulse
	};

	// 粒子更新と発生時にComputeShaderへ渡す値。
	struct UpdateConstantBuffer
	{
		XMFLOAT4 EmitPosition;       // xyz = emit position
		XMFLOAT4 EmitParam;          // x = count, y = start index, z = mode, w = max count
		XMFLOAT4 TimeParam;          // x = time, y = delta frame
		XMFLOAT4 LifeSizeParam;      // x = life min, y = life max, z = size min, w = size max
		XMFLOAT4 VelocityParam;      // x = speed min, y = speed max, z = y min, w = y max
		XMFLOAT4 GravityParam;       // x = gravity
	};

	// ビルボード描画用に、カメラ行列とカメラのRight / Upを渡す。
	struct RenderConstantBuffer
	{
		XMMATRIX ViewProjection;
		XMFLOAT4 CameraRight;
		XMFLOAT4 CameraUp;
	};

	// 1フレーム中に発生した粒子発生要求。
	struct EmitRequest
	{
		Vector3 Position;
		int Count = 0;
		EmitMode Mode = EmitMode::FireWork;
	};

private:
	ID3D11Buffer* m_VertexBuffer = nullptr;

	ID3D11InputLayout* m_VertexLayout = nullptr;
	ID3D11VertexShader* m_VertexShader = nullptr;
	ID3D11PixelShader* m_PixelShader = nullptr;

	// 粒子に貼るテクスチャ。
	ID3D11ShaderResourceView* m_Texture = nullptr;

	// 粒子の位置・速度・寿命をGPU上で更新するComputeShader。
	ID3D11ComputeShader* m_ComputeShader = nullptr;

	// 全粒子の位置・速度・寿命・色・サイズを保持するBuffer。
	ID3D11Buffer* m_ParticleBuffer = nullptr;

	// 描画時にVertexShaderから粒子データを読み込むためのSRV（入力）。
	ID3D11ShaderResourceView* m_ParticleSRV = nullptr;

	// 更新時にComputeShaderから粒子データを書き込むためのUAV（出力）。
	ID3D11UnorderedAccessView* m_ParticleUAV = nullptr;

	// ComputeShaderへ、粒子更新用のパラメータを渡す。
	ID3D11Buffer* m_UpdateConstantBuffer = nullptr;

	// VertexShaderへ、ViewProjection行列とビルボード用のCameraRight / CameraUpを渡す。
	ID3D11Buffer* m_RenderConstantBuffer = nullptr;

	// EmitCursorでリングバッファのように粒子発生位置を進める。
	int m_MaxCount = PARTICLE_MAX;
	int m_EmitCursor = 0;
	int m_DrawFrame = 0;

	float m_Time = 0.0f;

	std::vector<EmitRequest> m_EmitRequests;

public:
	void Init() override;
	void Uninit() override;
	void Update() override;
	void Draw() override;

	bool IsReady() const;

	void Clear();

#ifdef _DEBUG
	void DebugPrintParticleCount();
#endif

	void SetTexture(const char* fileName);

	void EmitFireWork(Vector3 position, int count);
	void EmitFloatUp(Vector3 position, int count);

	void EmitSpark(Vector3 position, int count);
	void EmitLanding(Vector3 position, int count);

	void EmitRain(Vector3 position, int count);

private:
	void ReleaseResource();

	void DispatchUpdate(
		float deltaFrame,
		const EmitRequest* emitRequest
	);

	void DrawParticles();

	void AddParticleRequest(
		Vector3 position,
		int count,
		EmitMode mode
	);


};
