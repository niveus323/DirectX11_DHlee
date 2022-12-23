#include "Window.h"

//static 멤버 변수 초기화.
int Window::width;
int Window::height;
wstring Window::title;
HWND Window::hwnd;
HINSTANCE Window::hInstance;

//메시지 처리
LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_CREATE:
		MessageBox(NULL, TEXT("창이 생성되었습니다!"), TEXT("시작"), MB_OK);	
		return 0;
		//왼쪽 마우스 버튼을 누르면 메시지 상자를 출력.
	case WM_LBUTTONDOWN:
		MessageBox(NULL, TEXT("Hello, World"), TEXT("Hello"), MB_OK);	//2번째 인자가 내용, 3번째 인자가 caption(제목)을 의미한다.
		return 0;
	case WM_KEYDOWN:
		if (wParam == VK_ESCAPE)	//ESC키를 누르면 예/아니오 메시지 상자를 띄운다.
		{
			if(MessageBox(NULL, TEXT("종료하시겠습니까?"), TEXT("종료"), MB_YESNO) == IDYES)
				PostMessage(hWnd, WM_CLOSE, 0, 0);	//WM_CLOSE를 보내도록 하여 응용 프로그램 종료 처리.
		}
		return 0;
	case WM_CLOSE:
		DestroyWindow(hWnd);
		return 0;
	case WM_DESTROY:
		PostQuitMessage(0);	//파괴 메시지를 받았을 경우 종료 메시지를 보내어 메시지 루프를 종료한다.
		return 0;
	}
	return DefWindowProc(hWnd, msg, wParam, lParam);	//명시적으로 처리하지 않는 다른 메시지들은 기본 창 프로시저에게 넘긴다.
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

	//윈도우 클래스 정의.
	WNDCLASS wc;

	//wc의 메모리 0으로 초기화.
	ZeroMemory(&wc, sizeof(wc));

	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = WndProc;	//메시지 처리 함수 지정. (창 프로시저 함수)
	wc.cbClsExtra = 0;	//윈도우 클래스 다음에 할당할 추가 바이트 수.
	wc.cbWndExtra = 0;	//윈도우 인스턴스 다음에 할당할 추가 바이트 수.
	wc.hInstance = hInstance;	//해당 윈도우 클래스를 등록한 응용 프로그램의 Instance Handle.
	wc.hIcon = LoadIcon(nullptr, IDI_APPLICATION); //아이콘 설정. IDI_APPLICATION은 기본 애플리케이션 아이콘을 의미. 
	wc.hCursor = LoadCursor(nullptr, IDC_ARROW);	//커서 설정. IDC_ARROW는 기본 커서를 의미한다. 
	wc.hbrBackground = (HBRUSH)GetStockObject(GRAY_BRUSH);	//배경 설정. GetStockObject를 통해 하얀색 브러쉬를 얻어와 설정함. hbrBackground는 HBRUSH 타입이고 StockObject는 HGDIOBJ 타입이므로 형변환 수행.
	wc.lpszMenuName = 0; //클래스 기본 메뉴. 0은 메뉴가 존재하지 않음을 의미.
	wc.lpszClassName = TEXT("BasicWndClass"); //wide character로 윈도우 클래스의 이름을 정의. TEXT로 멀티바이트 <-> 유니코드 변경을 자동화.

	if (!RegisterClass(&wc))	//윈도우 클래스를 등록.
	{
		MessageBox(0, TEXT("RegisterClass FAILED"), 0, 0);	//윈도으 클래스 등록에 실패했을 경우 "RegisterClass FAILED"라는 메시지를 출력.
		return false;	//그리고 false를 리턴한다.
	}

	//CreateWindow를 통해 설정한 윈도우의 실제 크기는 타이틀바, 메뉴, 테두리에 의하여 더 작게 설정된다.
	//실제 윈도우 크기를 지정한 변수로 만들기 위해서 AdjustWindowRect를 통해 width, height를 실제 윈도우 크기로 하였을 때
	//윈도우 너비와 높이를 어떻게 설정하여야 하는지 알 수 있게 해준다.
	RECT rect = { 0, 0, width, height };
	AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, false);
	Window::width = rect.right - rect.left;
	Window::height = rect.bottom - rect.top;

	//윈도우를 생성하여 메인 윈도우 핸들에 저장.
	hwnd = CreateWindow(
		wc.lpszClassName,	//윈도우 클래스 이름
		title.c_str(),	//윈도우 제목
		WS_OVERLAPPEDWINDOW,	//윈도우 스타일 (OVERLAPPEDWINDOW는 겹치는 창을 의미)
		0,	//생성될 윈도우의 x 위치
		0,	//생성될 윈도우의 y 위치
		Window::width,	//생성될 윈도우의 너비
		Window::height,	//생성될 윈도우의 높이
		NULL,				//생성될 윈도우의 부모에 대한 핸들
		NULL,				//메뉴 핸들 or 자식 윈도우의 ID (팝업 윈도우는 NULL 값 가능, 자식 윈도우는 다른 컨트롤과 중복되지 않는 정수 값)
		hInstance,	//인스턴스 핸들.
		NULL);				//추가적인 생성 플래그. MDI 클라이언트 윈도우 생성 시 CLIENTCREATESTRUCT 구조체 지정 필요.

	if (hwnd == NULL)
	{
		MessageBox(0, TEXT("CreateWindow FAILED"), 0, 0);	//윈도우 생성 실패시 "CreateWindow FAILED"라는 메시지 출력.
		return false;	//그리고 false 반환.
	}

	ShowWindow(hwnd, SW_SHOW);	//생성된 윈도우 화면에 출력.
	UpdateWindow(hwnd);		//윈도우 갱신.

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