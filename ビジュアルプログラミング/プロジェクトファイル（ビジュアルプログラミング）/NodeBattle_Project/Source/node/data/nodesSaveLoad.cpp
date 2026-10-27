#include "main.h"
#include "nodesSaveLoad.h"
#include "nodeGraphData.h"
#include "nodeManager.h"
#include "nodeRuntime.h"
#include <fstream>
#include <iomanip>
#include <cmath>
using nlohmann::json;

// JSONの解析は描画が始まる前にワーカーで済ませる。欠損や不正は通常ロード側で通知する
void NodesSaveLoad::PrepareCPU(const std::string &path)
{
	try
	{
		std::ifstream file(path);

		if (file)
		{
			json data;
			file >> data;
			// 解析したJSONを保持し、初期化時の再解析を省く
			m_PreparedData.emplace(path, std::move(data));
		}
	}
	catch (const std::exception &)
	{
		// 失敗した事前データは残さず、通常の読み込み側でエラーを確認する
		m_PreparedData.erase(path);
	}
}

// 設定・接続・配置をまとめて保存する
bool NodesSaveLoad::NodeEditorSave(const std::string &path, NodeManager *manager, const std::vector<NodePlacement> &placements, const std::wstring &name)
{

	json data = NodeGraphData::Encode(manager->GetNodes());

	// 保存名もノードと同時に書き込み、途中失敗で名前だけ消えないようにする
	if (!name.empty())
	{
		const int size = WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS, name.c_str(), static_cast<int>(name.size()), nullptr, 0, nullptr, nullptr);

		if (name.size() > 24 || size <= 0 || name.find_first_of(L"\r\n\t") != std::wstring::npos)
		{
			m_Message = L"名前は1〜24文字で入力してください";

			return false;
		}

		std::string encoded(size, '\0');

		WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS, name.c_str(), static_cast<int>(name.size()), &encoded[0], size, nullptr, nullptr);
		data["SaveName"] = encoded;
	}

	for (const NodePlacement &placement : placements)
	{
		for (auto &item : data["Nodes"])
		{
			if (item["ID"] == placement.id)
			{
				item["Position"] = {placement.position.x, placement.position.y};
			}
		}
	}

	// 書き終わったファイルだけを保存先へ渡すための一時ファイル
	const std::string temporary = path + ".tmp";

	std::ofstream file(temporary, std::ios::trunc);

	if (!file)
	{
		m_Message = L"保存先を開けません";

		return false;
	}

	file << std::setw(4) << data;

	file.close();

	if (!file || !MoveFileExA(temporary.c_str(), path.c_str(), MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH))
	{
		m_Message = L"保存できませんでした";

		return false;
	}

	m_Message = L"設定値と配置を保存しました";

	return true;
}

// 保存データを検証し、読み込んだノードと配置を出力配列へ渡す
bool NodesSaveLoad::NodeEditorLoad(const std::string &path, std::vector<NodeBase *> &nodes, std::vector<NodePlacement> &placements)
{
	std::ifstream file(path);

	if (!file)
	{
		m_Message = L"ファイルが見つかりません";

		return false;
	}

	json data;

	std::vector<NodeBase *> loadedNodes;
	std::vector<NodePlacement> loadedPlacements;

	try
	{
		const auto prepared = m_PreparedData.find(path);

		if (prepared != m_PreparedData.end())
		{
			data = prepared->second;
		}
		else
		{
			file >> data;
		}

		for (const auto &item : data.at("Nodes"))
		{
			if (item.contains("Position"))
			{
				const auto &p = item["Position"];

				if (!p.is_array() || p.size() != 2)
				{
					throw std::runtime_error("Invalid position");
				}

				// エディター上の保存座標は、原点から100万までに制限する
				const float positionLimit = 1000000.0f;
				const float x = p[0].get<float>();
				const float y = p[1].get<float>();

				if (!(x >= -positionLimit && x <= positionLimit) ||
					!(y >= -positionLimit && y <= positionLimit))
				{
					throw std::runtime_error("Invalid position");
				}
			}
		}

		if (!NodeGraphData::Decode(data, loadedNodes, m_Message))
		{
			return false;
		}

		size_t index = 0;
		for (auto *node : loadedNodes)
		{
			Vector2 position(60.0f + (index % 4) * 280.0f, 230.0f + (index / 4) * 145.0f);
			for (const auto &item : data["Nodes"])
			{
				if (item["ID"] == node->GetID() && item.contains("Position"))
				{
					position = Vector2(item["Position"][0].get<float>(), item["Position"][1].get<float>());
				}
			}
			loadedPlacements.push_back({node->GetID(), position});
			++index;
		}
	}
	catch (const std::exception &)
	{
		NodeGraphData::Delete(loadedNodes);
		m_Message = L"ファイルを読み込めません";
		return false;
	}

	// 成功したデータだけを呼び出し側へ渡す
	nodes.swap(loadedNodes);
	placements.swap(loadedPlacements);

	m_Message = L"設定値と配置を読み込みました";

	return true;
}

