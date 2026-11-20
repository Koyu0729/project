#include "main.h"
#include "renderer.h"
#include "texture.h"

#include <stdexcept>
#include <wrl/client.h>

// テクスチャキャッシュの静的メンバーを定義する
std::unordered_map<std::string, ID3D11ShaderResourceView*> Texture::m_TexturePool;

ID3D11ShaderResourceView* Texture::Load(const char* fileName)
{
    if (fileName == nullptr || *fileName == '\0')
    {
        return nullptr;
    }

    const auto cached = m_TexturePool.find(fileName);
    if (cached != m_TexturePool.end())
    {
        return cached->second;
    }

    const int length = MultiByteToWideChar(CP_ACP, MB_ERR_INVALID_CHARS, fileName, -1, nullptr, 0);
    if (length <= 0)
    {
        throw std::runtime_error(std::string("Invalid texture path: ") + fileName);
    }

    std::wstring wideName(length, L'\0');
    if (!MultiByteToWideChar(CP_ACP, MB_ERR_INVALID_CHARS, fileName, -1, &wideName[0], length))
    {
        throw std::runtime_error(std::string("Invalid texture path: ") + fileName);
    }

    TexMetadata metadata{};
    ScratchImage image;
    if (FAILED(LoadFromWICFile(wideName.c_str(), WIC_FLAGS_NONE, &metadata, image)))
    {
        throw std::runtime_error(std::string("Could not load texture: ") + fileName);
    }

    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> texture;
    if (FAILED(CreateShaderResourceView(
        Renderer::GetDevice(),
        image.GetImages(),
        image.GetImageCount(),
        metadata,
        texture.GetAddressOf()
    )))
    {
        throw std::runtime_error(std::string("Could not create texture: ") + fileName);
    }

    m_TexturePool.emplace(fileName, texture.Get());
    return texture.Detach();
}

void Texture::Unload()
{
    for (auto& entry : m_TexturePool)
    {
        if (entry.second != nullptr)
        {
            entry.second->Release();
            entry.second = nullptr;
        }
    }

    m_TexturePool.clear();
}
