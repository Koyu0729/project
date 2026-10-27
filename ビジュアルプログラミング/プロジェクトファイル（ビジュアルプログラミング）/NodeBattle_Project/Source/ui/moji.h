#pragma once
#include <d3d11.h>
#include <d2d1.h>
#include <dwrite.h>
#include <wrl/client.h>
#include <string>
#include <vector>

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d2d1.lib")
#pragma comment(lib, "dwrite.lib")

class CustomFontCollectionLoader;

//フォントの保存場所
namespace FontList
{
    const std::wstring FontPath[] =
    {
        L"font\\ZeroGothic.otf",
    };
}

struct FontData
{
    std::wstring font;							// フォント名
    IDWriteFontCollection* fontCollection;		// フォントコレクション
    DWRITE_FONT_WEIGHT fontWeight;				// フォントの太さ
    DWRITE_FONT_STYLE fontStyle;				// フォントスタイル
    DWRITE_FONT_STRETCH fontStretch;			// フォントの幅
    FLOAT fontSize;								// フォントサイズ
    WCHAR const* localeName;					// ロケール名
    DWRITE_TEXT_ALIGNMENT textAlignment;		// テキストの配置
    D2D1_COLOR_F Color;							// フォントの色

    D2D1_COLOR_F shadowColor;					// 影の色
    D2D1_POINT_2F shadowOffset;					// 影のオフセット

    // フォント設定
    // 第1引数：フォント名（L"メイリオ", L"Arial", L"Meiryo UI"等）
    // 第2引数：フォントの太さ（DWRITE_FONT_WEIGHT_NORMAL, DWRITE_FONT_WEIGHT_BOLD等）
    // 第3引数：フォントスタイル（DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STYLE_OBLIQUE, DWRITE_FONT_STYLE_ITALIC）
    // 第4引数：フォントの幅（DWRITE_FONT_STRETCH_NORMAL,DWRITE_FONT_STRETCH_EXTRA_EXPANDED等）
    // 第5引数：フォントサイズ（20, 30等）
    // 第6引数：ロケール名（L"ja-jp"等）
    // 第7引数：テキストの配置（DWRITE_TEXT_ALIGNMENT_LEADING：前, 等）
    // 第8引数：フォントの色（D2D1::ColorF(D2D1::ColorF::Black)：黒, D2D1::ColorF(D2D1::ColorF(0.0f, 0.2f, 0.9f, 1.0f))：RGBA指定等）
    // 第9引数：影の色（D2D1::ColorF(D2D1::ColorF::Black)：黒, D2D1::ColorF(D2D1::ColorF(0.0f, 0.2f, 0.9f, 1.0f))：RGBA指定等）
    // 第10引数：影のオフセット（D2D1::Point2F(2.0f, 2.0f)：右下にポイントずらす）
    // デフォルト設定
    FontData()
    {
        font = L"メイリオ";
        fontCollection = nullptr;
        fontWeight = DWRITE_FONT_WEIGHT::DWRITE_FONT_WEIGHT_NORMAL;
        fontStyle = DWRITE_FONT_STYLE::DWRITE_FONT_STYLE_NORMAL;
        fontStretch = DWRITE_FONT_STRETCH::DWRITE_FONT_STRETCH_NORMAL;
        fontSize = 20;
        localeName = L"ja-jp";
        textAlignment = DWRITE_TEXT_ALIGNMENT::DWRITE_TEXT_ALIGNMENT_LEADING;
        Color = D2D1::ColorF(D2D1::ColorF::White);

        shadowColor = D2D1::ColorF(D2D1::ColorF::Black);
        shadowOffset = D2D1::Point2F(2.0f, -2.0f);
    }
};

// フォント設定と、再利用する文字形式をまとめて保持する

struct FontOption
{
    FontData data;
    Microsoft::WRL::ComPtr<IDWriteTextFormat> format;
};

class Moji
{
public:
    bool InitWithDX11(ID3D11Texture2D* backBuffer);

    // 解放
    void Uninit();

    // 文字描画
    HRESULT Draw(const std::wstring& text, D2D1_POINT_2F pos, float zoomScale = 1.0f, D2D1_DRAW_TEXT_OPTIONS options = D2D1_DRAW_TEXT_OPTIONS_NONE, bool shadow = false);
    // 文字描画を管理する共通のインスタンスを取得する
    static Moji& Instance()
    {
        static Moji instance;
        return instance;
    }

    // フォント設定代入
    HRESULT SetFont(FontData set);

    HRESULT SetFont(
        const WCHAR* fontname,
        DWRITE_FONT_WEIGHT fontWeight,
        DWRITE_FONT_STYLE fontStyle,
        DWRITE_FONT_STRETCH fontStretch,
        FLOAT fontSize,
        const WCHAR* localeName,
        DWRITE_TEXT_ALIGNMENT textAlignment,
        D2D1_COLOR_F color,
        D2D1_COLOR_F shadowColor,
        D2D1_POINT_2F shadowOffset);

private:
    Moji() = default;
    // コピー禁止
    Moji(const Moji&) = delete;

    ~Moji() = default;
    Moji& operator=(const Moji&) = delete;

    // フォントのファイル名を取得する
    WCHAR* GetFontFileNameWithoutExtension(const std::wstring& filePath);
private:
    Microsoft::WRL::ComPtr <ID2D1Factory>			m_D2DFactory = nullptr;		// Direct2Dリソース
    Microsoft::WRL::ComPtr <ID2D1RenderTarget>		m_RenderTarget = nullptr;	// Direct2Dレンダーターゲット
    Microsoft::WRL::ComPtr <ID2D1SolidColorBrush>	m_Brush = nullptr;			// Direct2Dブラシ設定
    Microsoft::WRL::ComPtr <ID2D1SolidColorBrush>	m_ShadowBrush = nullptr;	// Direct2Dブラシ設定（影）
    Microsoft::WRL::ComPtr <IDWriteFactory>		    m_DWriteFactory = nullptr;  // DirectWriteリソース
    Microsoft::WRL::ComPtr <IDWriteTextFormat>		m_TextFormat = nullptr;		// DirectWriteテキスト形式
    Microsoft::WRL::ComPtr <IDWriteTextLayout>		m_TextLayout = nullptr;		// DirectWriteテキスト書式
    Microsoft::WRL::ComPtr <IDXGISurface>			m_BackBuffer = nullptr;		// サーフェス情報

    std::vector<Microsoft::WRL::ComPtr<IDWriteFontCollection>>   m_FontCollection;
    // フォントファイルリスト
    std::vector<Microsoft::WRL::ComPtr<IDWriteFontFile>> m_FontFileList;

    // フォントデータ
    FontData  Setting = FontData();

    // フォント名リスト
    std::vector<std::wstring> m_FontNamesList;

    // 同じフォント設定の文字形式を再利用するためのキャッシュ
    std::vector<FontOption> m_FontCache;
};

