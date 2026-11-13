#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include "json.hpp"
#include "vector2.h"

// 保存するノードの番号と編集画面上の位置
struct NodePlacement
{
	int id;
	Vector2 position;
};

class NodeManager;

class NodeBase;

class NodesSaveLoad
{

	// 保存・読み込みの結果を表示するメッセージ
	std::wstring m_Message;

	// 5枠の保存先フォルダー
	std::string m_SlotDirectory;

	// シーンのCPUロード時だけ使うJSON。編集開始後は破棄し、手動ロードは最新ファイルを読む
	std::unordered_map<std::string, nlohmann::json> m_PreparedData;

  public:
	static constexpr int SaveSlotCount = 5;
	// 保存枠の場所を決める
	explicit NodesSaveLoad(const std::string &slotDirectory = "nodeSaveData")
		: m_SlotDirectory(slotDirectory)
	{
	}

	void PrepareCPU(const std::string &path);

	void ClearPrepared()
	{
		m_PreparedData.clear();
	}
	// 設定・接続・配置をまとめて保存する
	bool NodeEditorSave(const std::string &path, NodeManager *manager, const std::vector<NodePlacement> &placements = {}, const std::wstring &name = L"");

	// ノードと配置を読み込む。画面の入れ替えは呼び出し側で行う
	bool NodeEditorLoad(const std::string &path, std::vector<NodeBase *> &nodes, std::vector<NodePlacement> &placements);

	// 実行に必要なノードだけ読み込む
	std::vector<NodeBase *> NodeRuntimeLoad(const std::string &path);

	// 敵のHP・ノードを確認し、成功したときだけまとめて渡す
	bool EnemyRuntimeLoad(const std::string &path, std::vector<NodeBase *> &nodes, int &hp);

	// 編集するノードと実行するノードが混ざらないようにコピーする
	std::vector<NodeBase *> CloneNodes(const std::vector<NodeBase *> &nodes);

	// ノードの番号で探す
	NodeBase *FindNode(const std::vector<NodeBase *> &nodes, int id);

	// 枠の番号を確認して、保存先と表示名を作る
	std::string GetSlotPath(int slot) const;

	bool HasSlot(int slot) const;

	std::wstring GetSlotCaption(int slot) const;

	// 古い1件の保存は、初めて枠を開いたときに枠1へ引き継ぐ
	bool InitializeSaveSlots();

	// 保存済みの枠は、上書きを選んだ場合だけ更新する
	bool SaveSlot(int slot, NodeManager *manager, const std::vector<NodePlacement> &placements = {}, bool overwrite = false, const std::wstring &name = L"");

	// 保存名の取得・変更と、確認済みの枠の削除
	std::wstring GetSlotName(int slot) const;

	bool RenameSlot(int slot, const std::wstring &name);

	bool DeleteSlot(int slot);

	const std::wstring &GetMessage() const
	{
		return m_Message;
	}

	// 保存する接続情報を確認する
	void DebugPrintSaveData(const std::string &path, NodeManager *manager) const;

};
