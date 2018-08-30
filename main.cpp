#include <windows.h>
#include "rmath.h"
#include "win32.h"
#include "model.h"
#include "render.h"

void
draw(windows_description *Window, model_description *Model)
{
	uint32 *a = Window->Screen.mainBuff;
	uint32 *b = (uint32 *)Model->diffuseMap;

	for (int i = 0; i < (Model->width * Model->height); i++)
	{
		*a++ = *b++;
	}
}

int CALLBACK 
WinMain(HINSTANCE Instance,
		HINSTANCE PrevInstance,
		LPSTR CmdLine,
		int CmdShow)
{

	if (!(*CmdLine))
	{
		return -1;
	}

 	OutputDebugStringA(CmdLine);
	windows_description Window = {};
	model_description Model = {};

	Model.quatRotate = {0, 0, 0, 1.0f};
	Model.pos = {0, 0, 0};

	Window.Screen.width = 1024;
	Window.Screen.height = 1024;

	if (prepareModel(&Model, CmdLine))
	{
		
		if (initWindow(&Window, Instance))
		{
			//draw(&Window, &Model);
			//updateScreen(&Window);

			initRender(&Window, &Model);
		}
		else
		{
			return 2;
		}
	}
	else
	{
		return 1;
	}

	return 0;
}