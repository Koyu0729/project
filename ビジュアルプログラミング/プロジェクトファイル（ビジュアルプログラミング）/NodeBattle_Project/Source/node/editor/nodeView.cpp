#include "main.h"
#include "nodeView.h"
#include "polyTex.h"
#include "manager.h"
#include "scene.h"

// 初期化処理を行う
void NodeView::Init(NodeBase *node)
{
	m_Position = Vector2(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2);

	m_Node = node;

	m_Category = node->GetCategory();

	Vector2 mouse = Mouse::Pos();

	switch (m_Category)
	{
	case NodeCategory::Start:
		m_BaseColor = D2D1::ColorF(D2D1::ColorF::SpringGreen);

		break;

	case NodeCategory::If:
		m_BaseColor = D2D1::ColorF(D2D1::ColorF::DeepSkyBlue);

		break;

	case NodeCategory::AttackMelee:
		m_BaseColor = D2D1::ColorF(D2D1::ColorF::Crimson);

		break;

	case NodeCategory::AttackRange:
		m_BaseColor = D2D1::ColorF(D2D1::ColorF::Orange);

		break;

	case NodeCategory::Action:
		m_BaseColor = D2D1::ColorF(D2D1::ColorF::MediumPurple);

		break;

	case NodeCategory::Movement:
		m_BaseColor = D2D1::ColorF(D2D1::ColorF::LimeGreen);

		break;

	case NodeCategory::Rotate:
		m_BaseColor = D2D1::ColorF(D2D1::ColorF::DodgerBlue);

		break;

	default:
		m_BaseColor = D2D1::ColorF(D2D1::ColorF::Gray);

		break;
	}

	m_CurrentColor = m_BaseColor;

	nodeRect = Manager::GetScene()->AddGameObject2D<PolyTex>(1);

	nodeRect->SetTexRule(
		m_Position,
		0.0f,
		Vector2(RECT_WIDTH, RECT_HEIGHT),
		"asset\\texture\\node\\rectangle.png");

	nodeRect->SetName(L"NodeRect");

	AddChild(nodeRect);

	nodeRect->AddHitBox();

	Vector2 nodePos = nodeRect->GetPosition();

	Vector2 nodeScale = nodeRect->GetScale();

	Vector2 center(
		nodePos.x + nodeScale.x * 0.5f,
		nodePos.y + nodeScale.y * 0.38f);

	if (m_Category != NodeCategory::Start)
	{
		CreatePort(
			nodeIn,
			L"NodeIn",
			center.x - nodeScale.x * IN_PORT_POSX,
			center.y);
	}

	if (m_Category == NodeCategory::If)
	{
		CreatePort(
			nodeOutTrue,
			L"NodeOutTrue",
			center.x + nodeScale.x * OUT_PORT_POSX,
			center.y - nodeScale.y * OUT_IF_PORT_POSY,
			"asset\\texture\\node\\null.png");

		CreatePort(
			nodeOutFalse,
			L"NodeOutFalse",
			center.x + nodeScale.x * OUT_PORT_POSX,
			center.y + nodeScale.y * OUT_IF_PORT_POSY,
			"asset\\texture\\node\\null.png");
	}
	else
	{
		CreatePort(
			nodeOutDef,
			L"NodeOut",
			center.x + nodeScale.x * OUT_PORT_POSX,
			center.y);
	}

	// 文字初期設定
	m_Font.font = L"メイリオ";

	m_Font.fontSize = 32.0f;

	m_Font.Color = D2D1::ColorF(D2D1::ColorF::White);

	m_Font.shadowColor = D2D1::ColorF(D2D1::ColorF::Black);

	m_Font.shadowOffset = D2D1::Point2F(2, 2);
}

// 終了処理を行う
void NodeView::Uninit()
{
	// ノード矩形の破棄

	if (nodeRect)
	{
		nodeRect->SetDestroy();

		nodeRect = nullptr;
	}

	// ポートの破棄

	if (nodeIn)
	{
		nodeIn->SetDestroy();

		nodeIn = nullptr;
	}

	if (nodeOutDef)
	{
		nodeOutDef->SetDestroy();

		nodeOutDef = nullptr;
	}

	if (nodeOutTrue)
	{
		nodeOutTrue->SetDestroy();

		nodeOutTrue = nullptr;
	}

	if (nodeOutFalse)
	{
		nodeOutFalse->SetDestroy();

		nodeOutFalse = nullptr;
	}

	// 状態リセット
	m_IsDragging = false;

	m_Children.clear();

	m_Node = nullptr;
}

