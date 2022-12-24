#include "D3DApp.h"

class InitDirect3DApp : public D3DApp
{
public:
	InitDirect3DApp(HINSTANCE hInstance);
	~InitDirect3DApp();

	bool Init();
	void OnResize();
	void UpdateScene(float dt);
	void DrawScene();
private:
	bool InitDirect3D();
};

#pragma warning(push)
#pragma warning(disable : 28251)
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE prevInstance, PSTR cmdLine, int showCmd)
{
#if defined(DEBUG) | defined(_DEBUG)
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif // DEBUG | _DEBUG

	InitDirect3DApp theApp(hInstance);

	if (!theApp.Init())
		return 0;

	return theApp.Run();
}
#pragma warning(pop)

InitDirect3DApp::InitDirect3DApp(HINSTANCE hInstance) : D3DApp(hInstance) {}

InitDirect3DApp::~InitDirect3DApp() {}

bool InitDirect3DApp::Init()
{
	if (!D3DApp::Init())
		return false;

	return true;
}

void InitDirect3DApp::OnResize()
{
	D3DApp::OnResize();
}

void InitDirect3DApp::UpdateScene(float dt)
{
}

void InitDirect3DApp::DrawScene()
{
	assert(md3dImmediateContext);
	assert(mSwapChain);

	//후면 버퍼를 파란색으로 설정.
	md3dImmediateContext->ClearRenderTargetView(mRenderTargetView, reinterpret_cast<const float*>(&Colors::Blue));
	//깊이 버퍼를 1.0f로, 스텐실 버퍼를 0으로 지운다.
	md3dImmediateContext->ClearDepthStencilView(mDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	HR(mSwapChain->Present(0, 0));
}

bool InitDirect3DApp::InitDirect3D()
{
	if (!D3DApp::InitDirect3D())
		return false;

#if defined(DEBUG) | defined(_DEBUG)

	IDXGIDevice* dxgiDevice = 0;
	HR(md3dDevice->QueryInterface(__uuidof(IDXGIDevice), (void**)&dxgiDevice));

	IDXGIAdapter* dxgiAdaptor = 0;
	HR(dxgiDevice->GetParent(__uuidof(IDXGIAdapter), (void**)&dxgiAdaptor));

	IDXGIFactory* dxgiFactory = 0;
	HR(dxgiAdaptor->GetParent(__uuidof(IDXGIFactory), (void**)&dxgiFactory));

	//문제2
	std::wostringstream woss;

	UINT i = 0;
	std::vector<IDXGIAdapter*> vAdapter;
	IDXGIAdapter* tempAdapter = 0;
	//ENUMAdapters는 비디오 카드를 열거한다.
	for (i = 0; dxgiFactory->EnumAdapters(i, &tempAdapter) != DXGI_ERROR_NOT_FOUND; ++i)
	{
		vAdapter.push_back(tempAdapter);
		DXGI_ADAPTER_DESC tempDesc;
		tempAdapter->GetDesc(&tempDesc);
		woss << "*** ADAPTER " << i << " : " << tempDesc.Description << "\n";
	}
	woss << "*** NUM ADAPTERS = " << i << "\n";

	D3D11_BLEND_DESC temp_BlendDesc;
	ZeroMemory(&temp_BlendDesc, sizeof(D3D11_BLEND_DESC));
	temp_BlendDesc.AlphaToCoverageEnable = false;
	temp_BlendDesc.IndependentBlendEnable = false;
	temp_BlendDesc.RenderTarget[0].BlendEnable = false;
	temp_BlendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	i = 0;
	//문제 3
	for (auto iter = vAdapter.begin(); iter != vAdapter.end(); iter++, ++i) {
		ID3D11Device* tempD3dDevice;
		ID3D11DeviceContext* tempD3dContext;
		D3D_FEATURE_LEVEL tempFeatureLevel;
		//IDXGIAdapter를 EnumAdapters로 얻으면 HARDWARE, CreateSoftwareAdapter로 얻으면 SOFTWARE가 된다.
		//D3D_DRIVER_TYPE_UNKNOWN은 해당 어댑터가 임의의 경로로 생성되어 특정하기 어려우므로 런타임에서 정보를 얻어오라고 지시하는 것이다.
		HRESULT hr = D3D11CreateDevice(
			*iter,
			D3D_DRIVER_TYPE::D3D_DRIVER_TYPE_UNKNOWN,
			0,
			0,
			0,
			0,
			D3D11_SDK_VERSION,
			&tempD3dDevice,
			&tempFeatureLevel,
			&tempD3dContext);
		if (tempD3dDevice != nullptr)
		{
			ID3D11BlendState* temp_BlendState;
			if (!FAILED(tempD3dDevice->CreateBlendState(&temp_BlendDesc, &temp_BlendState)))
			{
				woss << "*** D3D11 SUPPORTED FOR ADAPTER = " << i << "\n";
			}
			ReleaseCOM(temp_BlendState);
			ReleaseCOM(tempD3dContext);
			ReleaseCOM(tempD3dDevice);
		}
	}
	//문제 4
	i = 0;
	IDXGIOutput* pOutput;
	std::vector<IDXGIOutput*> vOutputs;
	while (dxgiAdaptor->EnumOutputs(i, &pOutput) != DXGI_ERROR_NOT_FOUND)
	{
		vOutputs.push_back(pOutput);
		++i;
	}
	woss << "*** NUM OUTPUTS FOR DEFAULT ADAPTER = " << i << "\n";

	//문제 5
	for (auto iter = vOutputs.begin(); iter != vOutputs.end(); iter++)
	{
		UINT _nModes = 0;
		(*iter)->GetDisplayModeList(DXGI_FORMAT::DXGI_FORMAT_R8G8B8A8_UNORM, NULL, &_nModes, NULL);

		if (_nModes <= 0)
			break;

#pragma warning(push)
#pragma warning(disable : 6385)
		DXGI_MODE_DESC* _displayModes = new DXGI_MODE_DESC[_nModes];
		(*iter)->GetDisplayModeList(DXGI_FORMAT::DXGI_FORMAT_R8G8B8A8_UNORM, NULL, &_nModes, _displayModes);
		for (UINT n = 0; n < _nModes; n++)
		{
			woss << "***WIDTH = " << _displayModes[n].Width << " HEIGHT = " << _displayModes[n].Height << " REFRESH = " << _displayModes[n].RefreshRate.Numerator << "/" << _displayModes[n].RefreshRate.Denominator << "\n";
		}
#pragma warning(pop)
	}

	OutputDebugString(woss.str().c_str());
#endif // defined(DEBUG) | defined(_DEBUG)


	return true;
}
