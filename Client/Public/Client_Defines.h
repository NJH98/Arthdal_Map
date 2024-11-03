#pragma once

#include <process.h>

#include "..\imgui\imgui.h"
#include "..\imgui\imgui_impl_dx11.h"
#include "..\imgui\imgui_impl_win32.h"
#include "..\imgui\ImGuiFileDialog.h"

namespace Client
{
	const unsigned int			g_iWinSizeX = { 1280 };
	const unsigned int			g_iWinSizeY = { 720 };

	enum LEVELID { LEVEL_STATIC, LEVEL_LOADING, LEVEL_LOGO, LEVEL_GAMEPLAY, LEVEL_END };
}

extern HINSTANCE g_hInst;
extern HWND g_hWnd;

using namespace Client;
using namespace std;
