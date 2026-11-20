#pragma once

struct VERTEX_3D

{

	XMFLOAT3 Position;

	XMFLOAT3 Normal;

	XMFLOAT4 Diffuse;

	XMFLOAT2 TexCoord;
};

struct MATERIAL

{

	XMFLOAT4 Ambient;

	XMFLOAT4 Diffuse;

	XMFLOAT4 Specular;

	XMFLOAT4 Emission;

	float Shininess;

	BOOL TextureEnable;

	float Dummy[2];
};

struct LIGHT

{

	BOOL Enable;

	BOOL Dummy[3];

	XMFLOAT4 Direction;

	XMFLOAT4 Diffuse;

	XMFLOAT4 Ambient;
};

// 描画デバイスと、描画に共通する設定を管理する
class Renderer 

{

  private:
	static D3D_FEATURE_LEVEL m_FeatureLevel;

	static ID3D11Device *m_Device;

	static ID3D11DeviceContext *m_DeviceContext;

	static IDXGISwapChain *m_SwapChain;

	static ID3D11RenderTargetView *m_RenderTargetView;

	static ID3D11DepthStencilView *m_DepthStencilView;

	static ID3D11Buffer *m_WorldBuffer;

	static ID3D11Buffer *m_ViewBuffer;

	static ID3D11Buffer *m_ProjectionBuffer;

	static ID3D11Buffer *m_MaterialBuffer;

	static ID3D11Buffer *m_LightBuffer;

	static ID3D11DepthStencilState *m_DepthStateEnable;

	static ID3D11DepthStencilState *m_DepthStateDisable;

	static ID3D11BlendState *m_BlendState;

	static ID3D11BlendState *m_BlendStateATC;

	static ID3D11BlendState *m_BlendStateAdd; // 加算合成用

  public:
	// 初期化処理を行う
	static void Init();

	// 終了処理を行う
	static void Uninit();

	// 画面と深度バッファをクリアする
	static void Begin();

	// 描画した画面を表示する
	static void End();

	// この画面の背景色を変える
	static void ClearBackground(float r, float g, float b)
	{
		float color[4] = {r, g, b, 1};

		m_DeviceContext->ClearRenderTargetView(m_RenderTargetView, color);
	}

	// 描画した画面を画像ファイルへ保存する
	static bool SaveFrame(const wchar_t *path);

	static void CreateVertexShader(ID3D11VertexShader **VertexShader, ID3D11InputLayout **VertexLayout, const char *FileName);

	static void CreatePixelShader(ID3D11PixelShader **PixelShader, const char *FileName);

	static void SetDepthEnable(bool Enable);

	static void SetATCEnable(bool Enable);

	static void SetAddBlendEnable(bool Enable);

	static void SetWorldViewProjection2D();

	static void SetWorldMatrix(XMMATRIX WorldMatrix);

	static void SetViewMatrix(XMMATRIX ViewMatrix);

	static void SetProjectionMatrix(XMMATRIX ProjectionMatrix);

	static void SetMaterial(MATERIAL Material);

	static void SetLight(LIGHT Light);

	static ID3D11Device *GetDevice(void)
	{
		return m_Device;
	}

	static ID3D11DeviceContext *GetDeviceContext(void)
	{
		return m_DeviceContext;
	}
};
