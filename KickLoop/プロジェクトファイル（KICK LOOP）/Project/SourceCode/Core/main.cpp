#include "main.h"

#include "GameData.h"

#include "manager.h"

#include <chrono>
#include <exception>
#include <thread>
#include "kosu_s_cri.h"

#include <vector> //可変長配列

#include "mouseInput.h" 

// ウィンドウクラス名とタイトル
const char* CLASS_NAME = "AppClass";
const char* WINDOW_NAME = "DX11ゲーム";

LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

HWND g_Window;

static bool g_IsWindowMovingOrSizing = false;
static bool g_IsCursorLockEnabled = true;
static bool g_IsCursorVisible = true;

HWND GetWindow()
{
	return g_Window;
}

// マウスのRaw Inputを登録する
static void RegisterRawMouse(HWND hWnd) {

	RAWINPUTDEVICE rid{};
	rid.usUsagePage = 0x01; // Generic Desktopページ
	rid.usUsage = 0x02; // マウス
	rid.dwFlags = 0;   
	rid.hwndTarget = hWnd;
	RegisterRawInputDevices(&rid, 1, sizeof(rid));
}

static void SetCursorVisible(bool visible)
{
	if (visible == g_IsCursorVisible)
	{
		return;
	}

	if (visible)
	{
		while (ShowCursor(TRUE) < 0)
		{
		}
	}
	else
	{
		while (ShowCursor(FALSE) >= 0)
		{
		}
	}

	g_IsCursorVisible = visible;
}

static void ApplyWindowActiveCursorState(HWND hWnd, bool isActive)
{
	RECT rect{};
	const bool shouldLock =
		isActive && g_IsCursorLockEnabled && !g_IsWindowMovingOrSizing &&
		!IsIconic(hWnd) && GetWindowRect(hWnd, &rect);

	if (shouldLock && ClipCursor(&rect))
	{
		SetCursorVisible(false);
	}
	else
	{
		ClipCursor(nullptr);
		SetCursorVisible(true);
	}
}

void SetCursorLockEnabled(bool enabled)
{
	g_IsCursorLockEnabled = enabled;

	if (GetActiveWindow() == g_Window)
	{
		ApplyWindowActiveCursorState(g_Window, true);
	}
}

static int RunGameLoop()
{
	using Clock = std::chrono::steady_clock;
	const std::chrono::duration<double> frameTime(1.0 / GAME_FPS);
	auto nextFrameTime = Clock::now() + frameTime;


	MSG msg;
	while(1)
	{
        if(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			if(msg.message == WM_QUIT)
			{
				break;
			}
			else
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
        }
		else
		{
			const auto currentTime = Clock::now();

			if (currentTime >= nextFrameTime)
			{
				nextFrameTime += frameTime;
				if (currentTime >= nextFrameTime)
				{
					// 1フレーム以上遅れたら、現在時刻を基準に待ち直す。
					nextFrameTime = currentTime + frameTime;
				}

				//CRI
				AudioManager::Update();

				// ボタンの押下・解放を確定する
				Mouse::Update();
				Manager::Update();
				Manager::Draw();
				Mouse::Clear(); // 移動量とホイール量をリセットする
			}
		}
	}

	return (int)msg.wParam;
}

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	// 実行ファイルの場所を基準に素材を読み込む
	wchar_t executablePath[32768]{};
	DWORD pathLength = GetModuleFileNameW(nullptr, executablePath, 32768);
	if (pathLength == 0 || pathLength >= 32768) return 1;
	for (DWORD i = pathLength; i > 0; --i)
	{
		if (executablePath[i - 1] == L'\\')
		{
			executablePath[i - 1] = L'\0';
			if (!SetCurrentDirectoryW(executablePath)) return 1;
			break;
		}
	}

#ifdef _DEBUG
	AllocConsole();
	FILE* fp;
	freopen_s(&fp, "CONOUT$", "w", stdout);

	printf("デバック用\n");
