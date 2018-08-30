#include "win32.h"

static LRESULT CALLBACK
MainCallback(HWND Window,
	UINT Message,
	WPARAM wParam,
	LPARAM lParam)
{
	switch (Message)
	{
		case WM_CLOSE:
			DestroyWindow(Window);
			break;
		case WM_DESTROY:
			PostQuitMessage(0);
			break;
		default:
			return DefWindowProc(Window, Message, wParam, lParam);
	}
	return 0;
}

static inline void
filZbuff(windows_description *Window)
{
#define MIN_FLOAT -3.402823466e+38F
#define MAX_FLOAT 3.402823466e+38F

	float fillFloat = MIN_FLOAT;

	for (int i = 0; i < Window->Screen.sizeBuff; i++)
	{
		Window->Screen.zBuff[i] = fillFloat;
	}
}

static bool 
initBitMap(windows_description *Window)
{
	BITMAPINFO BitMapInfo;

	BitMapInfo.bmiHeader = {};
	BitMapInfo.bmiHeader.biSize = sizeof(BitMapInfo.bmiHeader);
	BitMapInfo.bmiHeader.biWidth = Window->Screen.width;
	BitMapInfo.bmiHeader.biHeight = -Window->Screen.height;
	BitMapInfo.bmiHeader.biPlanes = 1;
	BitMapInfo.bmiHeader.biBitCount = 32;
	BitMapInfo.bmiHeader.biCompression = BI_RGB;

	Window->Screen.sizeBuff = Window->Screen.width * Window->Screen.height;
	Window->Screen.mainBuff = (uint32 *)VirtualAlloc(
		0, (Window->Screen.sizeBuff * 4),
		MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);

	if (!Window->Screen.mainBuff) return false;

	Window->Screen.zBuff = (float *)VirtualAlloc(
		0, (Window->Screen.sizeBuff * 4),
		MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);

	if (!Window->Screen.zBuff) return false;

	filZbuff(Window);

	Window->DeviceContext = GetDC(Window->WindowHandle);

	Window->BitMap = CreateDIBSection(
		Window->DeviceContext, &BitMapInfo,
		DIB_RGB_COLORS, (void **)&Window->Screen.mainBuff,
		0, 0);

	return true;
}

bool
initWindow(windows_description *Window, HINSTANCE Instance)
{
	WNDCLASSA WindowClass = {};

	WindowClass.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	WindowClass.lpfnWndProc = MainCallback;
	WindowClass.hInstance = Instance;
	WindowClass.lpszClassName = "Akros_Render";

	if (RegisterClassA(&WindowClass))
	{
		Window->WindowHandle = CreateWindowExA(
			0,
			WindowClass.lpszClassName,
			"Render",
			WS_OVERLAPPEDWINDOW | WS_VISIBLE,
			CW_USEDEFAULT,
			CW_USEDEFAULT,
			Window->Screen.width,
			Window->Screen.height,
			0,
			0,
			Instance,
			0);

		if (Window->WindowHandle)
		{
			if (!initBitMap(Window)) return false;
			else return true;
		}
		else
		{
			return false;
		}
	}
	else
	{
		return false;
	}
}

void
updateScreen(windows_description *Window)
{
	HDC mdc;

	mdc = CreateCompatibleDC(Window->DeviceContext);
	SelectObject(mdc, Window->BitMap);

	BitBlt(
		Window->DeviceContext, 0, 0,
		Window->Screen.width, Window->Screen.height, 
		mdc, 0, 0, SRCCOPY);

#if 1
	SecureZeroMemory(Window->Screen.mainBuff, Window->Screen.sizeBuff * 4);
	filZbuff(Window);
#endif
	DeleteDC(mdc);
}

void
receiveInput(void *StatePtr, model_description *Model)
{
	MSG Message;

	while (PeekMessage(&Message, 0, 0, 0, PM_REMOVE))
	{
		render_state *State = (render_state *)StatePtr;

		if (Message.message == WM_QUIT)
		{
			State->running = false;
		}

		TranslateMessage(&Message);
		DispatchMessageA(&Message);

		switch (Message.message)
		{
			case WM_MOUSEMOVE:
			{
				if (State->LButtonPress | State->RButtonPress)
				{
					processMouse(State, Model, LOWORD(Message.lParam), HIWORD(Message.lParam));
				}
			} break;
			case WM_RBUTTONDOWN:
			{
				if (!State->LButtonPress)
				{
					State->RButtonPress = true;
					State->mouseX = LOWORD(Message.lParam);
					State->mouseY = HIWORD(Message.lParam);
				}
			} break;
			case WM_LBUTTONDOWN:
			{
				if (!State->RButtonPress)
				{
					State->LButtonPress = true;
					State->mouseX = LOWORD(Message.lParam);
					State->mouseY = HIWORD(Message.lParam);
				}
			} break;
			case WM_RBUTTONUP:
			{
				State->RButtonPress = false;
			} break;
			case WM_LBUTTONUP:
			{
				State->LButtonPress = false;
			} break;
			case WM_MOUSEWHEEL:
			{
				bool dir = (int16)HIWORD(Message.wParam) & (1 << 16);
				processMouseWeel(State, Model, dir);
			} break;
			case WM_KEYUP:
			case WM_KEYDOWN:
			{
				bool press = Message.lParam & (1 << 31);
				processKeyboard(State, Model, (uint32)Message.wParam, press);
			} break;
		}
	}
}
