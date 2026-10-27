#pragma once

class Scene;
class Fade;

class Manager
{
private:
	static Scene* m_Scene;
	static Scene* m_SceneNext;
	static Fade m_Fade;
	static bool m_LoadSceneInit;

	//リザルトタイム用
	static float m_ResultTime;
public:
	static void Init();
	static void Uninit();
	static void Update();
	static void Draw();

	//リザルトタイム用
	static void SetResultTime(float time) { m_ResultTime = time; }
	static float GetResultTime() { return m_ResultTime; }

	template <typename T>
	static void SetScene()
	{
		m_SceneNext = new T();
		m_LoadSceneInit = true;
	}

	static void SetNextScene(Scene* next, bool isFade = true, bool loadInit = true);

	static Scene* GetScene() { return m_Scene; }
};