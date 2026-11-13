#include "main.h"
#include <algorithm>
#include "manager.h"
#include "nodeEditor.h"
#include <thread>
#include "kosuCRI.h"

#include "mouseInput.h"

#include <vector>

static void UpdateCursorConfinement(HWND window, bool enabled);
static void RegisterRawMouse(HWND window);
static bool SelectAssetDirectory();
static bool HasAssetDirectory(const std::wstring &path);

// ゲーム画面を登録するときのクラス名と、ウィンドウの表示名
const char *CLASS_NAME = "AppClass";

const char *WINDOW_NAME = "DX11ゲーム";

// マウス入力や描画に使うゲーム画面
HWND g_Window;

// カーソルの移動制限を有効にするか
static bool g_CursorConfinementEnabled = false;

// 移動やノード実行に渡す、前のフレームからの経過時間
static float g_FrameDeltaSeconds = 1.0f / 60.0f;

// 今回のフレームで進める時間を取得する
float GetFrameDeltaSeconds()
{
	return g_FrameDeltaSeconds;
}

// ゲーム画面を入力処理などから取得する
HWND GetGameWindow()
{
	return g_Window;
}

// マウスの入力通知を受け取る一時データ
static std::vector<BYTE> g_rawBuf;

// 起動時の準備、ゲームの更新、終了処理を行う
int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	if (!SelectAssetDirectory())
	{
		MessageBoxW(nullptr, L"assetフォルダーが見つかりません。ゲームのフォルダー内から起動してください。", L"起動できません", MB_OK | MB_ICONERROR);

		return 2;
	}

