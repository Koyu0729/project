#include "main.h"
#include "manager.h"
#include "renderer.h"
#include "texture.h"

// テクスチャキャッシュの静的メンバーを定義する
std::unordered_map<std::string, ID3D11ShaderResourceView*> Texture::m_TexturePool;

ID3D11ShaderResourceView* Texture::Load(const char* FileName)
{
    // 同じ画像の重複読み込みを防ぐ
    if (m_TexturePool.count(FileName) > 0)
    {
        // 読み込み済みのテクスチャを返す
        return m_TexturePool[FileName];
    }

    wchar_t wFileName[512];
    mbstowcs(wFileName, FileName, strlen(FileName) + 1);

    TexMetadata metadata;
    ScratchImage image;
    ID3D11ShaderResourceView* texture;

    LoadFromWICFile(wFileName, WIC_FLAGS_NONE, &metadata, image);

    CreateShaderResourceView(Renderer::GetDevice(), image.GetImages(), image.GetImageCount(), metadata, &texture);

    assert(texture);

    m_TexturePool[FileName] = texture;

    return texture;
}

void Texture::Unload()
{
    for (auto& uo_M : m_TexturePool)
    {
        if (uo_M.second != nullptr)
        {
            uo_M.second->Release();
            uo_M.second = nullptr;
        }
    }

    m_TexturePool.clear();
}
