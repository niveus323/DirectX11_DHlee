#pragma once
#include<Windows.h>
#include<string>
using std::wstring;

class Window
{
public:
	static bool InitializeWindow();
	static bool InitializeWindow(HINSTANCE hInstance, int width, int height, wstring title);

	static HWND getWindowHandle() { return hwnd; }

	static HINSTANCE getInstance() { return hInstance; }
	static void SetInstance(HINSTANCE hInstance);

	static int getWidth() { return width; }
	static void SetWidth(int width);

	static int getHeight() { return height; }
	static void SetHeight(int height);

	static wstring getTitle() { return title; }
	static void SetTitle(wstring title);
private:
	static int width;
	static int height;
	static wstring title;
	static HWND hwnd;
	static HINSTANCE hInstance;
};