// 毎フレームの処理
void NodeView::Update()
{
	if (nodeRect)
	{
		XMFLOAT4 color = XMFLOAT4(m_CurrentColor.r, m_CurrentColor.g, m_CurrentColor.b, m_CurrentColor.a);

		nodeRect->SetColor(color); 
	}
}

// 画面に描く処理
void NodeView::Draw()
{
	if (!m_Node || !nodeRect)
	{
		return;
	}

	if (!GetVisible())
	{
		return;
	}

	Vector2 rectPos = nodeRect->GetPosition();

	Vector2 rectScale = nodeRect->GetScale();

	D2D1_POINT_2F textPos = D2D1::Point2F(
		rectPos.x + rectScale.x * 0.05f, // ノード左上から横方向へ配置
		rectPos.y + rectScale.y * 0.20f	 // ノード左上から縦方向へ配置
	);

	float nameLength = (float)GetNode()->GetName().size();

	m_Font.fontSize = (std::min)(18.0f, 180.0f / (std::max)(1.0f, nameLength));

	if (m_Font.fontSize < 12.0f)
	{
		m_Font.fontSize = 12.0f; // 最小サイズ制限
	}

	if (m_Font.fontSize > 48.0f)
	{
		m_Font.fontSize = 48.0f;
	}

	Moji::Instance().SetFont(m_Font);

	Moji::Instance().Draw(
		GetNode()->GetName(),
		textPos,
		1.0f * m_Zoom,
		D2D1_DRAW_TEXT_OPTIONS_NONE,
		true);

	m_Font.fontSize = 13.0f;

	Moji::Instance().SetFont(m_Font);

	Moji::Instance().Draw(GetNode()->GetParameterSummary(), D2D1::Point2F(rectPos.x + rectScale.x * 0.05f, rectPos.y + rectScale.y * 0.47f), m_Zoom, D2D1_DRAW_TEXT_OPTIONS_NONE, true);

	if (m_Category == NodeCategory::If)
	{
		m_Font.fontSize = 15.0f;

		Moji::Instance().SetFont(m_Font);

		Moji::Instance().Draw(L"はい", D2D1::Point2F(rectPos.x + rectScale.x * 0.78f, rectPos.y + rectScale.y * 0.02f), m_Zoom, D2D1_DRAW_TEXT_OPTIONS_NONE, false);

		Moji::Instance().Draw(L"いいえ", D2D1::Point2F(rectPos.x + rectScale.x * 0.72f, rectPos.y + rectScale.y * 0.72f), m_Zoom, D2D1_DRAW_TEXT_OPTIONS_NONE, false);
	}
}

// ノードエディターにポートの当たり判定を渡す

// マウスが接続口の上にあるか調べる
bool NodeView::InsidePort(Port port)
{
	PolyTex *target = nullptr;

	switch (port)
	{
	case Port::In:
		target = nodeIn;

		break;

	case Port::OutDef:
		target = nodeOutDef;

		break;

	case Port::OutTrue:
		target = nodeOutTrue;

		break;

	case Port::OutFalse:
		target = nodeOutFalse;

		break;
	}

	// 接続口が存在しない場合は処理を終了する

	if (!target)
	{
		return false;
	}

	Vector2 targetPos = target->GetPosition();

	Vector2 targetSize = target->GetScale();

	return (target->GetHitBox()->OverlapAABB());
}

// 入力・出力の接続口を作る
void NodeView::CreatePort(PolyTex *&port, const std::wstring &name, float x, float y, const char *tex)
{
	port = Manager::GetScene()->AddGameObject2D<PolyTex>(1);

	port->AddHitBox();

	port->SetTexRule(
		Vector2(x, y),
		0.0f,
		Vector2(PORT_WIDTH, PORT_HEIGHT),
		tex);

	port->SetName(name);

	if (name == L"NodeOutTrue")
	{
		port->SetColor(XMFLOAT4(0.25f, 1.0f, 0.5f, 1));
	}
	else if (name == L"NodeOutFalse")
	{
		port->SetColor(XMFLOAT4(1.0f, 0.4f, 0.3f, 1));
	}

	AddChild(port);
}