#endif

	WNDCLASSEX wcex;
	{
		wcex.cbSize = sizeof(WNDCLASSEX);
		wcex.style = 0;
		wcex.lpfnWndProc = WndProc;
		wcex.cbClsExtra = 0;
		wcex.cbWndExtra = 0;
		wcex.hInstance = hInstance;
		wcex.hIcon = nullptr;
		wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
		wcex.hbrBackground = nullptr;
		wcex.lpszMenuName = nullptr;
		wcex.lpszClassName = CLASS_NAME;
		wcex.hIconSm = nullptr;

		RegisterClassEx(&wcex);


		RECT rc = { 0, 0, (LONG)SCREEN_WIDTH, (LONG)SCREEN_HEIGHT };
		AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);

		g_Window = CreateWindowEx(0, CLASS_NAME, WINDOW_NAME, WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT,
			rc.right - rc.left, rc.bottom - rc.top, nullptr, nullptr, hInstance, nullptr);
	}

	RegisterRawMouse(g_Window);


	CoInitializeEx(nullptr, COINITBASE_MULTITHREADED);
    

	//音声ライブラリを初期化する
	AudioManager::Init();

	GameData::InitOption();

	int exitCode = 1;
	bool timerStarted = false;

	try
	{
		Manager::Init();

		ShowWindow(g_Window, nCmdShow);
		UpdateWindow(g_Window);
		ApplyWindowActiveCursorState(g_Window, GetActiveWindow() == g_Window);

		timerStarted = timeBeginPeriod(1) == TIMERR_NOERROR;
		exitCode = RunGameLoop();
	}
	catch (const std::exception& error)
	{
		g_IsCursorLockEnabled = false;
		ApplyWindowActiveCursorState(g_Window, false);
		MessageBoxA(g_Window, error.what(), "KICK LOOP", MB_OK | MB_ICONERROR);
		DestroyWindow(g_Window);
	}

	ApplyWindowActiveCursorState(
		g_Window,
		false
	);


	if (timerStarted)
	{
		timeEndPeriod(1);
	}

	UnregisterClass(CLASS_NAME, wcex.hInstance);


	Manager::Uninit();

	//CRI
	AudioManager::Uninit();

	CoUninitialize();

	return exitCode;
}


LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_ACTIVATEAPP:
		ApplyWindowActiveCursorState(hWnd, wParam == TRUE);
		return 0;

	case WM_ENTERSIZEMOVE:
		g_IsWindowMovingOrSizing = true;
		ApplyWindowActiveCursorState(hWnd, GetActiveWindow() == hWnd);
		return 0;

	case WM_EXITSIZEMOVE:
		g_IsWindowMovingOrSizing = false;
		ApplyWindowActiveCursorState(hWnd, GetActiveWindow() == hWnd);
		return 0;

	case WM_MOVE:
	case WM_SIZE:
		if (GetActiveWindow() == hWnd)
		{
			ApplyWindowActiveCursorState(hWnd, true);
		}
		break;

	case WM_DESTROY:
	{
		ApplyWindowActiveCursorState(
			hWnd,
			false
		);

		PostQuitMessage(
			0
		);

		return 0;
	}

	case WM_KEYDOWN:
	{
		switch (wParam)
		{
#ifdef _DEBUG
		case VK_F12:
			DestroyWindow(
				hWnd
			);
			break;
#endif
		}

		break;
	}

	case WM_INPUT:
	{
		auto hRawInput = (HRAWINPUT)lParam;
		UINT dwSize = 0;
		std::vector<BYTE> buf;

		GetRawInputData(
			hRawInput,
			RID_INPUT,
			nullptr,
			&dwSize,
			sizeof(RAWINPUTHEADER)
		);

		buf.resize(
			dwSize
		);

		GetRawInputData(
			hRawInput,
			RID_INPUT,
			buf.data(),
			&dwSize,
			sizeof(RAWINPUTHEADER)
		);

		RAWINPUT* ri = reinterpret_cast<RAWINPUT*>(buf.data());

		if (ri->header.dwType != RIM_TYPEMOUSE)
		{
			break;
		}

		Mouse::SetMouseMove(
			ri->data.mouse.lLastX,
			ri->data.mouse.lLastY
		);

		if (ri->data.mouse.usButtonFlags & RI_MOUSE_LEFT_BUTTON_DOWN)  Mouse::LeftDown();
		if (ri->data.mouse.usButtonFlags & RI_MOUSE_LEFT_BUTTON_UP)    Mouse::LeftUp();

		if (ri->data.mouse.usButtonFlags & RI_MOUSE_RIGHT_BUTTON_DOWN) Mouse::RightDown();
		if (ri->data.mouse.usButtonFlags & RI_MOUSE_RIGHT_BUTTON_UP)   Mouse::RightUp();

		return 0;
	}

	default:
		break;
	}

	return DefWindowProc(
		hWnd,
		uMsg,
		wParam,
		lParam
	);
}
