#include "main.h"
#include "renderer.h"
#include "texture.h"
#include <algorithm>

// 読み込みの前半：PNGなどを画素データへ展開して保管する。まだ描画には使えない
bool Texture::PrepareCPU(const char *fileName)
{
	if (!fileName || !*fileName)
	{
		return false;
	}

	// CPU画像とGPU画像の共有一覧を同時に変更しないようにする
	std::lock_guard<std::mutex> lock(m_Mutex);
	const std::string key = CreateKey(fileName);

	if (m_TexturePool.count(key) || m_CPUImages.count(key))
	{
		return true;
	}

	auto image = std::make_unique<ScratchImage>();
	const int length = MultiByteToWideChar(CP_ACP, 0, fileName, -1, nullptr, 0);
	if (length <= 0)
	{
		return false;
	}
	std::vector<wchar_t> path(length);
	if (MultiByteToWideChar(CP_ACP, 0, fileName, -1, path.data(), length) == 0)
	{
		return false;
	}
	// 画像ファイルをメモリへ展開する。この段階ではGPU画像を作らない
	const HRESULT status = LoadFromWICFile(path.data(), WIC_FLAGS_NONE, nullptr, *image);

	if (FAILED(status))
	{
		OutputDebugStringA(("Texture could not be loaded: " + key + "\n").c_str());
		return false;
	}

	m_CPUImages.emplace(key, std::move(image));
	return true;
}

std::unordered_map<std::string, ID3D11ShaderResourceView *> Texture::m_TexturePool;
std::unordered_map<std::string, std::unique_ptr<ScratchImage>> Texture::m_CPUImages;
std::mutex Texture::m_Mutex;
// 読み込みの後半：保管した画素データから、DirectXで描画できる画像を作る
void Texture::UploadPrepared()
{
	std::vector<std::string> paths;

	{
		std::lock_guard<std::mutex> lock(m_Mutex);

		for (const auto &entry : m_CPUImages)
		{
			paths.push_back(entry.first);
		}
	}

	for (const auto &path : paths)
	{
		Load(path.c_str());
	}
}

// 失敗したポインターを登録せず、欠損画像は未設定として返す
ID3D11ShaderResourceView *Texture::Load(const char *fileName)
{
	if (!Renderer::GetDevice())
	{
		return nullptr;
	}

	if (!PrepareCPU(fileName))
	{
		return nullptr;
	}

	std::lock_guard<std::mutex> lock(m_Mutex);
	const std::string key = CreateKey(fileName);
	const auto cached = m_TexturePool.find(key);

	if (cached != m_TexturePool.end())
	{
		return cached->second;
	}

	auto &image = *m_CPUImages.at(key);
	ID3D11ShaderResourceView *texture = nullptr;
	// 展開した画素データから、描画に使うGPU画像を作る
	const HRESULT status = CreateShaderResourceView(Renderer::GetDevice(), image.GetImages(), image.GetImageCount(), image.GetMetadata(), &texture);

	if (FAILED(status))
	{
		if (texture)
		{
			texture->Release();
		}

		OutputDebugStringA(("Texture GPU creation failed: " + key + "\n").c_str());
		return nullptr;
	}

	m_TexturePool.emplace(key, texture);
	// GPU画像の登録後、不要になったCPU側の画素データを解放する
	m_CPUImages.erase(key);
	return texture;
}

// シーンを全て破棄した後、デバイスを終了する前に共有画像を解放する
void Texture::UnloadAll()
{
	std::lock_guard<std::mutex> lock(m_Mutex);

	for (const auto &entry : m_TexturePool)
	{
		entry.second->Release();
	}

	m_TexturePool.clear();
	m_CPUImages.clear();
}

// 同じ画像の区切り文字・大小文字の違いを、共有キャッシュの同じキーにする
std::string Texture::CreateKey(const char *fileName)
{
	std::string key(fileName);
	std::replace(key.begin(), key.end(), '/', '\\');
	CharLowerBuffA(&key[0], static_cast<DWORD>(key.size()));
	return key;
}