// 拡大率と画面の移動に合わせて、ノードと接続口を動かす
void NodeView::UpdateView(float zoom, const Vector2 &offset, bool allowDrag)
{
	Vector2 mousePos = Mouse::Pos();

	m_Zoom = zoom;

	Vector2 screenPos = m_Position * m_Zoom + offset;

	// ドラッグ開始

	if (allowDrag && nodeRect->GetHitBox()->Clicked() && !m_IsDragging)
	{
		m_IsDragging = true;

		m_DragOffset = mousePos - screenPos;
	}

	// ドラッグ終了

	if (nodeRect->GetHitBox()->Released())
	{
		m_IsDragging = false;
	}

	// 移動処理

	if (!allowDrag)
	{
		m_IsDragging = false;
	}

	if (m_IsDragging)
	{
		m_Position = (mousePos - m_DragOffset - offset) / m_Zoom;
	}

	// 描画系更新
	screenPos = m_Position * m_Zoom + offset;

	nodeRect->SetPosition(screenPos);

	nodeRect->SetScale(Vector2(RECT_WIDTH, RECT_HEIGHT) * m_Zoom);

	nodeRect->GetHitBox()->TransCenter();

	nodeRect->GetHitBox()->SetBox(screenPos, nodeRect->GetScale());

	Vector2 rectPos = nodeRect->GetPosition();

	Vector2 rectSize = nodeRect->GetScale();

	Vector2 center(
		rectPos.x + rectSize.x * 0.5f,
		rectPos.y + rectSize.y * 0.38f);

	if (m_Category != NodeCategory::Start && nodeIn)
	{
		nodeIn->SetPosition(Vector2(center.x - rectSize.x * IN_PORT_POSX, center.y));

		nodeIn->SetScale(Vector2(PORT_WIDTH, PORT_HEIGHT) * m_Zoom);

		nodeIn->GetHitBox()->TransCenter();

		nodeIn->GetHitBox()->SetBox(nodeIn->GetPosition(), nodeIn->GetScale());
	}

	if (m_Category == NodeCategory::If)
	{
		nodeOutTrue->SetPosition(Vector2(center.x + rectSize.x * OUT_PORT_POSX, center.y - rectSize.y * OUT_IF_PORT_POSY));

		nodeOutTrue->SetScale(Vector2(PORT_WIDTH, PORT_HEIGHT) * m_Zoom);

		nodeOutTrue->GetHitBox()->TransCenter();

		nodeOutTrue->GetHitBox()->SetBox(nodeOutTrue->GetPosition(), nodeOutTrue->GetScale());

		nodeOutFalse->SetPosition(Vector2(center.x + rectSize.x * OUT_PORT_POSX, center.y + rectSize.y * OUT_IF_PORT_POSY));

		nodeOutFalse->SetScale(Vector2(PORT_WIDTH, PORT_HEIGHT) * m_Zoom);

		nodeOutFalse->GetHitBox()->TransCenter();

		nodeOutFalse->GetHitBox()->SetBox(nodeOutFalse->GetPosition(), nodeOutFalse->GetScale());
	}
	else if (nodeOutDef)
	{
		nodeOutDef->SetPosition(Vector2(center.x + rectSize.x * OUT_PORT_POSX, center.y));

		nodeOutDef->SetScale(Vector2(PORT_WIDTH, PORT_HEIGHT) * m_Zoom);

		nodeOutDef->GetHitBox()->TransCenter();

		nodeOutDef->GetHitBox()->SetBox(nodeOutDef->GetPosition(), nodeOutDef->GetScale());
	}
}

// 線をつなぐために、接続口の中心を返す
Vector2 NodeView::GetPortCenter(Port port)
{
	PolyTex *target = nullptr;

	switch (port)
	{
	case Port::In:
		target = nodeIn;

		break;

	case Port::OutDef:
		target = nodeOutDef;

		break;

	case Port::OutTrue:
		target = nodeOutTrue;

		break;

	case Port::OutFalse:
		target = nodeOutFalse;

		break;
	}

	// 接続口が存在しない場合は処理を終了する

	if (!target)
	{
		return Vector2(0, 0);
	}

	Vector2 p = target->GetPosition();

	Vector2 s = target->GetScale();

	return Vector2(p.x + s.x * 0.5f, p.y + s.y * 0.5f);
}

// 選択中や実行中のノードを目立たせる
void NodeView::SetHighlighted(bool highlight)
{
	if (highlight)
	{
		m_CurrentColor = m_HighlightColor;
	}
	else
	{
		m_CurrentColor = m_BaseColor;
	}

	if (nodeRect)
	{
		XMFLOAT4 color(
			m_CurrentColor.r,
			m_CurrentColor.g,
			m_CurrentColor.b,
			m_CurrentColor.a);

		nodeRect->SetColor(color);
	}
}
