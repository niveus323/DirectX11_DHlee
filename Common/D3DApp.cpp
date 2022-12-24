#include "d3dApp.h"
#include <windowsx.h>
#include <iostream>

namespace
{
	D3DApp* gd3dApp = 0;
}

LRESULT CALLBACK
MainWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	return gd3dApp->MsgProc(hwnd, msg, wParam, lParam);
}


D3DApp::D3DApp(HINSTANCE hInstance) : mhAppInst(hInstance), mMainWndCaption(TEXT("D3D11 Application")), md3dDriverType(D3D_DRIVER_TYPE_HARDWARE), mClientWidth(800), mClientHeight(600), mEnable4xMsaa(false), mhMainWnd(0), mAppPaused(false), mMinimized(false), mMaximized(false), mResizing(false), m4xMsaaQuality(0), md3dDevice(0), md3dImmediateContext(0), mSwapChain(0), mDepthStencilBuffer(0), mRenderTargetView(0), mDepthStencilView(0)
{
	ZeroMemory(&mScreenViewport, sizeof(D3D11_VIEWPORT));

	gd3dApp = this;
}

D3DApp::~D3DApp()
{
	ReleaseCOM(mRenderTargetView);
	ReleaseCOM(mDepthStencilView);
	ReleaseCOM(mSwapChain);
	ReleaseCOM(mDepthStencilBuffer);

	if (md3dImmediateContext)
		md3dImmediateContext->ClearState();

	ReleaseCOM(md3dImmediateContext);
	ReleaseCOM(md3dDevice);
}

HINSTANCE D3DApp::AppInst() const
{
	return mhAppInst;
}

HWND D3DApp::MainWnd() const
{
	return mhMainWnd;
}

float D3DApp::AspectRatio() const
{
	return static_cast<float>(mClientWidth) / mClientHeight;
}

int D3DApp::Run()
{
	MSG msg = { 0 };

	mTimer.Reset();

	while (msg.message != WM_QUIT)
	{
		//Windows 메시지 처리.
		if (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{
			//애니메이션ㆍ게임 작업 수행.
			mTimer.Tick();

			if (!mAppPaused)
			{
				CalculateFrameStats();
				UpdateScene(mTimer.DeltaTime());    //DeltaTime을 넘겨 시간에 따른 갱신을 수행.
				DrawScene();
			}
			else
			{
				Sleep(100);
			}
		}
	}
	return (int)msg.wParam;
}

//초기화 코드. ex)자원 할당, 장면 물체 초기화, 광원 설정
//Window Handle 혹은 D3D 인터페이스가 필요한 클래스는 초기화 함수에서 D3DApp의 Init을 호출하도록 한다.
bool D3DApp::Init()
{
	if (!InitMainWindow())
		return false;
	if (!InitDirect3D())
		return false;

	return true;
}

//WM_SIZE 메시지 발생시 MsgProc가 호출.
//클라이언트 크기와 관련된 속성을 갱신. ex) 후면 버퍼,깊이ㆍ스텐실 버퍼
//※ 후면 버퍼는 ResizeBuffers로 변경가능하지만, 깊이ㆍ스텐실 버퍼는 재생성해야한다! (렌더 타겟 뷰, 깊이ㆍ스텐실 뷰도 재생성.)
void D3DApp::OnResize()
{
	assert(md3dImmediateContext);
	assert(md3dDevice);
	assert(mSwapChain);

	ReleaseCOM(mRenderTargetView);
	ReleaseCOM(mDepthStencilView);
	ReleaseCOM(mDepthStencilBuffer);

	HR(mSwapChain->ResizeBuffers(1, mClientWidth, mClientHeight, DXGI_FORMAT_R8G8B8A8_UNORM, 0));
	ID3D11Texture2D* backBuffer = nullptr;
	HR(mSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&backBuffer)));
	assert(backBuffer);
	HR(md3dDevice->CreateRenderTargetView(backBuffer, 0, &mRenderTargetView));
	ReleaseCOM(backBuffer);

	D3D11_TEXTURE2D_DESC depthStencilDesc{};

	depthStencilDesc.Width = mClientWidth;
	depthStencilDesc.Height = mClientHeight;
	depthStencilDesc.MipLevels = 1;
	depthStencilDesc.ArraySize = 1;
	depthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;

	if (mEnable4xMsaa)
	{
		depthStencilDesc.SampleDesc.Count = 4;
		depthStencilDesc.SampleDesc.Quality = m4xMsaaQuality - 1;
	}
	else
	{
		depthStencilDesc.SampleDesc.Count = 1;
		depthStencilDesc.SampleDesc.Quality = 0;
	}

	depthStencilDesc.Usage = D3D11_USAGE_DEFAULT;
	depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depthStencilDesc.CPUAccessFlags = 0;
	depthStencilDesc.MiscFlags = 0;

	HR(md3dDevice->CreateTexture2D(&depthStencilDesc, 0, &mDepthStencilBuffer));
	assert(mDepthStencilBuffer);
	HR(md3dDevice->CreateDepthStencilView(mDepthStencilBuffer, 0, &mDepthStencilView));

	md3dImmediateContext->OMSetRenderTargets(1, &mRenderTargetView, mDepthStencilView);

	mScreenViewport.TopLeftX = 0;
	mScreenViewport.TopLeftY = 0;
	mScreenViewport.Width = static_cast<float>(mClientWidth);
	mScreenViewport.Height = static_cast<float>(mClientHeight);
	mScreenViewport.MinDepth = 0.0f;
	mScreenViewport.MaxDepth = 1.0f;

	md3dImmediateContext->RSSetViewports(1, &mScreenViewport);
}

