#include "gameDataLoader.h"

#include "json.hpp"

#include <fstream>
#include <iomanip>

using json = nlohmann::json;

// 補助関数
namespace
{
	Vector3 ReadVector3(const json& jsonArray, const Vector3& defaultValue)
	{
		if (!jsonArray.is_array())
		{
			return defaultValue;
		}

		if (jsonArray.size() < 3)
		{
			return defaultValue;
		}

		return Vector3(
			jsonArray[0].get<float>(),
			jsonArray[1].get<float>(),
			jsonArray[2].get<float>()
		);
	}

	ObstaclePresetData ReadObstaclePresetData(const json& obstacleJson)
	{
		ObstaclePresetData preset;

		if (obstacleJson.contains("size"))
		{
			preset.size = ReadVector3(obstacleJson["size"], preset.size);
		}

		if (obstacleJson.contains("centerX"))
		{
			preset.centerX = obstacleJson["centerX"].get<float>();
		}

		if (obstacleJson.contains("centerY"))
		{
			preset.centerY = obstacleJson["centerY"].get<float>();
		}

		if (obstacleJson.contains("texturePath"))
		{
			preset.texturePath = obstacleJson["texturePath"].get<std::string>();
		}

		if (obstacleJson.contains("isRainbow"))
		{
			preset.isRainbow = obstacleJson["isRainbow"].get<bool>();
		}

		if (obstacleJson.contains("rainbowSpeed"))
		{
			preset.rainbowSpeed = obstacleJson["rainbowSpeed"].get<float>();
		}

		if (obstacleJson.contains("alpha"))
		{
			preset.alpha = obstacleJson["alpha"].get<float>();
		}

		// 新形式
		if (obstacleJson.contains("commentTexturePath"))
		{
			preset.commentTexturePath = obstacleJson["commentTexturePath"].get<std::string>();
		}
		// 古いJSON互換用
		else if (obstacleJson.contains("commentType"))
		{
			std::string commentType = obstacleJson["commentType"].get<std::string>();

			if (commentType == "Jamp")
			{
				preset.commentTexturePath = "asset\\texture\\jamp.png";
			}
			else if (commentType == "Kick")
			{
				preset.commentTexturePath = "asset\\texture\\slide.png";
			}
			else
			{
				preset.commentTexturePath = "";
			}
		}

		if (obstacleJson.contains("commentOffset"))
		{
			preset.commentOffset = ReadVector3(obstacleJson["commentOffset"], preset.commentOffset);
		}

		return preset;
	}

	void WriteObstaclePreset(json& outArray, const ObstaclePresetData& preset)
	{
		json obstacle;

		obstacle["size"] =
		{
			preset.size.x,
			preset.size.y,
			preset.size.z
		};

		obstacle["centerX"] = preset.centerX;
		obstacle["centerY"] = preset.centerY;

		obstacle["texturePath"] = preset.texturePath;

		obstacle["isRainbow"] = preset.isRainbow;
		obstacle["rainbowSpeed"] = preset.rainbowSpeed;
		obstacle["alpha"] = preset.alpha;

		obstacle["commentTexturePath"] = preset.commentTexturePath;

		obstacle["commentOffset"] =
		{
			preset.commentOffset.x,
			preset.commentOffset.y,
			preset.commentOffset.z
		};

		outArray.push_back(obstacle);
	}
}

bool GameDataLoader::SaveStageData(const std::string& path, const GameLoadData& data)
{
	json j;

	j["Ground"]["center"] =
	{
		data.ground.center.x,
		data.ground.center.y,
		data.ground.center.z
	};

	j["Ground"]["halfSize"] =
	{
		data.ground.halfSize.x,
		data.ground.halfSize.y,
		data.ground.halfSize.z
	};

	j["Ground"]["texturePath"] = data.ground.texturePath;
	j["Ground"]["tileHalfSizeY"] = data.ground.tileHalfSizeY;
	j["Ground"]["tileSize"] = data.ground.tileSize;

	j["Ground"]["colorPattern"] = json::array();

	for (int colorNumber : data.ground.colorPattern)
	{
		j["Ground"]["colorPattern"].push_back(colorNumber);
	}

	j["TexturePaths"] = json::array();

	for (const std::string& texturePath : data.texturePaths)
	{
		j["TexturePaths"].push_back(texturePath);
	}

	j["FloorObstaclePresets"] = json::array();

	for (const ObstaclePresetData& preset : data.floorObstaclePresets)
	{
		WriteObstaclePreset(j["FloorObstaclePresets"], preset);
	}

	j["WallObstaclePresets"] = json::array();

	for (const ObstaclePresetData& preset : data.wallObstaclePresets)
	{
		WriteObstaclePreset(j["WallObstaclePresets"], preset);
	}

	std::ofstream file(path);

	if (!file.is_open())
	{
		printf("stageData.json 書き出し失敗: %s\n", path.c_str());
		return false;
	}

	file << std::setw(4) << j;

	printf("stageData.json 書き出し成功: %s\n", path.c_str());

	return true;
}

