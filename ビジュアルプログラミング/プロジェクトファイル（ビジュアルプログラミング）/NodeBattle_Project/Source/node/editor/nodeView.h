// ノード本体に対応する編集画面の表示を管理する
#pragma once
#include "gameObject2D.h"
#include "nodeBase.h"
#include "moji.h"

#define IN_PORT_POSX (0.65f)
#define OUT_PORT_POSX (0.55f)
#define OUT_IF_PORT_POSY (0.2f)
#define RECT_WIDTH (200.0f)
#define RECT_HEIGHT (100.0f)
#define PORT_WIDTH (20.0f)
#define PORT_HEIGHT (20.0f)
class PolyTex;

class NodeView : public GameObject2D
{
  private:
#pragma region 描画用

	PolyTex *nodeRect = nullptr;

	PolyTex *nodeIn = nullptr;

	PolyTex *nodeOutDef = nullptr;

	PolyTex *nodeOutTrue = nullptr;

	PolyTex *nodeOutFalse = nullptr;

	FontData m_Font;

	D2D1::ColorF m_BaseColor = D2D1::ColorF(D2D1::ColorF::Gray);

	D2D1::ColorF m_HighlightColor = D2D1::ColorF(D2D1::ColorF::Gold);

	D2D1::ColorF m_CurrentColor = m_BaseColor;

#pragma endregion

#pragma region マウスの変数

	bool m_IsDragging = false; // ドラッグしているか
	Vector2 m_DragOffset;	   // ドラッグして移動した分の値

	// 拡大・縮小用
	float m_Zoom = 1.0f;

#pragma endregion

	// この見た目に対応する、ノード本体
	NodeBase *m_Node = nullptr;

	NodeCategory m_Category = NodeCategory::Category_Null;

  public:
	// 初期化処理を行う
	void Init(NodeBase *node);

	// 初期化処理を行う
	void Init() override {};

	// 終了処理を行う
	void Uninit() override;

	// 毎フレームの処理
	void Update() override;

	// 画面に描く処理
	void Draw() override;

	// 入力・出力の接続口を作る
	void CreatePort(
		PolyTex *&port,
		const std::wstring &name,
		float x = 0, float y = 0,
		const char *tex = "asset\\texture\\node\\null.png");

	void ZoomNodeView(float zoom)
	{
		m_Zoom = zoom;
	};

	// マウスが接続口の上にあるか調べる
	bool InsidePort(Port port);

	// 拡大率と画面の移動に合わせて、ノードと接続口を動かす
	void UpdateView(float zoom, const Vector2 &offset, bool allowDrag = true);

	// ノードの本体
	PolyTex *GetRect() const
	{
		return nodeRect;
	}

	// 線をつなぐために、接続口の中心を返す
	Vector2 GetPortCenter(Port port);

	// 表示に対応するノード本体を取得する
	NodeBase *GetNode() const
	{
		return m_Node;
	}

	// 選択中や実行中のノードを目立たせる
	void SetHighlighted(bool on);
};