//Window의 메시지 처리.
//해당 클래스에서 처리하지 않거나 응용 플로그램에 적합한 방식으로 처리하지 않는 메시지를 직접 처리하고 싶은 경우에만 재정의.
LRESULT D3DApp::MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	//프로그램 활성화ㆍ비활성화시 발생.
	case WM_ACTIVATE:
		if (LOWORD(wParam) == WA_INACTIVE)
		{
			mAppPaused = true;
			mTimer.Stop();
		}
		else
		{
			mAppPaused = false;
			mTimer.Start();
		}
		return 0;

	case WM_SIZE:
		mClientHeight = LOWORD(lParam);
		mClientHeight = HIWORD(lParam);
		if (md3dDevice)
		{
			if (wParam == SIZE_MINIMIZED)
			{
				mAppPaused = true;
				mMinimized = true;
				mMaximized = false;
			}
			else if (wParam == SIZE_MAXIMIZED)
			{
				mAppPaused = false;
				mMinimized = false;
				mMaximized = true;
			}
			else if (wParam == SIZE_RESTORED)
			{
				if (mMinimized)
				{
					mAppPaused = false;
					mMinimized = false;
					OnResize();
				}
				else if (mMaximized)
				{
					mAppPaused = false;
					mMaximized = false;
					OnResize();
				}
				else if (mResizing)
				{

				}
				else
				{
					OnResize();
				}

			}
		}
		return 0;

	//변경 테두리를 잡을 시 발생.
	case WM_ENTERSIZEMOVE:
		mAppPaused = true;
		mResizing = true;
		mTimer.Stop();
		return 0;

	//변경 테두리를 놓으면 발생.
	case WM_EXITSIZEMOVE:
		mAppPaused = false;
		mResizing = false;
		mTimer.Start();
		OnResize();
		return 0;

	//키 입력 시(키를 누를시) 발생
	case WM_KEYDOWN:
		if (wParam == VK_ESCAPE)	//ESC키를 누르면 예/아니오 메시지 상자를 띄운다.
		{
			if (MessageBox(NULL, TEXT("종료하시겠습니까?"), TEXT("종료"), MB_YESNO) == IDYES)
				PostMessage(mhMainWnd, WM_CLOSE, 0, 0);	//WM_CLOSE를 보내도록 하여 응용 프로그램 종료 처리.
		}
		return 0;

	//창 파괴 시 발생.
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;

	//입력한 키가 메뉴 항목에 대응하지 않을 경우 발생.
	case WM_MENUCHAR:
		//삐 소리를 방지하기 위해 0이 아닌 값을 리턴한다.
		return MAKELRESULT(0, MNC_CLOSE);

	//윈도우 크기 변경시 발생. 디폴트 최소/최대 크기 설정.
	case WM_GETMINMAXINFO:
		((MINMAXINFO*)lParam)->ptMinTrackSize.x = 200;
		((MINMAXINFO*)lParam)->ptMinTrackSize.y = 200;
		return 0;

	//마우스 입력 처리
	//windowsx.h의 GET_X_LPARAM, GET_Y_LPARAM 사용.
	case WM_LBUTTONDOWN:
	case WM_MBUTTONDOWN:
	case WM_RBUTTONDOWN:
		OnMouseDown(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		return 0;

	case WM_LBUTTONUP:
	case WM_MBUTTONUP:
	case WM_RBUTTONUP:
		OnMouseUp(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		return 0;

	case WM_MOUSEMOVE:
		OnMouseMove(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		return 0;
	}

	return DefWindowProc(hwnd, msg, wParam, lParam);
}

bool D3DApp::InitMainWindow()
{
	//윈도우 클래스 정의.
	WNDCLASS wc;

	//wc의 메모리 0으로 초기화.
	ZeroMemory(&wc, sizeof(wc));

	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = MainWndProc;	//메시지 처리 함수 지정. (창 프로시저 함수)
	wc.cbClsExtra = 0;	//윈도우 클래스 다음에 할당할 추가 바이트 수.
	wc.cbWndExtra = 0;	//윈도우 인스턴스 다음에 할당할 추가 바이트 수.
	wc.hInstance = mhAppInst;	//해당 윈도우 클래스를 등록한 응용 프로그램의 Instance Handle.
	wc.hIcon = LoadIcon(nullptr, IDI_APPLICATION); //아이콘 설정. IDI_APPLICATION은 기본 애플리케이션 아이콘을 의미. 
	wc.hCursor = LoadCursor(nullptr, IDC_ARROW);	//커서 설정. IDC_ARROW는 기본 커서를 의미한다. 
	wc.hbrBackground = (HBRUSH)GetStockObject(NULL_BRUSH);	//배경 설정. GetStockObject를 통해 하얀색 브러쉬를 얻어와 설정함. hbrBackground는 HBRUSH 타입이고 StockObject는 HGDIOBJ 타입이므로 형변환 수행.
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
	RECT rect = { 0, 0, mClientWidth, mClientHeight };
	AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, false);
	int width = rect.right - rect.left;
	int height = rect.bottom - rect.top;

	//윈도우를 생성하여 메인 윈도우 핸들에 저장.
	mhMainWnd = CreateWindow(
		wc.lpszClassName,	//윈도우 클래스 이름
		mMainWndCaption.c_str(),	//윈도우 제목
		WS_OVERLAPPEDWINDOW,	//윈도우 스타일 (OVERLAPPEDWINDOW는 겹치는 창을 의미)
		0,	//생성될 윈도우의 x 위치
		0,	//생성될 윈도우의 y 위치
		width,	//생성될 윈도우의 너비
		height,	//생성될 윈도우의 높이
		NULL,				//생성될 윈도우의 부모에 대한 핸들
		NULL,				//메뉴 핸들 or 자식 윈도우의 ID (팝업 윈도우는 NULL 값 가능, 자식 윈도우는 다른 컨트롤과 중복되지 않는 정수 값)
		mhAppInst,	//인스턴스 핸들.
		NULL);				//추가적인 생성 플래그. MDI 클라이언트 윈도우 생성 시 CLIENTCREATESTRUCT 구조체 지정 필요.

	if (!mhMainWnd)
	{
		MessageBox(0, TEXT("CreateWindow FAILED"), 0, 0);	//윈도우 생성 실패시 "CreateWindow FAILED"라는 메시지 출력.
		return false;	//그리고 false 반환.
	}

	ShowWindow(mhMainWnd, SW_SHOW);	//생성된 윈도우 화면에 출력.
	UpdateWindow(mhMainWnd);		//윈도우 갱신.

	return true;
}

