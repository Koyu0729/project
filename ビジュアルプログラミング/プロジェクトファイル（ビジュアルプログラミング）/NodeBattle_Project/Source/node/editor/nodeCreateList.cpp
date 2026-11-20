// ノードエディターの最初に呼ぶ（右クリックで表示）
#include "main.h"
#include "nodeCreateList.h"
#include "polyTex.h"
#include "manager.h"
#include "scene.h"
#include "textObject.h"

// 初期化処理を行う
void NodeCreateList::Init()
{
	m_Position = Vector2(10.0f, CREATE_LIST_TOP);

	m_Scale = Vector2(CREATE_LIST_SIZE_X, SCREEN_HEIGHT - CREATE_LIST_TOP);

	m_ScrollOffset = 0.0f;

	m_Visible = true;

	m_Select = false;

	m_ListFront = Manager::GetScene()->AddGameObject2D<PolyTex>(9);

	m_ListFront->SetTexRule(
		m_Position,
		0.0f,
		m_Scale,
		"asset\\texture\\listFront.png");

	m_ListFront->SetColor(XMFLOAT4(1, 1, 1, 1.0f));

	m_ListBack = Manager::GetScene()->AddGameObject2D<PolyTex>(7);

	m_ListBack->SetTexRule(
		m_Position,
		0.0f,
		m_Scale,
		"asset\\texture\\listBack.png");

	m_ListBack->SetColor(XMFLOAT4(1, 1, 0, 0.0f));

	// 一覧の項目を配置するY座標
	float y = m_Position.y + CATEGORY_SHIFT_Y;

	// カテゴリーごとにノード一覧を作成する

	for (int categoryIndex = 0; categoryIndex < (int)NodeCategory::Category_MAX; ++categoryIndex)
	{
		const auto &it = *NodeTable.find((NodeCategory)categoryIndex);
		NodeCategory category = it.first;
		auto &typeList = it.second;

		// カテゴリーの見出しを作成する
		{
			// 格納するオブジェクトの生成
			TextObject *categoryText = Manager::GetScene()->AddGameObject2D<TextObject>(8);
			// フォントの設定
			FontData categoryFontData;

			categoryFontData.font = L"メイリオ";

			categoryFontData.fontSize = 32.0f;

			categoryFontData.Color = D2D1::ColorF(D2D1::ColorF::White);

			categoryFontData.shadowColor = D2D1::ColorF(D2D1::ColorF::Black);

			categoryFontData.shadowOffset = D2D1::Point2F(2, 2);
			// 上のフォント設定を反映
			categoryText->SetFont(categoryFontData);

			// ノードの名前を反映
			categoryText->SetText(GetCategoryName(category));

			// 場所、サイズ
			categoryText->SetPosition(Vector2(m_Position.x + CATEGORY_SHIFT_X, y));

			categoryText->SetScale(Vector2(CREATE_LIST_SIZE_X - CATEGORY_SHIFT_X, TEXT_SIZE_Y));

			// ノードの情報を持つ
			categoryText->SetCategory(category);

			categoryText->SetType(NodeType::Type_Null);

			// 当たり判定を追加
			categoryText->AddHitBox();

			// テキストを管理
			m_Texts.push_back(categoryText);

			y += CATEGORY_SHIFT_Y;
		}

		for (int j = 0; j < (int)typeList.size(); j++)
		{
			const auto &tL = typeList[j];

			TextObject *typeText = Manager::GetScene()->AddGameObject2D<TextObject>(8);

			FontData TypeFontData;

			TypeFontData.font = L"メイリオ";

			TypeFontData.fontSize = 20.0f;

			TypeFontData.Color = D2D1::ColorF(D2D1::ColorF::White);

			TypeFontData.shadowColor = D2D1::ColorF(D2D1::ColorF::Black);

			TypeFontData.shadowOffset = D2D1::Point2F(2, 2);

			// 上のフォント設定を反映
			typeText->SetFont(TypeFontData);

			// ノードの名前を反映
			typeText->SetText(tL.name);

			// 場所、サイズ
			typeText->SetPosition(Vector2(m_Position.x + TYPE_SHIFT_X, y));

			typeText->SetScale(Vector2(CREATE_LIST_SIZE_X - TYPE_SHIFT_X, TEXT_SIZE_Y));

			// ノードの情報を持つ
			typeText->SetCategory(category);

			typeText->SetType(tL.type);

			// 当たり判定を追加
			typeText->AddHitBox();

			typeText->GetHitBox()->TransCenter();

			typeText->GetHitBox()->SetBox(typeText->GetPosition(), typeText->GetScale());

			// テキストを管理
			m_Texts.push_back(typeText);

			y += TYPE_SHIFT_Y;
		}
	}

	AddHitBox(true, true);

	if (m_Hit)
	{
		m_Hit->SetBlockSelect(true);
	}
}

