#include "d3dApp.h"
#include "MathHelper.h"

struct Vertex
{
	XMFLOAT3 Pos;
	XMFLOAT4 Color;
};

class BoxApp : public D3DApp
{
public:
	BoxApp(HINSTANCE hInstance);
	~BoxApp();

	bool Init();
	void OnResize();
	void UpdateScene(float dt);
	void DrawScene();

	void OnMouseDown(WPARAM btnState, int x, int y);
	void OnMouseUp(WPARAM btnState, int x, int y);
	void OnMouseMove(WPARAM btnState, int x, int y);

private:
	void BuildGeometryBuffers();
	void BuildFX();
	void BuildVertexLayout();

private:
	ID3D11Buffer* mBoxVB;
	ID3D11Buffer* mBoxIB;

	ID3DX11Effect* mFX;
	ID3DX11EffectTechnique* mTech;
	ID3DX11EffectMatrixVariable* mfxWorldViewProj;

	ID3D11InputLayout* mInputLayout;

	XMFLOAT4X4 mWorld;
	XMFLOAT4X4 mView;
	XMFLOAT4X4 mProj;

	float mTheta;
	float mPhi;
	float mRadius;

	POINT mLastMousePos;
};

#pragma warning(push)
#pragma warning(disable : 28251)
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR cmdLine, int showCmd)
{
#if defined(DEBUG) | defined(_DEBUG)
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

	BoxApp theApp(hInstance);

	if (!theApp.Init())
		return 0;

	return theApp.Run();
}
#pragma warning(pop)

BoxApp::BoxApp(HINSTANCE hInstance) :
	D3DApp(hInstance),
	mBoxVB(0),
	mBoxIB(0),
	mFX(0),
	mTech(0),
	mfxWorldViewProj(0),
	mInputLayout(0),
	mTheta(1.5f*MathHelper::Pi),
	mPhi(0.25f*MathHelper::Pi),
	mRadius(5.0f)
{
	mMainWndCaption = TEXT("Box Demo");
	
	mLastMousePos.x = 0;
	mLastMousePos.y = 0;

	XMMATRIX I = XMMatrixIdentity();
	XMStoreFloat4x4(&mWorld, I);
	XMStoreFloat4x4(&mView, I);
	XMStoreFloat4x4(&mProj, I);

}

BoxApp::~BoxApp()
{
	ReleaseCOM(mBoxVB);
	ReleaseCOM(mBoxIB);
	ReleaseCOM(mFX);
	ReleaseCOM(mInputLayout);
}

bool BoxApp::Init()
{
	if(!D3DApp::Init())
		return false;
	BuildGeometryBuffers();
	BuildFX();
	BuildVertexLayout();

	return true;
}

void BoxApp::OnResize()
{
	D3DApp::OnResize();
	//창 크기가 변경될 때마다 변경된 창의 크기 비율에 맞게 투영 변환이 이루어져야한다.
	XMMATRIX P = XMMatrixPerspectiveFovLH(0.25f * MathHelper::Pi, AspectRatio(), 1.0f, 1000.0f);
	XMStoreFloat4x4(&mProj, P);
}

void BoxApp::UpdateScene(float dt)
{
	float x = mRadius * sinf(mPhi) * cosf(mTheta);
	float z = mRadius * sinf(mPhi) * sinf(mTheta);
	float y = mRadius * cosf(mPhi);

	XMVECTOR pos = XMVectorSet(x, y, z, 1.0f);
	XMVECTOR target = XMVectorZero();
	XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

	//카메라 변환 행렬 생성
	XMMATRIX V = XMMatrixLookAtLH(pos, target, up);
	XMStoreFloat4x4(&mView, V);
}

