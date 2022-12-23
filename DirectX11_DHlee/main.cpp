#include "Window.h"

int Run();

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR pCmdLine, int nCmdShow)
{
	if (!Window::InitializeWindow(hInstance, 1280, 800, TEXT("Graphics_DHlee")))
		return 0;

	return Run();
}

int Run()
{
	MSG msg = { 0 };
	ZeroMemory(&msg, sizeof(msg));

	while (msg.message != WM_QUIT)
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{
			//.....
		}
	}
	return (int)msg.wParam;
}