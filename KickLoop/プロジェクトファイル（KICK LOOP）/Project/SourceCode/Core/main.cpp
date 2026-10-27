#include "main.h"

#include "GameData.h"

#include "manager.h"

#include <thread>
#include "kosu_s_cri.h"

#include <vector> //可変長配列

#include "mouseInput.h" 

//const wchar_t* CLASS_NAME = L"AppClass";
//const wchar_t* WINDOW_NAME = L"DX11ゲーム";

// マルチバイト版のウィンドウ名
const char* CLASS_NAME = "AppClass";
const char* WINDOW_NAME = "DX11ゲーム";

LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

HWND g_Window;

static bool g_IsWindowMovingOrSizing = false;
static bool g_IsCursorLockEnabled = true;

HWND GetWindow()
{
	return g_Window;
}

//マウス
static void RegisterRawMouse(HWND hWnd) {

	RAWINPUTDEVICE rid{};
	rid.usUsagePage = 0x01; //マウス、キーボード、ジョイスティック、ゲームパッドの判定取得用
	rid.usUsage = 0x02; // マウス
	rid.dwFlags = 0;   
	rid.hwndTarget = hWnd;
	RegisterRawInputDevices(&rid, 1, sizeof(rid));
}

static void SetCursorVisible(bool visible)
{
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
}

static void LockCursorToWindow(HWND hWnd)
{
	RECT rect;

	GetWindowRect(
		hWnd,
		&rect
	);

	ClipCursor(
		&rect
	);
}

static void UnlockCursor()
{
	ClipCursor(nullptr);
}

static void ApplyWindowActiveCursorState(HWND hWnd, bool isActive)
{
	if (isActive && g_IsCursorLockEnabled)
	{
		SetCursorVisible(false);
		LockCursorToWindow(hWnd);
	}
	else
	{
		UnlockCursor();
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

	//initMouse
	RegisterRawMouse(g_Window);


	CoInitializeEx(nullptr, COINITBASE_MULTITHREADED);
    

	//音声ライブラリを初期化する
	AudioManager::Init();

	GameData::InitOption();

	Manager::Init();


	ShowWindow(g_Window, nCmdShow);
	UpdateWindow(g_Window);

	ApplyWindowActiveCursorState(
		g_Window,
		true
	);


	DWORD dwExecLastTime;
	DWORD dwCurrentTime;
	timeBeginPeriod(1);
	dwExecLastTime = timeGetTime();
	dwCurrentTime = 0;


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
			dwCurrentTime = timeGetTime();

			if((dwCurrentTime - dwExecLastTime) >= (1000 / 60))
			{
				dwExecLastTime = dwCurrentTime;

				//CRI
				AudioManager::Update();

				//マウスリセット
				Mouse::Update();
				Manager::Update();
				Manager::Draw();
				Mouse::Clear();
			}
		}
	}
	ApplyWindowActiveCursorState(
		g_Window,
		false
	);


	timeEndPeriod(1);

	UnregisterClass(CLASS_NAME, wcex.hInstance);


	Manager::Uninit();

	//CRI
	AudioManager::Uninit();

	CoUninitialize();

	return (int)msg.wParam;
}


LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_ACTIVATEAPP:
	{
		if (g_IsWindowMovingOrSizing)
		{
			return 0;
		}

		ApplyWindowActiveCursorState(
			hWnd,
			wParam == TRUE
		);

		return 0;
	}

	case WM_ENTERSIZEMOVE:
	{
		g_IsWindowMovingOrSizing = true;

		UnlockCursor();

		SetCursorVisible(
			true
		);

		return 0;
	}

	case WM_EXITSIZEMOVE:
	{
		g_IsWindowMovingOrSizing = false;

		if (GetActiveWindow() == hWnd)
		{
			SetCursorVisible(
				false
			);

			LockCursorToWindow(
				hWnd
			);
		}

		return 0;
	}

	case WM_MOVE:
	case WM_SIZE:
	{
		if (wParam == SIZE_MINIMIZED)
		{
			break;
		}

		if (!g_IsWindowMovingOrSizing && GetActiveWindow() == hWnd && g_IsCursorLockEnabled)
		{
			LockCursorToWindow(
				hWnd
			);
		}

		break;
	}

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

		RAWINPUT* ri =
			reinterpret_cast<RAWINPUT*>(buf.data());

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

