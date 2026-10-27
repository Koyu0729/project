#pragma once
#include "nodeBase.h"
#include "json.hpp"
#include <string>
#include <vector>

// ノードと接続をJSONに保存し、JSONから読み戻す
namespace NodeGraphData
{

// ノードと接続を保存できる形にする
nlohmann::json Encode(const std::vector<NodeBase *> &nodes);

// 保存したデータからノードと接続を戻す
bool Decode(const nlohmann::json &data, std::vector<NodeBase *> &nodes, std::wstring &message);

// 読み込んだノード本体を解放する
void Delete(std::vector<NodeBase *> &nodes);

}