void BoxApp::DrawScene()
{
	md3dImmediateContext->ClearRenderTargetView(mRenderTargetView, reinterpret_cast<const float*>(&Colors::LightSteelBlue));
	md3dImmediateContext->ClearDepthStencilView(mDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
	md3dImmediateContext->IASetInputLayout(mInputLayout);
	md3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	UINT stride = sizeof(Vertex);
	UINT offset = 0;
	//박스를 그리기 위해 미리 지정해둔 정점 버퍼를 저장
	md3dImmediateContext->IASetVertexBuffers(0, 1, &mBoxVB, &stride, &offset);
	md3dImmediateContext->IASetIndexBuffer(mBoxIB, DXGI_FORMAT_R32_UINT, 0);

	XMMATRIX world = XMLoadFloat4x4(&mWorld);
	XMMATRIX view = XMLoadFloat4x4(&mView);
	XMMATRIX proj = XMLoadFloat4x4(&mProj);
	XMMATRIX worldViewProj = world * view * proj;

	mfxWorldViewProj->SetMatrix(reinterpret_cast<float*>(&worldViewProj));

	D3DX11_TECHNIQUE_DESC techDesc;
	mTech->GetDesc(&techDesc);
	for (UINT p = 0; p < techDesc.Passes; ++p)
	{
		mTech->GetPassByIndex(p)->Apply(0, md3dImmediateContext);

		// 36 indices for the box.
		md3dImmediateContext->DrawIndexed(36, 0, 0);
	}

	HR(mSwapChain->Present(0, 0));
}

void BoxApp::OnMouseDown(WPARAM btnState, int x, int y)
{
	mLastMousePos.x = x;
	mLastMousePos.y = y;

	SetCapture(mhMainWnd);
}

void BoxApp::OnMouseUp(WPARAM btnState, int x, int y)
{
	ReleaseCapture();
}

void BoxApp::OnMouseMove(WPARAM btnState, int x, int y)
{
	if ((btnState & MK_LBUTTON) != 0)
	{
		// Make each pixel correspond to a quarter of a degree.
		float dx = XMConvertToRadians(0.25f * static_cast<float>(x - mLastMousePos.x));
		float dy = XMConvertToRadians(0.25f * static_cast<float>(y - mLastMousePos.y));

		// Update angles based on input to orbit camera around box.
		mTheta += dx;
		mPhi += dy;

		// Restrict the angle mPhi.
		mPhi = MathHelper::Clamp(mPhi, 0.1f, MathHelper::Pi - 0.1f);
	}
	else if ((btnState & MK_RBUTTON) != 0)
	{
		// Make each pixel correspond to 0.005 unit in the scene.
		float dx = 0.005f * static_cast<float>(x - mLastMousePos.x);
		float dy = 0.005f * static_cast<float>(y - mLastMousePos.y);

		// Update the camera radius based on input.
		mRadius += dx - dy;

		// Restrict the radius.
		mRadius = MathHelper::Clamp(mRadius, 3.0f, 15.0f);
	}

	mLastMousePos.x = x;
	mLastMousePos.y = y;
}

void BoxApp::BuildGeometryBuffers()
{
	Vertex vertices[] =
	{
		{ XMFLOAT3(-1.0f, -1.0f, -1.0f), Convert::ToXmFloat4(Colors::White)   },
		{ XMFLOAT3(-1.0f, +1.0f, -1.0f), Convert::ToXmFloat4(Colors::Black) },
		{ XMFLOAT3(+1.0f, +1.0f, -1.0f), Convert::ToXmFloat4(Colors::Red) },
		{ XMFLOAT3(+1.0f, -1.0f, -1.0f), Convert::ToXmFloat4(Colors::Green) },
		{ XMFLOAT3(-1.0f, -1.0f, +1.0f), Convert::ToXmFloat4(Colors::Blue) },
		{ XMFLOAT3(-1.0f, +1.0f, +1.0f), Convert::ToXmFloat4(Colors::Yellow) },
		{ XMFLOAT3(+1.0f, +1.0f, +1.0f), Convert::ToXmFloat4(Colors::Cyan) },
		{ XMFLOAT3(+1.0f, -1.0f, +1.0f), Convert::ToXmFloat4(Colors::Magenta) }
	};

	//GPU가 정점 배열에 접근하기 위해서는 정점들을 버퍼에 저장해야한다.
	//정점을 담는 버퍼를 정점 버퍼라 하며, CPUㆍGPU가 어떻게 자료에 접근할 수 있고,
	//버퍼가 파이프라인의 어디에 묶이는지에 대한 정보도 가진다.

	//D3D11_BUFFER_DESC는 생성할 버퍼의 정보를 서술한다.
	//D3D11_BUFFER_DESC의 Usage는 버퍼가 쓰이는 방식을 의미한다.
		//1. D3D11_USAGE_DEFAULT : GPU가 버퍼의 자원을 읽고 써야할 때 지정한다. 
		//CPU는 이 용도를 갖는 자원을 매핑 API를 통해 읽거나 쓸수는 없지만, UpdateSubresource를 사용하는 것은 가능하다.
		//2. D3D11_USAGE_IMMUTABLE : GPU의 읽기 전용임을 명시한다.
		//생성 시점에서 자원을 초기화할 때를 제외하면, CPU는 해당 자원을 읽지 못하며, 매핑ㆍ갱신 메서드에 사용할 수 없다.
		//3. D3D11_USAGE_DYNAMMIC : CPU가 자원의 자료내용을 자주 갱신해야할 때 지정한다.
		//GPU가 해당 용도의 자원을 읽을 수 있으며, CPU는 매핑API를 통해 자료를 기록할 수 있다.
		//GPU의 자원을 CPU에서 동적으로 갱신하면 성능상 피해가 생긴다.(새 자료를 CPU 메모리 -> GPU 메모리로 전송해야 하기 때문.)
		//꼭 필요한 경우가 아니라면 사용을 자제해야한다.
		//4. D3D11_USAGE_STAGING : 자원이 자료를 GPU 메모리에서 CPU메모리로 전송할 수 있어야 할 때 지정한다.
		//GPU 메모리 -> CPU 메모리로의 자료 복사는 느린 연산이다. 꼭 필요한 경우가 아니라면 피해야한다.
	//D3D11_BUFFER_DESC의 CPUAccessFlags는 CPU가 버퍼에 접근하는 방식을 지정한다.
		//CPU가 버퍼에 접근하지 않아야 한다면 NULL(0)을 설정하고 접근할 경우 접근 용도에 따른 Flag를 설정한다.
		//1. D3D11_CPU_ACCESS_WRITE : CPU가 접근하여 버퍼를 갱신(쓰기가 가능해야하므로 Usage가 DYNAMIC/STAGING이어야 한다)
		//2. D3D11_CPU_ACCESS_READ : CPU가 접근하여 자료를 읽을 수 있도록 한다.(Usage가 STAGING이어야 한다.)

	//정점 버퍼
	D3D11_BUFFER_DESC vbd{};
	vbd.Usage = D3D11_USAGE_IMMUTABLE;	//GPU 읽기 전용의 버퍼임을 명시
	vbd.ByteWidth = sizeof(Vertex) * 8;
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;	//정점 버퍼임을 명시하기 위한 Flag설정
	vbd.CPUAccessFlags = 0;	//CPU가 버퍼에 접근하는 방식 - CPU가 버퍼를 읽거나 쓰지 않기 때문에 0으로 설정.
	vbd.MiscFlags = 0;
	vbd.StructureByteStride = 0;
	D3D11_SUBRESOURCE_DATA vinitData{};
	vinitData.pSysMem = vertices;
	HR(md3dDevice->CreateBuffer(&vbd, &vinitData, &mBoxVB));

	UINT indices[] = {
		// front face
		0, 1, 2,
		0, 2, 3,

		// back face
		4, 6, 5,
		4, 7, 6,

		// left face
		4, 5, 1,
		4, 1, 0,

		// right face
		3, 2, 6,
		3, 6, 7,

		// top face
		1, 5, 6,
		1, 6, 2,

		// bottom face
		4, 0, 3,
		4, 3, 7
	};

	//색인 버퍼
	D3D11_BUFFER_DESC ibd{};
	ibd.Usage = D3D11_USAGE_IMMUTABLE;
	ibd.ByteWidth = sizeof(UINT) * 36;
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;	
	ibd.CPUAccessFlags = 0;
	ibd.MiscFlags = 0;
	ibd.StructureByteStride = 0;
	D3D11_SUBRESOURCE_DATA iinitData{};
	iinitData.pSysMem = indices;
	HR(md3dDevice->CreateBuffer(&ibd, &iinitData, &mBoxIB));
}

void BoxApp::BuildFX()
{
	//Windows SDK 10에서 효과 파일(FX 파일)을 실행시키는 방법 2가지.

	//1.fx파일을 직접 컴파일하여 fxo파일을 실행시킨다.
	//주의! FX를 직접 컴파일하기 위해 설정할때는 명령어가 fx파일이 속한 폴더의 상대경로로 입력해야하지만
	//애플리케이션 내에서 fxo파일을 실행시킬 때는 작성된 코드의 위치로부터 상대경로를 작성해야한다. (솔루션 위치로부터 시작되는 것도 아님!!)
	std::ifstream fin("../FX/color.fxo", std::ios::binary);

	//파일의 마지막 포인터로 이동하여 파일 크기를 측정
	fin.seekg(0, std::ios_base::end);
	int size = (int)fin.tellg();
	//파일 크기 측정 이후에는 포인터를 맨 앞으로 다시 이동
	fin.seekg(0, std::ios_base::beg);
	//측정한 크기만큼 vector의 크기 설정
	std::vector<char> compiledShader(size);
	//vector에 셰이더 정보를 저장
	fin.read(&compiledShader[0], size);
	//파일 닫기
	fin.close();

	//얻은 셰이더 정보를 가지고 효과 인터페이스 생성
	HR(D3DX11CreateEffectFromMemory(&compiledShader[0],size,0,md3dDevice, &mFX));
	
	/*
	//2. fx파일을 애플리케이션 빌드 과정에서 컴파일
	//
	DWORD shaderFlags = 0;
#if defined( DEBUG ) || defined( _DEBUG )
	shaderFlags |= D3D10_SHADER_DEBUG;
	shaderFlags |= D3D10_SHADER_SKIP_OPTIMIZATION;
#endif
	ID3D10Blob* compiledShader = 0;
	ID3D10Blob* compilationMsgs = 0;
	HRESULT hr = D3DCompileFromFile(TEXT("../FX/color.fx"), NULL, NULL, NULL, "fx_5_0", shaderFlags, 0, &compiledShader, &compilationMsgs);

	//쉐이더 컴파일에서 오류가 발생하면  compilationMsgs에 메시지가 저장됨
	if (compilationMsgs != 0)
	{
		MessageBoxA(0, (char*)compilationMsgs->GetBufferPointer(), 0, 0);
		ReleaseCOM(compilationMsgs);
	}

	//D3DCompileFromFile이 실패한 경우 해당 메시지를 출력
	if (FAILED(hr))
	{
		LPWSTR output = nullptr;
		FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, hr, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)&output, 0, NULL);
		MessageBox(NULL, output, TEXT("D3DCompileFromFile"), MB_OK);
	}

	HR(D3DX11CreateEffectFromMemory(compiledShader->GetBufferPointer(), compiledShader->GetBufferSize(),0, md3dDevice, &mFX));
	//컴파일된 쉐이더 릴리즈
	ReleaseCOM(compiledShader);
	*/
	
	mTech = mFX->GetTechniqueByName("ColorTech");
	mfxWorldViewProj = mFX->GetVariableByName("gWorldViewProj")->AsMatrix();
}

void BoxApp::BuildVertexLayout()
{
	// Create the vertex input layout.
	D3D11_INPUT_ELEMENT_DESC vertexDesc[] =
	{
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0}
	};

	// Create the input layout
	D3DX11_PASS_DESC passDesc;
	mTech->GetPassByIndex(0)->GetDesc(&passDesc);
	HR(md3dDevice->CreateInputLayout(vertexDesc, 2, passDesc.pIAInputSignature,
		passDesc.IAInputSignatureSize, &mInputLayout));
}