bool D3DApp::InitDirect3D()
{
	//디바이스 초기화
	UINT createDeviceFlags = 0;
#if defined(DEBUG) || defined(_DEBUG)
	createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;	//D3D11_CREATE_DEVICE_DEBUG와 OR연산한 결과를 createDeviceFlags에 저장
#endif // defined(DEBUG) || defined(_DEBUG)

	D3D_FEATURE_LEVEL featureLevel;
	//D3D11 장치 생성.
	HRESULT hr = D3D11CreateDevice(0,	/* 기본 디스플레이 어댑터 사용.*/
		md3dDriverType, /* 드라이버 형식(생성자에 의해 D3D_DRIVER_TYPE_HARDWARE 지정 - 하드웨어에서 D3D 기능을 구현. D3D_DRIVER_TYPE_REFERENCE - ) */
		0,
		createDeviceFlags,
		0,
		0,
		D3D11_SDK_VERSION,
		&md3dDevice,
		&featureLevel,
		&md3dImmediateContext);

	if (FAILED(hr))
	{
		MessageBox(0, TEXT("D3D11CreateDevice Failed."), 0, 0);
		return false;
	}

	if (featureLevel != D3D_FEATURE_LEVEL_11_0)
	{
		MessageBox(0, TEXT("Direct3D Feature Level 11 unsupported."), 0, 0);
		return false;
	}

	HR(md3dDevice->CheckMultisampleQualityLevels(DXGI_FORMAT_R8G8B8A8_UNORM, 4, &m4xMsaaQuality));
	assert(m4xMsaaQuality > 0);

	DXGI_SWAP_CHAIN_DESC sd{};
	sd.BufferDesc.Width = mClientWidth;
	sd.BufferDesc.Height = mClientHeight;
	sd.BufferDesc.RefreshRate.Numerator = 60;
	sd.BufferDesc.RefreshRate.Denominator = 1;
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	sd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	sd.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

	if (mEnable4xMsaa)
	{
		sd.SampleDesc.Count = 4;
		sd.SampleDesc.Quality = m4xMsaaQuality - 1;
	}
	else
	{
		sd.SampleDesc.Count = 1;
		sd.SampleDesc.Quality = 0;
	}

	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.BufferCount = 1;
	sd.OutputWindow = mhMainWnd;
	sd.Windowed = true;
	sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	sd.Flags = 0;

	//SWAP chain을 생성하기 위한 IDXGIFactory
	IDXGIDevice* dxgiDevice = 0;
	HR(md3dDevice->QueryInterface(__uuidof(IDXGIDevice), (void**)&dxgiDevice));

	IDXGIAdapter* dxgiAdaptor = 0;
	HR(dxgiDevice->GetParent(__uuidof(IDXGIAdapter), (void**)&dxgiAdaptor));

	IDXGIFactory* dxgiFactory = 0;
	HR(dxgiAdaptor->GetParent(__uuidof(IDXGIFactory), (void**)&dxgiFactory));

	HR(dxgiFactory->CreateSwapChain(md3dDevice, &sd, &mSwapChain));

	//MakeWindowAssociation은 화면전환기능을 설정할 수 있게 해주는 함수. DXGI_MWA로 시작하는 플래그를 지정한다.
	//DXGI_MWA_NO_WINDOW_CHAGNGES : 애플리케이션 메시지 큐를 감시하지 않게 하는 플래그.
	HR(dxgiFactory->MakeWindowAssociation(mhMainWnd, DXGI_MWA_NO_WINDOW_CHANGES));
	//DXGI_MWA_NO_ALT_ENTER : ALT-ENTER의 입력에 응답하지 않게 하여 전체화면-창모드 전환을 허용하지 않게 하는 플래그. 
	//HR(dxgiFactory->MakeWindowAssociation(mhMainWnd, DXGI_MWA_NO_ALT_ENTER));
	//DXGI_MWA_NO_PRINT_SCREEN : Print Screen 키에 응답하지 않게 하는 플래그.
	//HR(dxgiFactory->MakeWindowAssociation(mhMainWnd, DXGI_MWA_NO_PRINT_SCREEN));

	ReleaseCOM(dxgiDevice);
	ReleaseCOM(dxgiAdaptor);
	ReleaseCOM(dxgiFactory);

	OnResize();

	return true;
}

