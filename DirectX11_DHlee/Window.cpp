#include "Window.h"

//static ��� ���� �ʱ�ȭ.
int Window::width;
int Window::height;
wstring Window::title;
HWND Window::hwnd;
HINSTANCE Window::hInstance;

//�޽��� ó��
LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_CREATE:
		MessageBox(NULL, TEXT("â�� �����Ǿ����ϴ�!"), TEXT("����"), MB_OK);	
		return 0;
		//���� ���콺 ��ư�� ������ �޽��� ���ڸ� ���.
	case WM_LBUTTONDOWN:
		MessageBox(NULL, TEXT("Hello, World"), TEXT("Hello"), MB_OK);	//2��° ���ڰ� ����, 3��° ���ڰ� caption(����)�� �ǹ��Ѵ�.
		return 0;
	case WM_KEYDOWN:
		if (wParam == VK_ESCAPE)	//ESCŰ�� ������ ��/�ƴϿ� �޽��� ���ڸ� ����.
		{
			if(MessageBox(NULL, TEXT("�����Ͻðڽ��ϱ�?"), TEXT("����"), MB_YESNO) == IDYES)
				PostMessage(hWnd, WM_CLOSE, 0, 0);	//WM_CLOSE�� �������� �Ͽ� ���� ���α׷� ���� ó��.
		}
		return 0;
	case WM_CLOSE:
		DestroyWindow(hWnd);
		return 0;
	case WM_DESTROY:
		PostQuitMessage(0);	//�ı� �޽����� �޾��� ��� ���� �޽����� ������ �޽��� ������ �����Ѵ�.
		return 0;
	}
	return DefWindowProc(hWnd, msg, wParam, lParam);	//��������� ó������ �ʴ� �ٸ� �޽������� �⺻ â ���ν������� �ѱ��.
}

bool Window::InitializeWindow()
{
	return InitializeWindow(hInstance, width, height, title);
}

bool Window::InitializeWindow(HINSTANCE hInstance, int width, int height, wstring title)
{
	Window::hInstance = hInstance;
	Window::width = width;
	Window::height = height;
	Window::title = title;

	//������ Ŭ���� ����.
	WNDCLASS wc;

	//wc�� �޸� 0���� �ʱ�ȭ.
	ZeroMemory(&wc, sizeof(wc));

	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = WndProc;	//�޽��� ó�� �Լ� ����. (â ���ν��� �Լ�)
	wc.cbClsExtra = 0;	//������ Ŭ���� ������ �Ҵ��� �߰� ����Ʈ ��.
	wc.cbWndExtra = 0;	//������ �ν��Ͻ� ������ �Ҵ��� �߰� ����Ʈ ��.
	wc.hInstance = hInstance;	//�ش� ������ Ŭ������ ����� ���� ���α׷��� Instance Handle.
	wc.hIcon = LoadIcon(nullptr, IDI_APPLICATION); //������ ����. IDI_APPLICATION�� �⺻ ���ø����̼� �������� �ǹ�. 
	wc.hCursor = LoadCursor(nullptr, IDC_ARROW);	//Ŀ�� ����. IDC_ARROW�� �⺻ Ŀ���� �ǹ��Ѵ�. 
	wc.hbrBackground = (HBRUSH)GetStockObject(GRAY_BRUSH);	//��� ����. GetStockObject�� ���� �Ͼ�� �귯���� ���� ������. hbrBackground�� HBRUSH Ÿ���̰� StockObject�� HGDIOBJ Ÿ���̹Ƿ� ����ȯ ����.
	wc.lpszMenuName = 0; //Ŭ���� �⺻ �޴�. 0�� �޴��� �������� ������ �ǹ�.
	wc.lpszClassName = TEXT("BasicWndClass"); //wide character�� ������ Ŭ������ �̸��� ����. TEXT�� ��Ƽ����Ʈ <-> �����ڵ� ������ �ڵ�ȭ.

	if (!RegisterClass(&wc))	//������ Ŭ������ ���.
	{
		MessageBox(0, TEXT("RegisterClass FAILED"), 0, 0);	//������ Ŭ���� ��Ͽ� �������� ��� "RegisterClass FAILED"��� �޽����� ���.
		return false;	//�׸��� false�� �����Ѵ�.
	}

	//CreateWindow�� ���� ������ �������� ���� ũ��� Ÿ��Ʋ��, �޴�, �׵θ��� ���Ͽ� �� �۰� �����ȴ�.
	//���� ������ ũ�⸦ ������ ������ ����� ���ؼ� AdjustWindowRect�� ���� width, height�� ���� ������ ũ��� �Ͽ��� ��
	//������ �ʺ�� ���̸� ��� �����Ͽ��� �ϴ��� �� �� �ְ� ���ش�.
	RECT rect = { 0, 0, width, height };
	AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, false);
	Window::width = rect.right - rect.left;
	Window::height = rect.bottom - rect.top;

	//�����츦 �����Ͽ� ���� ������ �ڵ鿡 ����.
	hwnd = CreateWindow(
		wc.lpszClassName,	//������ Ŭ���� �̸�
		title.c_str(),	//������ ����
		WS_OVERLAPPEDWINDOW,	//������ ��Ÿ�� (OVERLAPPEDWINDOW�� ��ġ�� â�� �ǹ�)
		0,	//������ �������� x ��ġ
		0,	//������ �������� y ��ġ
		Window::width,	//������ �������� �ʺ�
		Window::height,	//������ �������� ����
		NULL,				//������ �������� �θ� ���� �ڵ�
		NULL,				//�޴� �ڵ� or �ڽ� �������� ID (�˾� ������� NULL �� ����, �ڽ� ������� �ٸ� ��Ʈ�Ѱ� �ߺ����� �ʴ� ���� ��)
		hInstance,	//�ν��Ͻ� �ڵ�.
		NULL);				//�߰����� ���� �÷���. MDI Ŭ���̾�Ʈ ������ ���� �� CLIENTCREATESTRUCT ����ü ���� �ʿ�.

	if (hwnd == NULL)
	{
		MessageBox(0, TEXT("CreateWindow FAILED"), 0, 0);	//������ ���� ���н� "CreateWindow FAILED"��� �޽��� ���.
		return false;	//�׸��� false ��ȯ.
	}

	ShowWindow(hwnd, SW_SHOW);	//������ ������ ȭ�鿡 ���.
	UpdateWindow(hwnd);		//������ ����.

	return true;
}

void Window::SetInstance(HINSTANCE hInstance)
{
	Window::hInstance = hInstance;
}

void Window::SetWidth(int width)
{
	Window::width = width;
}

void Window::SetHeight(int height)
{
	Window::height = height;
}

void Window::SetTitle(wstring title)
{
	Window::title = title;
}