// 実行に必要なノードだけ読み込む
std::vector<NodeBase *> NodesSaveLoad::NodeRuntimeLoad(const std::string &path)
{
	std::ifstream file(path);

	std::vector<NodeBase *> nodes;

	if (!file)
	{
		m_Message = L"ファイルが見つかりません";

		return nodes;
	}

	try
	{
		json data;

		const auto prepared = m_PreparedData.find(path);

		if (prepared != m_PreparedData.end())
		{
			data = prepared->second;
		}
		else
		{
			file >> data;
		}

		NodeGraphData::Decode(data, nodes, m_Message);
	}
	catch (const std::exception &)
	{
		m_Message = L"ファイルを読み込めません";
	}

	return nodes;
}

// 敵の保存データを読み、途中失敗でHPやノードが一部だけ変わらないようにする
bool NodesSaveLoad::EnemyRuntimeLoad(const std::string &path, std::vector<NodeBase *> &nodes, int &hp)
{
	std::ifstream file(path);

	if (!file)
	{
		m_Message = L"敵のセーブデータが見つかりません：" + std::wstring(path.begin(), path.end());

		return false;
	}

	// 既存の敵に渡す前に、読み込み内容を仮置きして検証する
	std::vector<NodeBase *> loadedNodes;

	try
	{
		json data;

		const auto prepared = m_PreparedData.find(path);

		if (prepared != m_PreparedData.end())
		{
			data = prepared->second;
		}
		else
		{
			file >> data;
		}

		// HPの指定がない場合は100を使用する
		const auto savedHP = data.value("HP", json(100));

		if (!savedHP.is_number_integer() || savedHP < 1 || savedHP > 100000)
		{
			throw std::runtime_error("Invalid enemy HP");
		}

		const int loadedHP = savedHP.get<int>();

		if (!NodeGraphData::Decode(data, loadedNodes, m_Message) || !NodeRuntime::Validate(loadedNodes, m_Message))
		{
			NodeGraphData::Delete(loadedNodes);

			m_Message = L"敵のセーブデータ：" + m_Message;

			return false;
		}

		NodeGraphData::Delete(nodes);

		nodes.swap(loadedNodes);

		hp = loadedHP;

		m_Message = L"敵のHPとノードを読み込みました";

		return true;
	}
	catch (const std::exception &)
	{
		NodeGraphData::Delete(loadedNodes);

		m_Message = L"敵のセーブデータを読み込めません：HP・ノードを確認してください";

		return false;
	}
}

// 編集するノードと実行するノードが混ざらないようにコピーする
std::vector<NodeBase *> NodesSaveLoad::CloneNodes(const std::vector<NodeBase *> &source)
{
	std::vector<NodeBase *> nodes;

	NodeGraphData::Decode(NodeGraphData::Encode(source), nodes, m_Message);

	return nodes;
}

// ノードの番号で探す
NodeBase *NodesSaveLoad::FindNode(const std::vector<NodeBase *> &nodes, int id)
{
	for (auto *node : nodes)
	{
		if (node && node->GetID() == id)
		{
			return node;
		}
	}

	return nullptr;
}

// 保存枠の番号からパスを返す（6個目以降の保存は作らない）
std::string NodesSaveLoad::GetSlotPath(int slot) const
{
	if (slot >= 1 && slot <= SaveSlotCount)
	{
		return m_SlotDirectory + "/Slot" + std::to_string(slot) + ".json";
	}
	else
	{
		return "";
	}
}

// 壊れたデータも保存済みとして扱い、勝手に空き枠へ戻さない
bool NodesSaveLoad::HasSlot(int slot) const
{
	std::string path = GetSlotPath(slot);

	return !path.empty() && GetFileAttributesA(path.c_str()) != INVALID_FILE_ATTRIBUTES;
}

// 保存されているノード数も表示し、ロードする枠を見分けやすくする
std::wstring NodesSaveLoad::GetSlotCaption(int slot) const
{
	std::wstring caption = L"枠" + std::to_wstring(slot);

	if (!HasSlot(slot))
	{
		return caption + L"：空き";
	}

	try
	{
		std::ifstream file(GetSlotPath(slot));

		json data;

		file >> data;

		return caption + L"：" + GetSlotName(slot) + L"（" + std::to_wstring(data.at("Nodes").size()) + L"ノード）";
	}
	catch (const std::exception &)
	{
		return caption + L"：読込不可（上書きできます）";
	}
}

