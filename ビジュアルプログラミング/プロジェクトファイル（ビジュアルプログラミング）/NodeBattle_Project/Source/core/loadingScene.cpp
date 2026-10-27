#include "main.h"
#include "loadingScene.h"
#include "polyTex.h"
#include "moji.h"
#include "textObjectBase.h"
#include "camera.h"

bool LoadingScene::Init()
{
	AddGameObject3D<Camera>(0);

	AddGameObject2D<PolyTex>(0)->SetTexRule(Vector2(0, 0), 0, Vector2(SCREEN_WIDTH, SCREEN_HEIGHT), "asset\\texture\\wallpaper\\titleBattle.png");

	AddGameObject2D<PolyTex>(1)->SetTexRule(Vector2(370, 260), 0, Vector2(540, 200), "asset\\texture\\kuro.png")->SetColor(XMFLOAT4(1, 1, 1, 0.85f));

	// 点の数を変えて、読み込みが続いていることを伝える
	m_Frame = 0;
	m_LoadingText = AddGameObject2D<TextObjectBase>(2);
	m_LoadingText->SetPosition(Vector2(435, 300));
	FontData font;
	font.font = L"メイリオ";
	font.fontSize = 32;
	m_LoadingText->SetFont(font);
	m_LoadingText->SetColor(0.65f, 1.0f, 0.8f, 1.0f);
	m_LoadingText->SetText(L"読み込み中");

	return true;
}

void LoadingScene::Uninit()
{
	Scene::Uninit();
	m_LoadingText = nullptr;
}

void LoadingScene::Update()
{
	Scene::Update();

	// 点の増減は待機中のアニメーションで、実際の読み込み進捗を示すものではない
	m_Frame = (m_Frame + 1) % 120;
	std::wstring text = L"読み込み中";

	for (int i = 0; i < m_Frame / 30; ++i)
	{
		text += L"・";
	}

	m_LoadingText->SetText(text.c_str());
}

void LoadingScene::Draw()
{
	Scene::Draw();

	FontData font;
	font.font = L"メイリオ";
	font.fontSize = 20;
	font.Color = D2D1::ColorF(D2D1::ColorF::White);
	Moji::Instance().SetFont(font);
	Moji::Instance().Draw(L"ノードとフィールドを準備しています", D2D1::Point2F(435, 385));
}