// JSONを読み込む。失敗時は出力データを変えない
bool GameDataLoader::LoadStageData(const std::string& path, GameLoadData& outData)
{
	std::ifstream file(path);

	if (!file.is_open())
	{
		printf("stageData.json 読み込み失敗: %s\n", path.c_str());
		return false;
	}

	json j;

	try
	{
		file >> j;
	}
	catch (const std::exception& e)
	{
		printf("stageData.json JSON解析失敗: %s\n", e.what());
		return false;
	}

	GameLoadData data;

	if (j.contains("Ground"))
	{
		const json& ground = j["Ground"];

		if (ground.contains("center"))
		{
			data.ground.center = ReadVector3(ground["center"], data.ground.center);
		}

		if (ground.contains("halfSize"))
		{
			data.ground.halfSize = ReadVector3(ground["halfSize"], data.ground.halfSize);
		}

		if (ground.contains("texturePath"))
		{
			data.ground.texturePath = ground["texturePath"].get<std::string>();
		}

		if (ground.contains("tileHalfSizeY"))
		{
			data.ground.tileHalfSizeY = ground["tileHalfSizeY"].get<float>();
		}

		if (ground.contains("tileSize"))
		{
			data.ground.tileSize = ground["tileSize"].get<float>();
		}

		if (ground.contains("colorPattern") && ground["colorPattern"].is_array())
		{
			data.ground.colorPattern.clear();

			for (const json& colorNumber : ground["colorPattern"])
			{
				data.ground.colorPattern.push_back(colorNumber.get<int>());
			}
		}
	}

	if (j.contains("TexturePaths") && j["TexturePaths"].is_array())
	{
		data.texturePaths.clear();

		for (const json& texturePath : j["TexturePaths"])
		{
			data.texturePaths.push_back(texturePath.get<std::string>());
		}
	}

	if (j.contains("FloorObstaclePresets") && j["FloorObstaclePresets"].is_array())
	{
		data.floorObstaclePresets.clear();

		for (const json& obstacleJson : j["FloorObstaclePresets"])
		{
			data.floorObstaclePresets.push_back(ReadObstaclePresetData(obstacleJson));
		}
	}

	if (j.contains("WallObstaclePresets") && j["WallObstaclePresets"].is_array())
	{
		data.wallObstaclePresets.clear();

		for (const json& obstacleJson : j["WallObstaclePresets"])
		{
			data.wallObstaclePresets.push_back(ReadObstaclePresetData(obstacleJson));
		}
	}

	outData = data;

	printf("stageData.json 読み込み成功: %s\n", path.c_str());

	return true;
}