// 既存の保存を消さず、保存枠がまだない場合だけ枠1へコピーする
bool NodesSaveLoad::InitializeSaveSlots()
{
	if (GetFileAttributesA((m_SlotDirectory + "/SlotsImported.flag").c_str()) != INVALID_FILE_ATTRIBUTES)
	{
		return true;
	}

	for (int slot = 1; slot <= SaveSlotCount; ++slot)
	{
		if (HasSlot(slot))
		{
			std::ofstream marker(m_SlotDirectory + "/SlotsImported.flag");

			return true;
		}
	}

	std::string sourcePath = m_SlotDirectory + "/PlayerData.json";

	if (GetFileAttributesA(sourcePath.c_str()) == INVALID_FILE_ATTRIBUTES)
	{
		return true;
	}

	if (!CopyFileA(sourcePath.c_str(), GetSlotPath(1).c_str(), TRUE))
	{
		m_Message = L"以前の保存を枠1へ引き継げませんでした。元の保存は残っています";

		return false;
	}

	std::ofstream marker(m_SlotDirectory + "/SlotsImported.flag");

	return true;
}

// 上書きの指定と番号を確認してから、ノード・設定・配置を保存する
bool NodesSaveLoad::SaveSlot(int slot, NodeManager *manager, const std::vector<NodePlacement> &placements, bool overwrite, const std::wstring &name)
{
	std::string path = GetSlotPath(slot);

	if (path.empty())
	{
		m_Message = L"セーブは5枠までです。保存済みの枠を選んで上書きしてください";

		return false;
	}

	if (HasSlot(slot) && !overwrite)
	{
		m_Message = L"この枠は保存済みです。上書きを選ぶか、空き枠を選んでください";

		return false;
	}

	std::wstring previousNameValue;

	if (name.empty())
	{
		previousNameValue = GetSlotName(slot);
	}
	else
	{
		previousNameValue = name;
	}

	const std::wstring previousName = previousNameValue;

	if (!NodeEditorSave(path, manager, placements, previousName))
	{
		return false;
	}

	m_Message = L"枠" + std::to_wstring(slot) + L"に保存しました";

	return true;
}

// 日本語の保存名をUTF-8のJSONから取り出す
std::wstring NodesSaveLoad::GetSlotName(int slot) const
{
	try
	{
		std::ifstream file(GetSlotPath(slot));

		json data;

		file >> data;

		const std::string name = data.value("SaveName", std::string());

		const int size = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, name.c_str(), -1, nullptr, 0);

		if (size > 1)
		{
			std::wstring result(size, L'\0');

			MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, name.c_str(), -1, &result[0], size);
			result.resize(size - 1);

			return result;
		}
	}
	catch (const std::exception &)
	{
	}

	return L"名前なし";
}

// ノードの内容を残して名前だけ変更し、書き終わった後に置き換える
bool NodesSaveLoad::RenameSlot(int slot, const std::wstring &name)
{
	if (!HasSlot(slot) || name.empty() || name.size() > 24 || name.find_first_of(L"\r\n\t") != std::wstring::npos)
	{
		m_Message = L"名前は1〜24文字で入力してください";

		return false;
	}

	try
	{
		std::ifstream source(GetSlotPath(slot));

		json data;

		source >> data;
		source.close();

		const int size = WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS, name.c_str(), static_cast<int>(name.size()), nullptr, 0, nullptr, nullptr);

		if (size <= 0)
		{
			throw std::runtime_error("Invalid save name");
		}

		std::string encoded(size, '\0');

		WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS, name.c_str(), static_cast<int>(name.size()), &encoded[0], size, nullptr, nullptr);
		data["SaveName"] = encoded;

		const std::string temporary = GetSlotPath(slot) + ".tmp";

		std::ofstream output(temporary, std::ios::trunc);

		output << std::setw(4) << data;
		output.close();

		if (!output || !MoveFileExA(temporary.c_str(), GetSlotPath(slot).c_str(), MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH))
		{
			throw std::runtime_error("Could not rename save");
		}
	}
	catch (const std::exception &)
	{
		m_Message = L"名前を変更できませんでした。保存データを確認してください";

		return false;
	}

	m_Message = L"保存名を変更しました";

	return true;
}

// 確認画面で決定された枠だけを削除する（編集中のノードは残す）
bool NodesSaveLoad::DeleteSlot(int slot)
{
	if (!HasSlot(slot) || !DeleteFileA(GetSlotPath(slot).c_str()))
	{
		m_Message = L"この枠を削除できませんでした";

		return false;
	}

	m_Message = L"枠" + std::to_wstring(slot) + L"を削除しました";

	return true;
}

// 保存する接続情報を確認する
void NodesSaveLoad::DebugPrintSaveData(const std::string &, NodeManager *manager) const
{
	manager->PrintConnections();
}