/* 평균 FPS 계산.
* ※ FPS는 장면 변경에 따른 시간 증가/감소를 보여주지는 않는다.
ex1) 1000FPS로 실행되는 프로그램이 장면 변경시 250FPS로 떨어졌을 경우 3ms의 차이.
ex2) 100FPS로 실행되는 프로그램이 장면 변경시 76.9FPS로 떨어졌을 경우 3ms의 차이.(프레임 시간 : 10ms -> 13ms)
렌더링에 걸린 시간을 보는 것은 프레임 시간이 직관적이다.
*/
void D3DApp::CalculateFrameStats()
{
	static int frameCnt = 0;
	static float timeElapsed = 0.0f;
	++frameCnt;

	//1초 동안의 프레임 수 계산.
	if ((mTimer.GameTime() - timeElapsed) >= 1.0f)
	{
		float fps = (float)frameCnt;
		float mspf = 1000.0f / fps;    //프레임 시간 계산. (밀리초 단위)
		std::wostringstream outs;
		outs.precision(6);
		outs << mMainWndCaption << TEXT("    ")
			<< TEXT("FPS: ") << fps << TEXT("    ")
			<< TEXT("Frame Time: ") << mspf << TEXT(" (ms)");
		SetWindowText(mhMainWnd, outs.str().c_str());

		frameCnt = 0;
		timeElapsed += 1.0f;
	}
}

