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
		//Windows �޽��� ó��.
		if (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{
			//�ִϸ��̼Ǥ����� �۾� ����.
			mTimer.Tick();

			if (!mAppPaused)
			{
				CalculateFrameStats();
				UpdateScene(mTimer.DeltaTime());    //DeltaTime�� �Ѱ� �ð��� ���� ������ ����.
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

//�ʱ�ȭ �ڵ�. ex)�ڿ� �Ҵ�, ��� ��ü �ʱ�ȭ, ���� ����
//Window Handle Ȥ�� D3D �������̽��� �ʿ��� Ŭ������ �ʱ�ȭ �Լ����� D3DApp�� Init�� ȣ���ϵ��� �Ѵ�.
bool D3DApp::Init()
{
	if (!InitMainWindow())
		return false;
	if (!InitDirect3D())
		return false;

	return true;
}

//WM_SIZE �޽��� �߻��� MsgProc�� ȣ��.
//Ŭ���̾�Ʈ ũ��� ���õ� �Ӽ��� ����. ex) �ĸ� ����,���̤����ٽ� ����
//�� �ĸ� ���۴� ResizeBuffers�� ���氡��������, ���̤����ٽ� ���۴� ������ؾ��Ѵ�! (���� Ÿ�� ��, ���̤����ٽ� �䵵 �����.)
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

//Window�� �޽��� ó��.
//�ش� Ŭ�������� ó������ �ʰų� ���� �÷α׷��� ������ ������� ó������ �ʴ� �޽����� ���� ó���ϰ� ���� ��쿡�� ������.
LRESULT D3DApp::MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	//���α׷� Ȱ��ȭ����Ȱ��ȭ�� �߻�.
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

	//���� �׵θ��� ���� �� �߻�.
	case WM_ENTERSIZEMOVE:
		mAppPaused = true;
		mResizing = true;
		mTimer.Stop();
		return 0;

	//���� �׵θ��� ������ �߻�.
	case WM_EXITSIZEMOVE:
		mAppPaused = false;
		mResizing = false;
		mTimer.Start();
		OnResize();
		return 0;

	//Ű �Է� ��(Ű�� ������) �߻�
	case WM_KEYDOWN:
		if (wParam == VK_ESCAPE)	//ESCŰ�� ������ ��/�ƴϿ� �޽��� ���ڸ� ����.
		{
			if (MessageBox(NULL, TEXT("�����Ͻðڽ��ϱ�?"), TEXT("����"), MB_YESNO) == IDYES)
				PostMessage(mhMainWnd, WM_CLOSE, 0, 0);	//WM_CLOSE�� �������� �Ͽ� ���� ���α׷� ���� ó��.
		}
		return 0;

	//â �ı� �� �߻�.
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;

	//�Է��� Ű�� �޴� �׸� �������� ���� ��� �߻�.
	case WM_MENUCHAR:
		//�� �Ҹ��� �����ϱ� ���� 0�� �ƴ� ���� �����Ѵ�.
		return MAKELRESULT(0, MNC_CLOSE);

	//������ ũ�� ����� �߻�. ����Ʈ �ּ�/�ִ� ũ�� ����.
	case WM_GETMINMAXINFO:
		((MINMAXINFO*)lParam)->ptMinTrackSize.x = 200;
		((MINMAXINFO*)lParam)->ptMinTrackSize.y = 200;
		return 0;

	//���콺 �Է� ó��
	//windowsx.h�� GET_X_LPARAM, GET_Y_LPARAM ���.
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
	//������ Ŭ���� ����.
	WNDCLASS wc;

	//wc�� �޸� 0���� �ʱ�ȭ.
	ZeroMemory(&wc, sizeof(wc));

	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = MainWndProc;	//�޽��� ó�� �Լ� ����. (â ���ν��� �Լ�)
	wc.cbClsExtra = 0;	//������ Ŭ���� ������ �Ҵ��� �߰� ����Ʈ ��.
	wc.cbWndExtra = 0;	//������ �ν��Ͻ� ������ �Ҵ��� �߰� ����Ʈ ��.
	wc.hInstance = mhAppInst;	//�ش� ������ Ŭ������ ����� ���� ���α׷��� Instance Handle.
	wc.hIcon = LoadIcon(nullptr, IDI_APPLICATION); //������ ����. IDI_APPLICATION�� �⺻ ���ø����̼� �������� �ǹ�. 
	wc.hCursor = LoadCursor(nullptr, IDC_ARROW);	//Ŀ�� ����. IDC_ARROW�� �⺻ Ŀ���� �ǹ��Ѵ�. 
	wc.hbrBackground = (HBRUSH)GetStockObject(NULL_BRUSH);	//��� ����. GetStockObject�� ���� �Ͼ�� �귯���� ���� ������. hbrBackground�� HBRUSH Ÿ���̰� StockObject�� HGDIOBJ Ÿ���̹Ƿ� ����ȯ ����.
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
	RECT rect = { 0, 0, mClientWidth, mClientHeight };
	AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, false);
	int width = rect.right - rect.left;
	int height = rect.bottom - rect.top;

	//�����츦 �����Ͽ� ���� ������ �ڵ鿡 ����.
	mhMainWnd = CreateWindow(
		wc.lpszClassName,	//������ Ŭ���� �̸�
		mMainWndCaption.c_str(),	//������ ����
		WS_OVERLAPPEDWINDOW,	//������ ��Ÿ�� (OVERLAPPEDWINDOW�� ��ġ�� â�� �ǹ�)
		0,	//������ �������� x ��ġ
		0,	//������ �������� y ��ġ
		width,	//������ �������� �ʺ�
		height,	//������ �������� ����
		NULL,				//������ �������� �θ� ���� �ڵ�
		NULL,				//�޴� �ڵ� or �ڽ� �������� ID (�˾� ������� NULL �� ����, �ڽ� ������� �ٸ� ��Ʈ�Ѱ� �ߺ����� �ʴ� ���� ��)
		mhAppInst,	//�ν��Ͻ� �ڵ�.
		NULL);				//�߰����� ���� �÷���. MDI Ŭ���̾�Ʈ ������ ���� �� CLIENTCREATESTRUCT ����ü ���� �ʿ�.

	if (!mhMainWnd)
	{
		MessageBox(0, TEXT("CreateWindow FAILED"), 0, 0);	//������ ���� ���н� "CreateWindow FAILED"��� �޽��� ���.
		return false;	//�׸��� false ��ȯ.
	}

	ShowWindow(mhMainWnd, SW_SHOW);	//������ ������ ȭ�鿡 ���.
	UpdateWindow(mhMainWnd);		//������ ����.

	return true;
}

bool D3DApp::InitDirect3D()
{
	//����̽� �ʱ�ȭ
	UINT createDeviceFlags = 0;
#if defined(DEBUG) || defined(_DEBUG)
	createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;	//D3D11_CREATE_DEVICE_DEBUG�� OR������ ����� createDeviceFlags�� ����
#endif // defined(DEBUG) || defined(_DEBUG)

	D3D_FEATURE_LEVEL featureLevel;
	//D3D11 ��ġ ����.
	HRESULT hr = D3D11CreateDevice(0,	/* �⺻ ���÷��� ����� ���.*/
		md3dDriverType, /* ����̹� ����(�����ڿ� ���� D3D_DRIVER_TYPE_HARDWARE ���� - �ϵ����� D3D ����� ����. D3D_DRIVER_TYPE_REFERENCE - ) */
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

	//SWAP chain�� �����ϱ� ���� IDXGIFactory
	IDXGIDevice* dxgiDevice = 0;
	HR(md3dDevice->QueryInterface(__uuidof(IDXGIDevice), (void**)&dxgiDevice));

	IDXGIAdapter* dxgiAdaptor = 0;
	HR(dxgiDevice->GetParent(__uuidof(IDXGIAdapter), (void**)&dxgiAdaptor));

	IDXGIFactory* dxgiFactory = 0;
	HR(dxgiAdaptor->GetParent(__uuidof(IDXGIFactory), (void**)&dxgiFactory));

	HR(dxgiFactory->CreateSwapChain(md3dDevice, &sd, &mSwapChain));

	//MakeWindowAssociation�� ȭ����ȯ����� ������ �� �ְ� ���ִ� �Լ�. DXGI_MWA�� �����ϴ� �÷��׸� �����Ѵ�.
	//DXGI_MWA_NO_WINDOW_CHAGNGES : ���ø����̼� �޽��� ť�� �������� �ʰ� �ϴ� �÷���.
	HR(dxgiFactory->MakeWindowAssociation(mhMainWnd, DXGI_MWA_NO_WINDOW_CHANGES));
	//DXGI_MWA_NO_ALT_ENTER : ALT-ENTER�� �Է¿� �������� �ʰ� �Ͽ� ��üȭ��-â��� ��ȯ�� ������� �ʰ� �ϴ� �÷���. 
	//HR(dxgiFactory->MakeWindowAssociation(mhMainWnd, DXGI_MWA_NO_ALT_ENTER));
	//DXGI_MWA_NO_PRINT_SCREEN : Print Screen Ű�� �������� �ʰ� �ϴ� �÷���.
	//HR(dxgiFactory->MakeWindowAssociation(mhMainWnd, DXGI_MWA_NO_PRINT_SCREEN));

	ReleaseCOM(dxgiDevice);
	ReleaseCOM(dxgiAdaptor);
	ReleaseCOM(dxgiFactory);

	OnResize();

	return true;
}

/* ��� FPS ���.
* �� FPS�� ��� ���濡 ���� �ð� ����/���Ҹ� ���������� �ʴ´�.
ex1) 1000FPS�� ����Ǵ� ���α׷��� ��� ����� 250FPS�� �������� ��� 3ms�� ����.
ex2) 100FPS�� ����Ǵ� ���α׷��� ��� ����� 76.9FPS�� �������� ��� 3ms�� ����.(������ �ð� : 10ms -> 13ms)
�������� �ɸ� �ð��� ���� ���� ������ �ð��� �������̴�.
*/
void D3DApp::CalculateFrameStats()
{
	static int frameCnt = 0;
	static float timeElapsed = 0.0f;
	++frameCnt;

	//1�� ������ ������ �� ���.
	if ((mTimer.GameTime() - timeElapsed) >= 1.0f)
	{
		float fps = (float)frameCnt;
		float mspf = 1000.0f / fps;    //������ �ð� ���. (�и��� ����)
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

