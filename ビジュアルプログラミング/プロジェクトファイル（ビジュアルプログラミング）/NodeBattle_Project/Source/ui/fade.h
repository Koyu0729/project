#pragma once

class Fade
{
  public:
	enum State
	{
		None,
		FadeIn,
		FadeOut
	};

  private:
	State m_State = None;
	float m_Time = 0.0f;
	float m_Timer = 0.0f;
	float m_Alpha = 0.0f;
	bool m_Finished = true;

	ID3D11Buffer *m_VertexBuffer = nullptr;
	ID3D11InputLayout *m_VertexLayout = nullptr;
	ID3D11VertexShader *m_VertexShader = nullptr;
	ID3D11PixelShader *m_PixelShader = nullptr;

	ID3D11ShaderResourceView *m_Texture = nullptr;

  public:
	void Init();
	void Uninit();
	void Update();
	void Draw();

	void StartFadeIn(float time);
	void StartFadeOut(float time);

	bool IsFinished() const
	{
		return m_Finished;
	}
	State GetState() const
	{
		return m_State;
	}
};