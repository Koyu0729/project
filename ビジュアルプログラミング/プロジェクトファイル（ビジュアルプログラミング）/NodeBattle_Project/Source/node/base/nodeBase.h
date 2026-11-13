#pragma once
#include <vector>
#include <string>
#include <cmath>
#include "nodeList.h"
#include "vector3.h"
#define ID_NULL (-1)
#define ID_NEXT_NULL (-1)
class Player;

enum class NodeStatus
{
	Running,
	Completed,
	Failed
};

struct NodeSettings
{

	// ノードごとの時間・HP・距離・角度の設定
	float duration = 0.5f;

	float hpPercent = 50.0f;

	float distance = 1.5f;

	float angleDegrees = 90.0f;
};

class NodeBase
{
  protected:
	// ノードの種類・名前・番号
	NodeCategory m_Category;

	NodeType m_Type;

	std::wstring m_Name;

	int m_ID = ID_NULL;

	float frame = 0.0f;

	// このノードで動かすプレイヤー
	Player *m_GameChara = nullptr;

	// 設定値と、実行してからの時間・残りの回転量
	NodeSettings m_Settings;

	float m_Elapsed = 0.0f;

	float m_TurnRemaining = 0.0f;

	// 条件の結果などを表示する説明
	std::wstring m_ExecutionDetail;

  public:
	NodeBase(int id, NodeCategory category, NodeType type)
		: m_Category(category), m_Type(type), m_ID(id)
	{
		auto list = NodeTable.find(category);

		if (list != NodeTable.end())
		{
			for (const auto &info : list->second)
			{
				if (info.type == type)
				{
					m_Name = info.name;

					break;
				}
			}
		}

		if (m_Name.empty())
		{
			m_Name = L"旧アクション";
		}

		if (type == NodeType::Action_Wait || type == NodeType::Movement_Stop)
		{
			m_Settings.duration = 0.2f;
		}

		if (category == NodeCategory::AttackMelee || category == NodeCategory::AttackRange)
		{
			m_Settings.duration = 0.35f;
		}

		if (type == NodeType::Movement_ToTarget)
		{
			m_Settings.distance = 1.4f;
		}

		if (type == NodeType::Movement_Escape)
		{
			m_Settings.distance = 6.0f;
		}
	}

	virtual ~NodeBase() = default;

	// プレイヤーへ行動を渡して、次のノードを返す
	virtual NodeBase *Execute() = 0; 

	// このノードを始めるときに、時間と行動を準備する
	virtual void Begin();

	// 設定時間や条件を見ながら、ノードを進める
	virtual NodeStatus Tick(float deltaSeconds);

	// このノードの行動を終了し、停止時の状態も解除する
	virtual void End();

	NodeCategory GetCategory() const
	{
		return m_Category;
	}

	NodeType GetType() const
	{
		return m_Type;
	}

	const std::wstring &GetName() const
	{
		return m_Name;
	}

	int GetID() const
	{
		return m_ID;
	}

	virtual void SetNext(NodeBase *next, Port port = Port::OutDef) = 0;

	virtual NodeBase *GetNext() const = 0;

	virtual NodeBase *GetNextForPort(Port port) const
	{
		if (port == Port::OutDef)
		{
			return GetNext();
		}
		else
		{
			return nullptr;
		}
	}

	void SetPlayer(Player *player)
	{
		m_GameChara = player;
	}

	Player *GetPlayer() const
	{
		return m_GameChara;
	}

	const NodeSettings &GetSettings() const
	{
		return m_Settings;
	}

	void SetSettings(const NodeSettings &settings)
	{
		m_Settings = settings;
	}

	// 設定値が使える範囲に入っているか確認する
	bool HasValidSettings() const
	{
		// 待機・移動・攻撃に使う時間を確認する
		if (!(m_Settings.duration >= 0.05f && m_Settings.duration <= 10.0f))
		{
			return false;
		}

		// HPの条件に使う割合を確認する
		if (!(m_Settings.hpPercent >= 0 && m_Settings.hpPercent <= 100))
		{
			return false;
		}

		// 敵との距離の設定を確認する
		if (!(m_Settings.distance >= 0.1f && m_Settings.distance <= 100))
		{
			return false;
		}

		// 回転する角度の設定を確認する
		if (!(m_Settings.angleDegrees >= 1 && m_Settings.angleDegrees <= 360))
		{
			return false;
		}

		return true;
	}

	float GetElapsed() const
	{
		return m_Elapsed;
	}

	const std::wstring &GetExecutionDetail() const
	{
		return m_ExecutionDetail;
	}

	// 実行中の説明に使う、設定値つきの名前を作る
	std::wstring GetDisplayName() const;

	// ノードの中に表示する設定値の説明を作る
	std::wstring GetParameterSummary() const;
};
