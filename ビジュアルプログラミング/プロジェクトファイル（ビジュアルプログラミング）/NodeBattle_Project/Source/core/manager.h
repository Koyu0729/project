// シーンの読み込み中はLoadingSceneを表示する
#pragma once
#include <new>
#include <thread> //別スレッド用
#include <atomic> // スレッド間で共有する読み込み状態の管理
#include "fade.h"

#include "scene.h"
#include "loadingScene.h"

// シーンの切り替え状態
#include "nodeBase.h"
#include "button.h"

#include "kosuCRI.h"

enum class ManagerState
{
	Default,
	CreateLoadScene, // ロードシーン移動
	LoadingScene,	 // CPUロード処理中
	ChangeScene		 // シーン移動
};
// シーンの生成・読み込み・切り替えを管理する
class Manager
{

  private:
	static ManagerState m_State;

	static class Scene *m_Scene; // 現在のシーンを格納する
	// 読み込み中の次シーン。準備完了後にm_Sceneへ渡す
	static class Scene *m_SceneNext;

	// 初期化中のシーンと、破棄時にGPU終了処理が必要かを管理する
	static Scene *m_PreparingScene;
	static bool m_NextGPUStarted;
	// 読み込みの失敗だけをメイン側へ伝える（スレッド終了後に確認する）
	static bool m_LoadFailed;

	static Fade m_Fade;

	// 別スレッドが読み込み中かを通知する。falseになった後、joinで終了を確認する
	static std::atomic<bool> m_Loading;
	// ファイルの読み込みを行うスレッド。GPUの初期化はメイン側で行う
	static std::thread m_LoadThread;

		// 次に生成するシーンの関数を保持し、読み込み開始時に呼び出す
	static Scene *(*m_CreateScene)();
	static void PrepareSceneLoad(Scene *(*createScene)());

	static void LoadSceneCPU();
	static void ReturnToTitle();

	// 指定した種類のシーンを生成する
	template <typename T>
	static Scene *CreateScene()
	{
		return new (std::nothrow) T();
	}

  public:
	static void Init();	  // 初期化
	static void Uninit(); // 終了
	static void Update(); // 更新
	static void Draw();	  // 描画
	// 読み込みの開始と、保留中のシーンの取り消し
	static bool StartLoading();
	static void ClearPendingScene();

	// CPUの読み込みスレッドの終了を待つ
	static void WaitForLoading();

	// シーンの種類を指定して、ロードを予約する
	template <typename T>
	static void LoadingSetScene()
	{
		PrepareSceneLoad(&CreateScene<T>);
	}



	static Scene *GetScene()
	{
		if (m_PreparingScene)
		{
			return m_PreparingScene;
		}

		return m_Scene;
	}


	// シーンの切り替え状態を取得する
	static ManagerState GetState();


};
