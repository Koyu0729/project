#include "gameDataLoader.h"

#include "json.hpp"

#include <cmath>
#include <cstdio>
#include <fstream>
#include <limits>
#include <stdexcept>

using json = nlohmann::json;

namespace
{
	double ReadNumber(const json& value)
	{
		if (!value.is_number())
		{
			throw std::runtime_error("Expected a number.");
		}

		const double number = value.get<double>();
		if (!std::isfinite(number))
		{
			throw std::runtime_error("Number must be finite.");
		}
		return number;
	}

	float ReadFloat(const json& value)
	{
		const double number = ReadNumber(value);
		const float maxValue = (std::numeric_limits<float>::max)();
		if (number < -maxValue || number > maxValue)
		{
			throw std::runtime_error("Number is outside the float range.");
		}
		return (float)number;
	}

	float ReadFloat(const json& object, const char* key, float defaultValue)
	{
		const auto value = object.find(key);
		return value == object.end() ? defaultValue : ReadFloat(*value);
	}

	Vector3 ReadVector3(const json& object, const char* key, const Vector3& defaultValue)
	{
		const auto values = object.find(key);
		if (values == object.end())
		{
			return defaultValue;
		}

		if (!values->is_array() || values->size() < 3)
		{
			throw std::runtime_error(std::string(key) + " must contain three coordinates.");
		}

		return Vector3(ReadFloat((*values)[0]), ReadFloat((*values)[1]), ReadFloat((*values)[2]));
	}

	GroundLoadData ReadGroundData(const json& ground)
	{
		GroundLoadData data;
		data.texturePath = ground.value("texturePath", data.texturePath);
		data.center = ReadVector3(ground, "center", data.center);
		data.halfSize = ReadVector3(ground, "halfSize", data.halfSize);
		data.tileHalfSizeY = ReadFloat(ground, "tileHalfSizeY", data.tileHalfSizeY);
		data.tileSize = ReadFloat(ground, "tileSize", data.tileSize);

		// °‚Ì‰¡•‚ğA‰œs‚«•ûŒü‚Ì¶¬ŠÔŠu‚É‚àg‚¤B
		if (data.halfSize.x <= 0.0f)
		{
			throw std::runtime_error("Ground.halfSize.x must be positive.");
		}

		const auto colors = ground.find("colorPattern");
		if (colors == ground.end())
		{
			return data;
		}

		if (!colors->is_array())
		{
			throw std::runtime_error("Ground.colorPattern must be an array.");
		}

		data.colorPattern.clear();
		for (const json& value : *colors)
		{
			const double color = std::trunc(ReadNumber(value));
			if (color < (std::numeric_limits<int>::min)() || color > (std::numeric_limits<int>::max)())
			{
				throw std::runtime_error("Ground.colorPattern contains an out-of-range color.");
			}
			data.colorPattern.push_back((int)color);
		}

		return data;
	}

	GameLoadData ReadStageData(const json& document)
	{
		if (!document.is_object())
		{
			throw std::runtime_error("Stage data must be an object.");
		}

		GameLoadData data;

		const auto ground = document.find("Ground");
		if (ground != document.end())
		{
			data.ground = ReadGroundData(*ground);
		}

		return data;
	}
}

bool GameDataLoader::LoadStageData(const std::string& path, GameLoadData& outData)
{
	std::ifstream file(path);
	if (!file)
	{
		printf("stageData.json “Ç‚İ‚İ¸”s: %s\n", path.c_str());
		return false;
	}

	try
	{
		json document;
		file >> document;

		// °İ’è‚Ì“Ç‚İ‚İ‚ÆŒŸØ‚ª¬Œ÷‚µ‚½‚çŒ‹‰Ê‚ğ”½‰f‚·‚éB
		outData = ReadStageData(document);
	}
	catch (const std::exception& error)
	{
		printf("stageData.json “Ç‚İ‚İ¸”s: %s (%s)\n", path.c_str(), error.what());
		return false;
	}

	printf("stageData.json “Ç‚İ‚İ¬Œ÷: %s\n", path.c_str());
	return true;
}

GameLoadData GameDataLoader::CreateDefaultGameLoadData()
{
	return {};
}
