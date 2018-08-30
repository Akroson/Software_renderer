#pragma once
#include <windows.h>
#include <stdio.h>
#include "rmath.h"
#include "model.h"

struct offscreen_description
{
	uint32* mainBuff;
	float* zBuff;
	int width;
	int height;
	uint32 sizeBuff;
};

struct windows_description
{
	HWND WindowHandle;
	HDC DeviceContext;
	HBITMAP BitMap;
	offscreen_description Screen;
};

#include "render.h"

bool 
initWindow(windows_description *Window, HINSTANCE Instance);

void 
updateScreen(windows_description *Window);

int 
receiveInput(void *StatePtr, model_description *Model);