// 終了処理を行う
void NodeCreateList::Uninit()
{
	m_Visible = false;

	if (m_ListFront)
	{
		m_ListFront->SetDestroy();

		m_ListFront = nullptr;
	}

	if (m_ListBack)
	{
		m_ListBack->SetDestroy();

		m_ListBack = nullptr;
	}

	// 一覧に登録した文字オブジェクトを削除対象に設定する

	for (auto &td : m_Texts)
	{
		if (td)
		{
			td->SetDestroy();

			td = nullptr;
		}
	}

	m_Texts.clear();
}

// 毎フレームの処理
void NodeCreateList::Update()
{
	if (!m_Visible)
	{
		if (m_ListFront)
		{
			m_ListFront->SetVisible(false);
		}

		if (m_ListBack)
		{
			m_ListBack->SetVisible(false);
		}

		for (auto &text : m_Texts)
		{
			if (text)
			{
				text->SetVisible(false);
			}
		}

		return;
	}
	else
	{
		if (m_ListFront)
		{
			m_ListFront->SetVisible(true);
		}

		if (m_ListBack)
		{
			m_ListBack->SetVisible(true);
		}

		// ノードクリエイトリストが有効であれば

		for (auto &t : m_Texts)
		{
			if (t)
			{
				t->SetVisible(true);
			}
		}
	}

	if (!m_ListFront || !m_ListBack)
	{
		return;
	}

	// スクロール処理
	float delta = 0.0f;

	Vector2 mp = Mouse::Pos();

	bool inList =
		mp.x >= m_Position.x &&
		mp.x <= m_Position.x + m_Scale.x &&
		mp.y >= m_Position.y &&
		mp.y <= m_Position.y + m_Scale.y;

	if (inList)
	{
		delta = -Mouse::Wheel();
	}

	if (m_Texts.empty())
	{
		return;
	}

	float totalHeight = CATEGORY_SHIFT_Y;

	for (auto *text : m_Texts)
	{
		float rowHeight;

		if (text->GetType() == NodeType::Type_Null)
		{
			rowHeight = CATEGORY_SHIFT_Y;
		}
		else
		{
			rowHeight = TYPE_SHIFT_Y;
		}

		totalHeight += rowHeight;
	}

	float maxScroll = (std::max)(0.0f, totalHeight - m_Scale.y + TEXT_SIZE_Y);

	m_ScrollOffset = (std::max)(0.0f, (std::min)(maxScroll, m_ScrollOffset + delta));

	float y = m_Position.y + CATEGORY_SHIFT_Y - m_ScrollOffset;

	for (auto *text : m_Texts)
	{
		bool category = text->GetType() == NodeType::Type_Null;

		float textOffsetX;

		if (category)
		{
			textOffsetX = CATEGORY_SHIFT_X;
		}
		else
		{
			textOffsetX = TYPE_SHIFT_X;
		}

		text->SetPosition(Vector2(m_Position.x + textOffsetX, y));

		text->SetVisible(y >= m_Position.y && y + TEXT_SIZE_Y <= m_Position.y + m_Scale.y);

		if (text->GetHitBox())
		{
			text->GetHitBox()->SetBox(text->GetPosition(), text->GetScale());
		}

		float rowHeight;

		if (category)
		{
			rowHeight = CATEGORY_SHIFT_Y;
		}
		else
		{
			rowHeight = TYPE_SHIFT_Y;
		}

		y += rowHeight;
	}

	// カテゴリーとタイプを返す

	for (auto *t : m_Texts)
	{
		if (!t)
		{
			continue;
		}

		if (t->GetType() == NodeType::Type_Null)
		{
			continue;
		}

		if (t->GetVisible() && t->GetSelectText() && Mouse::LeftRelease())
		{
			m_Select = true;

			m_SelectedCategory = t->GetCategory();

			m_SelectedType = t->GetType();

			m_ReleasedPos = Mouse::Pos();

			t->ResetSelectText();

			break;
		}
	}
}

// 画面に描く処理
void NodeCreateList::Draw()
{
	if (!m_Visible)
	{
		return;
	}

	if (!m_ListFront || !m_ListBack)
	{
		return;
	}
}

// 一覧のスクロール位置を指定する
void NodeCreateList::ScrollTo(float offset)
{
	m_ScrollOffset = offset;
}

const wchar_t *NodeCreateList::GetCategoryName(NodeCategory c)
{
	// https://cpprefjp.github.io/reference/unordered_map/unordered_map/at.html
	return CategoryNameTable.at(c);
}

// ノード一覧の文字表示を取得する
const std::vector<TextObject *> &NodeCreateList::GetTexts() const
{
	return m_Texts;
}
