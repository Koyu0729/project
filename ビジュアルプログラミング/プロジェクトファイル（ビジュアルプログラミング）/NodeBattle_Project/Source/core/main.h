#pragma once

#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>

#define NOMINMAX
#include <windows.h>
#include <assert.h>
#include <functional>

#include <d3d11.h>
#pragma comment (lib, "d3d11.lib")

#include <DirectXMath.h>
using namespace DirectX;

#include "DirectXTex.h"
#include "vector3.h"
#include "vector2.h"

#if _DEBUG
#pragma comment(lib, "DirectXTex_Debug.lib")
#else
#pragma comment(lib, "DirectXTex_Release.lib")
#endif

#pragma comment (lib, "winmm.lib")

#define SCREEN_WIDTH	(1280)
#define SCREEN_HEIGHT	(720)

HWND GetGameWindow();
float GetFrameDeltaSeconds();

// Windowsから届くウィンドウの通知を処理する
LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
