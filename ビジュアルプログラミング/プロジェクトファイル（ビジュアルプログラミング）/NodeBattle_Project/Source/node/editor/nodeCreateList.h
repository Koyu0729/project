// 右クリックで新しいノードを作るときに出てくる選択式リスト
#pragma once

#include "scene.h"
#include "nodeBase.h"
#include "polyTex.h"

// リストのサイズ
#define CREATE_LIST_SIZE_X (300.0f) // ノード追加一覧のサイズ
#define CREATE_LIST_SIZE_Y (400.0f) // ノード追加一覧のサイズ

// 上部の保存・ステージ・テストボタンより下に一覧を収める
constexpr float CREATE_LIST_TOP = 170.0f;

// テキストの当たり判定のサイズ
#define TEXT_SIZE_X (220.0f) // テキストの当たり判定のサイズ（ｘ）
#define TEXT_SIZE_Y (40.0f)	 // テキストの当たり判定のサイズ（ｙ）

// カテゴリーの文字が出たときにずらすポジションの量
#define CATEGORY_SHIFT_X (10.0f) // カテゴリーの文字が出たときにずらすポジションの量（ｘ）
#define CATEGORY_SHIFT_Y (50.0f) // カテゴリーの文字が出たときにずらすポジションの量（ｙ）

// タイプの文字が出たときにずらすポジションの量
#define TYPE_SHIFT_X (30.0f) // タイプの文字が出たときにずらすポジションの量（ｘ）
#define TYPE_SHIFT_Y (45.0f) // タイプの文字が出たときにずらすポジションの量（ｙ）

class TextObject;

class NodeCreateList : public GameObject2D
{

	// 追加できるノードの種類を一覧で表示する

  private:
	// 枠
	PolyTex *m_ListFront = nullptr;
	// 裏
	PolyTex *m_ListBack = nullptr;

	// テキスト用配列
	std::vector<TextObject *> m_Texts;

	NodeCategory m_SelectedCategory = NodeCategory::Category_Null;

	NodeType m_SelectedType = NodeType::Type_Null;

	Vector2 m_ReleasedPos = {0.0f, 0.0f};

	bool m_Select = false;

	// スクロール加算量
	float m_ScrollOffset = 0.0f;

  public:
	// 初期化処理を行う
	void Init() override;

	// 終了処理を行う
	void Uninit() override;

	// 毎フレームの処理
	void Update() override;

	// 画面に描く処理
	void Draw() override;

	// 編集画面の選択状態を解除する
	void SelectReset()
	{
		m_Select = false;
	}

	// 一覧のスクロール位置を指定する
	void ScrollTo(float offset);

	const wchar_t *GetCategoryName(NodeCategory c);

	// 右クリック位置に表示

	void SetPosition(const Vector2 &pos)
	{
		m_Position = pos;

		if (m_Hit)
		{
			m_Hit->SetBox(m_Position, m_Scale);
		}

		if (m_ListFront)
		{
			m_ListFront->SetPosition(pos);
		}

		if (m_ListBack)
		{
			m_ListBack->SetPosition(pos);
		}
	}

	bool GetSelect() const
	{
		return m_Select;
	}

	void SetReleasedPos(const Vector2 &pos)
	{
		m_ReleasedPos = pos;
	}

	NodeCategory GetSelectedCategory() const
	{
		return m_SelectedCategory;
	}

	NodeType GetSelectedType() const
	{
		return m_SelectedType;
	}

	Vector2 GetReleasedPos() const
	{
		return m_ReleasedPos;
	}

	// ノード一覧の文字表示を取得する
	const std::vector<TextObject *> &GetTexts() const;
};
