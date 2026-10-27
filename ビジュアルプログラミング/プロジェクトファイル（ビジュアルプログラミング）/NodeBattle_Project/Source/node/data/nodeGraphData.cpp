#include "nodeGraphData.h"
#include "nodeCreate.h"
#include "nodeManager.h"
#include <stdexcept>

using nlohmann::json;

// 読み込んだノード本体を解放する
void NodeGraphData::Delete(std::vector<NodeBase *> &nodes)
{
	for (auto *node : nodes)
	{
		delete node;
	}

	nodes.clear();
}

// ノードと接続を保存できる形にする
json NodeGraphData::Encode(const std::vector<NodeBase *> &nodes)
{
	json data = {{"version", 2}, {"Nodes", json::array()}, {"Connections", json::array()}};

	for (auto *node : nodes)
	{
		if (!node)
		{
			continue;
		}

		const auto &s = node->GetSettings();

		data["Nodes"].push_back({{"ID", node->GetID()}, {"Category", (int)node->GetCategory()}, {"Type", (int)node->GetType()}, {"Settings", {{"duration", s.duration}, {"hpPercent", s.hpPercent}, {"distance", s.distance}, {"angleDegrees", s.angleDegrees}}}});

		for (Port port : {Port::OutDef, Port::OutTrue, Port::OutFalse})
		{
			if (auto *next = node->GetNextForPort(port))
			{
				data["Connections"].push_back({{"fromID", node->GetID()}, {"toID", next->GetID()}, {"fromPort", (int)port}});
			}
		}
	}

	return data;
}

// 保存したデータからノードと接続を戻す
bool NodeGraphData::Decode(const json &data, std::vector<NodeBase *> &nodes, std::wstring &message)
{

	// 読み込み途中で失敗しても、今のノードを消さないための仮置き
	NodeManager staged;

	// 登録前のノードだけ、この関数が削除する
	NodeBase *unregisteredNode = nullptr;

	try
	{
		const int version = data.value("version", 1);

		if (version < 1 || version > 2 || !data.at("Nodes").is_array() || !data.at("Connections").is_array())
		{
			throw std::runtime_error("Unsupported graph format");
		}

		if (data["Nodes"].size() > 512 || data["Connections"].size() > 1024)
		{
			throw std::runtime_error("Graph too large");
		}

		for (const auto &item : data["Nodes"])
		{
			int id = item.at("ID").get<int>();

			if (id < 0 || id > 1000000 || staged.GetNodeUseID(id))
			{
				throw std::runtime_error("Invalid ID");
			}

			unregisteredNode = NodeCreate::CreateLogic((NodeCategory)item.at("Category").get<int>(), (NodeType)item.at("Type").get<int>(), id);
			NodeBase *node = unregisteredNode;

			if (!node)
			{
				throw std::runtime_error("Unsupported node");
			}

			NodeSettings settings = node->GetSettings();

			if (item.contains("Settings"))
			{
				const auto &s = item.at("Settings");

				settings.duration = s.value("duration", settings.duration);

				settings.hpPercent = s.value("hpPercent", settings.hpPercent);

				settings.distance = s.value("distance", settings.distance);

				settings.angleDegrees = s.value("angleDegrees", settings.angleDegrees);
			}

			node->SetSettings(settings);

			if (!node->HasValidSettings())
			{
				throw std::runtime_error("Invalid settings");
			}

			// 登録後の削除はstagedに任せる。登録で例外が出た場合はcatchで削除する
			staged.RegisterNode(node);
			unregisteredNode = nullptr;
		}

		for (const auto &item : data["Connections"])
		{
			auto *from = staged.GetNodeUseID(item.at("fromID").get<int>());

			auto *to = staged.GetNodeUseID(item.at("toID").get<int>());

			Port port = (Port)item.at("fromPort").get<int>();

			if (!from || !to || from->GetNextForPort(port) || !staged.RegisterConnect(from, to, port))
			{
				throw std::runtime_error("Invalid connection");
			}
		}

		Delete(nodes);

		// 仮置きしたノードを渡し、以降の削除は呼び出した側に任せる
		staged.TransferNodesTo(nodes);

		message = L"読み込みました";

		return true;
	}
	catch (...)
	{
		delete unregisteredNode;

		message = L"読み込めません：ファイルの形式・設定値・接続を確認してください";

		return false;
	}
}
