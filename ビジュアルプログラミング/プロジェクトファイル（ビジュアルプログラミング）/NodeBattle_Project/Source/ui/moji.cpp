#include "main.h"
#include <stdexcept>
#include <filesystem>

#include "moji.h"

#include "fontList.h"
#include "customFontLoader.h"

CustomFontCollectionLoader* gFontCollectionLoader = nullptr;

// 文字描画に必要なリソースを初期化する
bool Moji::InitWithDX11(ID3D11Texture2D* backBuffer)
{
    HRESULT hr = S_OK;

    //Direct2DのFactory作成
    hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, m_D2DFactory.ReleaseAndGetAddressOf());
    if (FAILED(hr)) return false;

    // DirectWriteのFactory作成
    hr = DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory),
        reinterpret_cast<IUnknown**>(m_DWriteFactory.ReleaseAndGetAddressOf()));
    if (FAILED(hr)) return false;

    // バックバッファをDXGIサーフェイスに変換
    hr = backBuffer->QueryInterface(__uuidof(IDXGISurface),
        reinterpret_cast<void**>(m_BackBuffer.ReleaseAndGetAddressOf()));
    if (FAILED(hr)) return false;

    // D2Dのレンダーターゲット作成
    D2D1_RENDER_TARGET_PROPERTIES props =
        D2D1::RenderTargetProperties(
            D2D1_RENDER_TARGET_TYPE_DEFAULT,
            D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED),
            96.0f, 96.0f);

    hr = m_D2DFactory->CreateDxgiSurfaceRenderTarget(m_BackBuffer.Get(), &props, m_RenderTarget.ReleaseAndGetAddressOf());
    if (FAILED(hr)) return false;

    // ブラシ作成
    m_RenderTarget->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::White), m_Brush.ReleaseAndGetAddressOf());
    m_RenderTarget->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Black), m_ShadowBrush.ReleaseAndGetAddressOf());

    if (!gFontCollectionLoader)
    {
        gFontCollectionLoader = new CustomFontCollectionLoader();
        hr = m_DWriteFactory->RegisterFontCollectionLoader(gFontCollectionLoader);
        if (FAILED(hr))
        {
            delete gFontCollectionLoader;
            gFontCollectionLoader = nullptr;
            return false;
        }
    }

    // フォントごとのコレクションを作成
    for (const auto& path : FontList::FontPath)
    {
        std::vector<std::wstring> key{ path }; // コレクションキー

        Microsoft::WRL::ComPtr<IDWriteFontCollection> fontCollection;
        hr = m_DWriteFactory->CreateCustomFontCollection(
            gFontCollectionLoader,
            key.data(),
            static_cast<UINT32>(key.size() * sizeof(std::wstring)),
            &fontCollection);

        if (SUCCEEDED(hr))
        {
            m_FontCollection.push_back(fontCollection);
        }
    }

    return true;
}

void Moji::Uninit()
{
    if (m_DWriteFactory && gFontCollectionLoader)
    {
        m_DWriteFactory->UnregisterFontCollectionLoader(gFontCollectionLoader);
        delete gFontCollectionLoader;
        gFontCollectionLoader = nullptr;
    }
    // 保持している文字形式のキャッシュを解放する
    m_FontCache.clear();

    m_TextFormat.Reset();
    m_TextLayout.Reset();
    m_Brush.Reset();
    m_ShadowBrush.Reset();
    m_RenderTarget.Reset();
    m_DWriteFactory.Reset();
    m_D2DFactory.Reset();
    m_BackBuffer.Reset();

    m_FontCollection.clear();
}

HRESULT Moji::Draw(const std::wstring& text, D2D1_POINT_2F pos, float zoomScale, D2D1_DRAW_TEXT_OPTIONS options, bool shadow)
{
    if (!m_RenderTarget || !m_TextFormat) return E_FAIL;

    HRESULT hr = S_OK;

    // ターゲットサイズを取得
    D2D1_SIZE_F size = m_RenderTarget->GetSize();

    // レイアウト作成
    hr = m_DWriteFactory->CreateTextLayout(
        text.c_str(),
        static_cast<UINT32>(text.size()),
        m_TextFormat.Get(),
        size.width,
        size.height,
        m_TextLayout.ReleaseAndGetAddressOf()
    );
    if (FAILED(hr)) return hr;

    m_RenderTarget->BeginDraw();

    // スケーリング
    m_RenderTarget->SetTransform(
        D2D1::Matrix3x2F::Scale(zoomScale, zoomScale, D2D1::Point2F(pos.x, pos.y))
    );

    // 影描画（オプション）
    if (shadow && m_ShadowBrush)
    {
        D2D1_POINT_2F shadowPos = D2D1::Point2F(
            pos.x + Setting.shadowOffset.x,
            pos.y + Setting.shadowOffset.y
        );

        m_RenderTarget->DrawTextLayout(
            shadowPos,
            m_TextLayout.Get(),
            m_ShadowBrush.Get(),
            options
        );
    }

    // 本体描画
    m_RenderTarget->DrawTextLayout(
        pos,
        m_TextLayout.Get(),
        m_Brush.Get(),
        options
    );

    hr = m_RenderTarget->EndDraw();

    return hr;
}