#ifdef _DEBUG
	// Debug版では開発用コンソールを表示する
	FILE *fp = nullptr;
	AllocConsole();
	freopen_s(&fp, "CONOUT$", "w", stdout);
	printf("デバッグ用\n");
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

		RECT rc = {0, 0, (LONG)SCREEN_WIDTH, (LONG)SCREEN_HEIGHT};

		AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);

		g_Window = CreateWindowEx(0, CLASS_NAME, WINDOW_NAME, WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT,
								  rc.right - rc.left, rc.bottom - rc.top, nullptr, nullptr, hInstance, nullptr);
	}

	// マウス入力を初期化する
	RegisterRawMouse(g_Window);

	CoInitializeEx(nullptr, COINITBASE_MULTITHREADED);

	// 音声を初期化する
	printf("Initializing audio\n");

	AudioManager::Init();
	printf("Initializing renderer and title\n");

	Manager::Init();

	ShowWindow(g_Window, nCmdShow);

	UpdateWindow(g_Window);

	ShowCursor(FALSE);

	g_CursorConfinementEnabled = true;

	UpdateCursorConfinement(g_Window, g_CursorConfinementEnabled);

	DWORD dwExecLastTime;

	DWORD dwCurrentTime;

	timeBeginPeriod(1);

	dwExecLastTime = timeGetTime();

	dwCurrentTime = 0;

	MSG msg;

	while (1)
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			if (msg.message == WM_QUIT)
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

			if ((dwCurrentTime - dwExecLastTime) >= (1000 / 60))
			{
				g_FrameDeltaSeconds = (std::min)(0.1f, (dwCurrentTime - dwExecLastTime) / 1000.0f);

				dwExecLastTime = dwCurrentTime;

				// CRI
				AudioManager::Update();

				Mouse::Update();

				Manager::Update();

				Manager::Draw();

				Mouse::Clear();
				// FPS計測＆ウィンドウタイトル表示
				static int frameCount = 0;

				static DWORD lastTime = timeGetTime();

				frameCount++;

				DWORD now = timeGetTime();

				if (now - lastTime >= 1000) // 1秒ごとに更新
				{
					float fps = frameCount * 1000.0f / (now - lastTime);

					frameCount = 0;

					lastTime = now;

					char title[256];

					sprintf_s(title, "DX11ゲーム - FPS: %.1f", fps);

					SetWindowTextA(g_Window, title);
				}
			}
		}
	}

	ShowCursor(TRUE);

	ClipCursor(NULL); // 閉じ込め解除

	timeEndPeriod(1);

	UnregisterClass(CLASS_NAME, wcex.hInstance);

	// CRI
	Manager::Uninit();

	AudioManager::Uninit();

	CoUninitialize();

	return (int)msg.wParam;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{

	switch (uMsg)
	{

	case WM_ACTIVATE:
		if (LOWORD(wParam) == WA_INACTIVE)
		{
			ClipCursor(nullptr);
		}
		else
		{
			UpdateCursorConfinement(hWnd, g_CursorConfinementEnabled);
		}
		break;

	case WM_MOVE:
	case WM_SIZE:
		UpdateCursorConfinement(hWnd, g_CursorConfinementEnabled);
		break;

	case WM_DESTROY:
		g_CursorConfinementEnabled = false;
		ClipCursor(nullptr);
		PostQuitMessage(0);

		break;

	case WM_KEYDOWN:

		switch (wParam)
		{
		case VK_ESCAPE:
		{
			// 保存・名前変更・削除の画面では、編集側でキャンセルする
			auto *editor = dynamic_cast<NodeEditor *>(Manager::GetScene());

			if ((editor && editor->IsEditorDialogOpen()) || (lParam & (1LL << 30)))
			{
				return 0;
			}

			DestroyWindow(hWnd);

			break;
		}
		}

		break;

	case WM_INPUT:
	{
		// https://qiita.com/i_saint/items/957d76a238bd8f046b64

		auto hRawInput = (HRAWINPUT)lParam;

		UINT dwSize = 0;

		std::vector<BYTE> buf;

		GetRawInputData(hRawInput, RID_INPUT, nullptr, &dwSize, sizeof(RAWINPUTHEADER));

		buf.resize(dwSize);

		GetRawInputData(hRawInput, RID_INPUT, buf.data(), &dwSize, sizeof(RAWINPUTHEADER));

		RAWINPUT *ri = reinterpret_cast<RAWINPUT *>(buf.data());

		if (ri->header.dwType != RIM_TYPEMOUSE)
		{
			break;
		}

		Mouse::SetMouseMove(ri->data.mouse.lLastX, ri->data.mouse.lLastY);

		if (ri->data.mouse.usButtonFlags & RI_MOUSE_LEFT_BUTTON_DOWN)
		{
			Mouse::LeftDown();
		}

		if (ri->data.mouse.usButtonFlags & RI_MOUSE_LEFT_BUTTON_UP)
		{
			Mouse::LeftUp();
		}

		if (ri->data.mouse.usButtonFlags & RI_MOUSE_RIGHT_BUTTON_DOWN)
		{
			Mouse::RightDown();
		}

		if (ri->data.mouse.usButtonFlags & RI_MOUSE_RIGHT_BUTTON_UP)
		{
			Mouse::RightUp();
		}

		if (ri->data.mouse.usButtonFlags & RI_MOUSE_MIDDLE_BUTTON_DOWN)
		{
			Mouse::MiddleDown();
		}

		if (ri->data.mouse.usButtonFlags & RI_MOUSE_MIDDLE_BUTTON_UP)
		{
			Mouse::MiddleUp();
		}

	}

	break;

	case WM_MOUSEWHEEL:
	{
		short delta = GET_WHEEL_DELTA_WPARAM(wParam);

		Mouse::Wheel(delta / 120.0f);
	}

	break;

	default:
		break;
	}

	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

// 操作中のゲーム画面だけに、マウスの位置を収める
static void UpdateCursorConfinement(HWND window, bool enabled)
{
	if (!enabled || !IsWindowVisible(window) || IsIconic(window) || GetForegroundWindow() != window)
	{
		ClipCursor(nullptr);

		return;
	}

	RECT client{};

	GetClientRect(window, &client);

	POINT corners[2] = {{client.left, client.top}, {client.right, client.bottom}};

	MapWindowPoints(window, nullptr, corners, 2);

	RECT clip = {corners[0].x, corners[0].y, corners[1].x, corners[1].y};

	if (clip.right > clip.left && clip.bottom > clip.top)
	{
		ClipCursor(&clip);
	}
	else
	{
		ClipCursor(nullptr);
	}
}

// ゲーム画面にWindowsのマウス入力を登録する
static void RegisterRawMouse(HWND hWnd)
{
	// https://learn.microsoft.com/ja-jp/windows/win32/inputdev/using-raw-input

	RAWINPUTDEVICE rid{};

	rid.usUsagePage = 0x01; // マウス、キーボード、ジョイスティック、ゲームパッドの判定取得用
	rid.usUsage = 0x02;		// マウス
	rid.dwFlags = 0;

	rid.hwndTarget = hWnd;

	RegisterRawInputDevices(&rid, 1, sizeof(rid));
}

// 起動場所から画像と保存データのあるフォルダーを探す
static bool SelectAssetDirectory()
{

	wchar_t current[32768]{};

	GetCurrentDirectoryW(32768, current);

	if (HasAssetDirectory(current))
	{
		return true;
	}

	wchar_t executable[32768]{};

	GetModuleFileNameW(nullptr, executable, 32768);

	std::wstring path(executable);

	for (int parent = 0; parent < 5; ++parent)
	{
		const auto separator = path.find_last_of(L"\\/");

		if (separator == std::wstring::npos)
		{
			break;
		}

		path.resize(separator);

		if (HasAssetDirectory(path))
		{
			return SetCurrentDirectoryW(path.c_str()) != FALSE;
		}
	}

	return false;
}

// 指定したフォルダーにassetがあるか確認する
static bool HasAssetDirectory(const std::wstring &path)
{
	DWORD attributes = GetFileAttributesW((path + L"\\asset").c_str());

	return attributes != INVALID_FILE_ATTRIBUTES && (attributes & FILE_ATTRIBUTE_DIRECTORY);
}
