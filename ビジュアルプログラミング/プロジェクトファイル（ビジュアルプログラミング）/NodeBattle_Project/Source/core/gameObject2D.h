// 2Dオブジェクトの配置・子オブジェクト・当たり判定を管理する
#pragma once
#include "texture.h"

#include "vector3.h"
#include "vector2.h"

#include "uiHitBox.h"

#include <DirectXMath.h>

using namespace DirectX;

#include "gameObject.h"

class GameObject2D : public GameObject
{
  protected:
	// 表示と入力をまとめて管理する子オブジェクト
	std::vector<GameObject2D *> m_Children = {};

	Vector2 m_Position = {0.0f, 0.0f};

	float m_Rotation = 0.0f;

	Vector2 m_Scale = {100.0f, 100.0f};

	ID3D11ShaderResourceView *m_Texture = nullptr;

	// 当たり判定のポインター

	UIHitBox *m_Hit = nullptr;

	// オブジェクトの表示状態
	bool m_Visible = true;

  public:
	// オブジェクトを削除するときに、自分の当たり判定も解放する
	~GameObject2D() override
	{
		delete m_Hit;
		m_Hit = nullptr;
	}

	bool Destroy()
	{
		return m_Destroy;
	}

	void AddChild(GameObject2D *child)
	{
		if (!child)
		{
			return;
		}

		m_Children.push_back(child);
	}

	void RemoveChild(GameObject2D *child)
	{
		if (!child)
		{
			return;
		}

		for (auto it = m_Children.begin(); it != m_Children.end(); ++it)
		{
			if (*it == child)
			{
				m_Children.erase(it);

				break;
			}
		}
	}

	// 当たり判定生成（initで呼び出す）
	void AddHitBox(bool isAABB = true, bool leftTop = false)
	{
		if (!m_Hit)
		{
			m_Hit = new UIHitBox();

			if (leftTop)
			{
				m_Hit->TransCenter();
			}
		}

		if (isAABB)
		{
			m_Hit->SetBox(m_Position, m_Scale, false);
		}
		else
		{
			m_Hit->SetBox(m_Position, m_Scale, true, m_Rotation);
		}

		// 表示状態を当たり判定の有効状態に反映する
		m_Hit->SetEnable(m_Visible);
	}

	// 本体と子オブジェクトを削除対象に設定する

	void SetDestroy()
	{
		for (auto *child : m_Children)
		{
			if (child)
			{
				child->SetDestroy();
			}
		}

		m_Destroy = true;
	}

	//  ポジション変更(当たり判定もずれる）

	void SetPosition(Vector2 position, bool children = true)
	{
		Vector2 delta = position - m_Position;

		m_Position = position;

		if (m_Hit)
		{
			m_Hit->SetBox(m_Position, m_Scale);
		}

		if (children)
		{
			for (auto *child : m_Children)
			{
				if (!child)
				{
					continue;
				}

				child->SetPosition(child->GetPosition() + delta);
			}
		}
	}

	void SetRotation(float rotation, bool children = true, bool isOBB = false)
	{
		m_Rotation = rotation;

		if (m_Hit)
		{
			if (isOBB)
			{
				m_Hit->SetBox(m_Position, m_Scale, true, m_Rotation);
			}
		}

		if (!children)
		{
			return;
		}

		for (auto *child : m_Children)
		{
			if (!child)
			{
				continue;
			}

			child->SetRotation(rotation, true, isOBB);
		}
	}

	//  スケール変更(当たり判定もずれる）

	void SetScale(Vector2 scale, bool children = true)
	{
		Vector2 delta = scale - m_Scale;

		m_Scale = scale;

		if (m_Hit)
		{
			m_Hit->SetBox(m_Position, m_Scale);
		}

		if (children)
		{
			for (auto *child : m_Children)
			{
				if (!child)
				{
					continue;
				}

				child->SetScale(child->GetScale() + delta);
			}
		}
	}

	Vector2 GetPosition()
	{
		return m_Position;
	}

	float GetRotation()
	{
		return m_Rotation;
	}

	Vector2 GetScale()
	{
		return m_Scale;
	}

	// 表示と当たり判定の有効状態を切り替える

	void SetVisible(bool visible)
	{
		m_Visible = visible;

		if (m_Hit)
		{
			m_Hit->SetEnable(visible);
		}

		// 子オブジェクトにも表示状態を反映する
		for (GameObject2D *children : m_Children)
		{
			if (!children)
			{
				continue;
			}

			children->SetVisible(visible);
		}
	}

	bool GetVisible() const
	{
		return m_Visible;
	}

	UIHitBox *GetHitBox()
	{
		return m_Hit;
	}
};