HRESULT Moji::SetFont(FontData set)
{
    for (auto& entry : m_FontCache)
    {
        if (entry.data.font == set.font &&
            entry.data.fontWeight == set.fontWeight &&
            entry.data.fontStyle == set.fontStyle &&
            entry.data.fontStretch == set.fontStretch &&
            entry.data.fontSize == set.fontSize &&
            wcscmp(entry.data.localeName, set.localeName) == 0)
        {
            // 既存を再利用（CreateTextFormat回避）
            m_TextFormat = entry.format;
            Setting = set;
            if (m_Brush) m_Brush->SetColor(set.Color);
            if (m_ShadowBrush) m_ShadowBrush->SetColor(set.shadowColor);
            return S_OK;
        }
    }
    Setting = set; // 設定をコピーして保持
    return SetFont(
        set.font.c_str(),
        set.fontWeight,
        set.fontStyle,
        set.fontStretch,
        set.fontSize,
        set.localeName,
        set.textAlignment,
        set.Color,
        set.shadowColor,
        set.shadowOffset
    );
}

HRESULT Moji::SetFont(
    const WCHAR* fontname,
    DWRITE_FONT_WEIGHT fontWeight,
    DWRITE_FONT_STYLE fontStyle,
    DWRITE_FONT_STRETCH fontStretch,
    FLOAT fontSize,
    const WCHAR* localeName,
    DWRITE_TEXT_ALIGNMENT textAlignment,
    D2D1_COLOR_F color,
    D2D1_COLOR_F shadowColor,
    D2D1_POINT_2F shadowOffset)
{
    if (!m_DWriteFactory || !m_RenderTarget) return E_FAIL;

    IDWriteFontCollection* useCollection = nullptr;
    if (!m_FontCollection.empty())
    {
        useCollection = m_FontCollection[0].Get();
    }

    HRESULT hr = m_DWriteFactory->CreateTextFormat(
        fontname,
        useCollection,
        fontWeight,
        fontStyle,
        fontStretch,
        fontSize,
        localeName,
        m_TextFormat.ReleaseAndGetAddressOf());

    if (FAILED(hr))
    {
        OutputDebugStringW(L"CreateTextFormat failed!\n");
        return hr;
    }
    FontData cacheData;
    cacheData.font = fontname;
    cacheData.fontWeight = fontWeight;
    cacheData.fontStyle = fontStyle;
    cacheData.fontStretch = fontStretch;
    cacheData.fontSize = fontSize;
    cacheData.localeName = localeName;
    cacheData.textAlignment = textAlignment;
    cacheData.Color = color;
    cacheData.shadowColor = shadowColor;
    cacheData.shadowOffset = shadowOffset;

    FontOption entry;
    entry.data = cacheData;
    entry.format = m_TextFormat;
    m_FontCache.push_back(entry);

    OutputDebugStringW((L"CreateTextFormat succeeded for font: " + std::wstring(fontname) + L"\n").c_str());

    m_TextFormat->SetTextAlignment(textAlignment);
    m_Brush->SetColor(color);
    m_ShadowBrush->SetColor(shadowColor);

    Setting.shadowOffset = shadowOffset;

    return S_OK;
}
// 拡張子を除くファイル名を返す
WCHAR* Moji::GetFontFileNameWithoutExtension(const std::wstring& filePath)
{
    // 末尾から検索してファイル名と拡張子の位置を取得
    size_t start = filePath.find_last_of(L"/\\") + 1;
    size_t end = filePath.find_last_of(L'.');

    // ファイル名を取得
    std::wstring fileNameWithoutExtension = filePath.substr(start, end - start).c_str();

    // 新しいWCHAR配列を作成
    WCHAR* fileName = new WCHAR[fileNameWithoutExtension.length() + 1];

    // 文字列をコピー
    wcscpy_s(fileName, fileNameWithoutExtension.length() + 1, fileNameWithoutExtension.c_str());

    // ファイル名を返す
    return fileName;
}
