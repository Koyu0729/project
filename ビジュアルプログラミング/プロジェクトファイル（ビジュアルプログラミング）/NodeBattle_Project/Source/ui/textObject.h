#pragma once
#include "nodeList.h"
#include "textObjectBase.h"

// ノード一覧の項目を表示し、クリックによる選択を管理する
class TextObject : public TextObjectBase
{
  private:
	bool m_Select = false;
	NodeCategory m_Category = NodeCategory::Category_Null;
	NodeType m_Type = NodeType::Type_Null;

  public:
	void Init() override;
	void Uninit() override;
	void Update() override;
	void Draw() override;

	void ResetSelectText()
	{
		m_Select = false;
	}
	// ノードクリエイトリストで設定
	void SetCategory(NodeCategory c)
	{
		m_Category = c;
	}
	// ノードクリエイトリストで受け取る
	void SetType(NodeType t)
	{
		m_Type = t;
	}
	NodeCategory GetCategory() const
	{
		return m_Category;
	}
	// 項目に対応するノードの種類を取得する
	NodeType GetType() const
	{
		return m_Type;
	}
	bool GetSelectText()
	{
		return m_Select;
	}
};