// 初期ステージの設定データを作る
GameLoadData GameDataLoader::CreateDefaultGameLoadData()
{
	GameLoadData data;

	data.ground.center = Vector3(25.0f, 0.0f, 23.0f);
	data.ground.halfSize = Vector3(10.0f, 0.5f, 100.0f);
	data.ground.texturePath = "asset\\texture\\frameBox.png";
	data.ground.tileHalfSizeY = 0.05f;
	data.ground.tileSize = 4.0f;

	data.ground.colorPattern =
	{
		1, 2, 3, 4, 5,
		2, 3, 4, 5, 1,
		3, 4, 5, 1, 2,
		4, 5, 1, 2, 3,
		5, 1, 2, 3, 4
	};

	data.texturePaths.push_back("asset\\texture\\go.png");
	data.texturePaths.push_back("asset\\texture\\frameBox.png");
	data.texturePaths.push_back("asset\\texture\\frameBoxR.png");
	data.texturePaths.push_back("asset\\texture\\siro.png");
	data.texturePaths.push_back("asset\\texture\\slide.png");
	data.texturePaths.push_back("asset\\texture\\jamp.png");
	data.texturePaths.push_back("asset\\texture\\TimeA.png");
	data.texturePaths.push_back("asset\\texture\\car_handle.png");

	{
		ObstaclePresetData preset;

		preset.size = Vector3(10.0f, 2.5f, 10.0f);
		preset.centerX = 25.0f;
		preset.centerY = 2.5f;
		preset.texturePath = "asset\\texture\\siro.png";

		preset.isRainbow = true;
		preset.rainbowSpeed = 2.0f;
		preset.alpha = 0.5f;

		preset.commentTexturePath = "asset\\texture\\jamp.png";
		preset.commentOffset = Vector3(0.0f, 1.0f, 5.0f);

		data.floorObstaclePresets.push_back(preset);
	}

	{
		ObstaclePresetData preset;

		preset.size = Vector3(10.0f, 5.0f, 10.0f);
		preset.centerX = 25.0f;
		preset.centerY = 5.5f;
		preset.texturePath = "asset\\texture\\siro.png";

		preset.isRainbow = true;
		preset.rainbowSpeed = 1.5f;
		preset.alpha = 0.5f;

		preset.commentTexturePath = "asset\\texture\\jamp.png";
		preset.commentOffset = Vector3(0.0f, 2.0f, 0.0f);

		data.floorObstaclePresets.push_back(preset);
	}

	{
		ObstaclePresetData preset;

		preset.size = Vector3(5.0f, 5.0f, 10.0f);
		preset.centerX = 30.0f;
		preset.centerY = 5.5f;
		preset.texturePath = "asset\\texture\\siro.png";

		preset.isRainbow = true;
		preset.rainbowSpeed = 1.5f;
		preset.alpha = 0.5f;

		preset.commentTexturePath = "asset\\texture\\jamp.png";
		preset.commentOffset = Vector3(0.0f, 2.0f, 0.0f);

		data.floorObstaclePresets.push_back(preset);
	}

	{
		ObstaclePresetData preset;

		preset.size = Vector3(5.0f, 5.0f, 10.0f);
		preset.centerX = 20.0f;
		preset.centerY = 5.5f;
		preset.texturePath = "asset\\texture\\siro.png";

		preset.isRainbow = true;
		preset.rainbowSpeed = 1.5f;
		preset.alpha = 0.5f;

		preset.commentTexturePath = "asset\\texture\\jamp.png";
		preset.commentOffset = Vector3(0.0f, 2.0f, 0.0f);

		data.floorObstaclePresets.push_back(preset);
	}

	{
		ObstaclePresetData preset;

		preset.size = Vector3(10.0f, 2.5f, 5.0f);
		preset.centerX = 25.0f;
		preset.centerY = 5.5f;
		preset.texturePath = "asset\\texture\\siro.png";

		preset.isRainbow = true;
		preset.rainbowSpeed = 1.5f;
		preset.alpha = 0.5f;

		preset.commentTexturePath = "asset\\texture\\jamp.png";
		preset.commentOffset = Vector3(0.0f, 2.0f, 0.0f);

		data.floorObstaclePresets.push_back(preset);
	}

	{
		ObstaclePresetData preset;

		preset.size = Vector3(2.5f, 5.0f, 10.0f);
		preset.centerX = 25.0f;
		preset.centerY = 5.5f;
		preset.texturePath = "asset\\texture\\siro.png";

		preset.isRainbow = true;
		preset.rainbowSpeed = 1.5f;
		preset.alpha = 0.5f;

		preset.commentTexturePath = "asset\\texture\\jamp.png";
		preset.commentOffset = Vector3(0.0f, 2.0f, 0.0f);

		data.floorObstaclePresets.push_back(preset);
	}

	{
		ObstaclePresetData preset;

		preset.size = Vector3(2.5f, 100.0f, 20.0f);
		preset.centerX = 32.5f;
		preset.centerY = 100.0f;
		preset.texturePath = "asset\\texture\\siro.png";

		preset.isRainbow = true;
		preset.rainbowSpeed = 2.0f;
		preset.alpha = 0.5f;

		preset.commentTexturePath = "asset\\texture\\slide.png";
		preset.commentOffset = Vector3(-5.0f, -135.0f, 0.0f);

		data.wallObstaclePresets.push_back(preset);
	}

	{
		ObstaclePresetData preset;

		preset.size = Vector3(2.5f, 100.0f, 20.0f);
		preset.centerX = 17.5f;
		preset.centerY = 100.0f;
		preset.texturePath = "asset\\texture\\siro.png";

		preset.isRainbow = true;
		preset.rainbowSpeed = 1.5f;
		preset.alpha = 0.5f;

		preset.commentTexturePath = "asset\\texture\\slide.png";
		preset.commentOffset = Vector3(5.0f, -135.0f, 0.0f);

		data.wallObstaclePresets.push_back(preset);
	}

	{
		ObstaclePresetData preset;

		preset.size = Vector3(2.5f, 8.0f, 20.0f);
		preset.centerX = 32.5f;
		preset.centerY = 20.0f;
		preset.texturePath = "asset\\texture\\siro.png";

		preset.isRainbow = true;
		preset.rainbowSpeed = 2.0f;
		preset.alpha = 0.5f;

		preset.commentTexturePath = "asset\\texture\\slide.png";
		preset.commentOffset = Vector3(-5.0f, -8.0f, 8.0f);

		data.wallObstaclePresets.push_back(preset);
	}

	{
		ObstaclePresetData preset;

		preset.size = Vector3(2.5f, 8.0f, 20.0f);
		preset.centerX = 17.5f;
		preset.centerY = 20.0f;
		preset.texturePath = "asset\\texture\\siro.png";

		preset.isRainbow = true;
		preset.rainbowSpeed = 2.0f;
		preset.alpha = 0.5f;

		preset.commentTexturePath = "asset\\texture\\slide.png";
		preset.commentOffset = Vector3(5.0f, -8.0f, 8.0f);

		data.wallObstaclePresets.push_back(preset);
	}

	return data;
}