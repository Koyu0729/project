// 始点と終点を結ぶベジェ曲線で、ノードの接続を表示する
// 制御点は始点と終点から水平方向に離して配置する
// 参考：GYのコードを基にした曲線描画
#pragma once
#include "gameObject2D.h"

// ベジェ曲線を描画するクラス
class DrawLine : public GameObject2D
{
private:
    ID3D11Buffer* m_VertexBuffer;

    ID3D11InputLayout* m_VertexLayout;
    ID3D11VertexShader* m_VertexShader;
    ID3D11PixelShader* m_PixelShader;

    Vector2 m_P0 ={}; // 始点
    Vector2 m_P1 ={}; // 制御点1
    Vector2 m_P2 ={}; // 制御点2
    Vector2 m_P3 ={}; // 終点

    XMFLOAT4 m_Color = { 0,0,0,1 };

public:
    void Init() override;
    void Uninit() override;
    void Update() override;
    void Draw() override;

    // 始点と終点を受け取り、曲線の制御点を設定する
    void SetPoints(const Vector2& p0, const Vector2& p3);
    void SetColor(const XMFLOAT4& color);
};

