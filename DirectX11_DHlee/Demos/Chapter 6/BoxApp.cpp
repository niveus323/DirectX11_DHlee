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
	//â ũ�Ⱑ ����� ������ ����� â�� ũ�� ������ �°� ���� ��ȯ�� �̷�������Ѵ�.
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

	//ī�޶� ��ȯ ��� ����
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
	//�ڽ��� �׸��� ���� �̸� �����ص� ���� ���۸� ����
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

	//GPU�� ���� �迭�� �����ϱ� ���ؼ��� �������� ���ۿ� �����ؾ��Ѵ�.
	//������ ��� ���۸� ���� ���۶� �ϸ�, CPU��GPU�� ��� �ڷῡ ������ �� �ְ�,
	//���۰� ������������ ��� ���̴����� ���� ������ ������.

	//D3D11_BUFFER_DESC�� ������ ������ ������ �����Ѵ�.
	//D3D11_BUFFER_DESC�� Usage�� ���۰� ���̴� ����� �ǹ��Ѵ�.
		//1. D3D11_USAGE_DEFAULT : GPU�� ������ �ڿ��� �а� ����� �� �����Ѵ�. 
		//CPU�� �� �뵵�� ���� �ڿ��� ���� API�� ���� �аų� ������ ������, UpdateSubresource�� ����ϴ� ���� �����ϴ�.
		//2. D3D11_USAGE_IMMUTABLE : GPU�� �б� �������� �����Ѵ�.
		//���� �������� �ڿ��� �ʱ�ȭ�� ���� �����ϸ�, CPU�� �ش� �ڿ��� ���� ���ϸ�, ���Τ����� �޼��忡 ����� �� ����.
		//3. D3D11_USAGE_DYNAMMIC : CPU�� �ڿ��� �ڷ᳻���� ���� �����ؾ��� �� �����Ѵ�.
		//GPU�� �ش� �뵵�� �ڿ��� ���� �� ������, CPU�� ����API�� ���� �ڷḦ ����� �� �ִ�.
		//GPU�� �ڿ��� CPU���� �������� �����ϸ� ���ɻ� ���ذ� �����.(�� �ڷḦ CPU �޸� -> GPU �޸𸮷� �����ؾ� �ϱ� ����.)
		//�� �ʿ��� ��찡 �ƴ϶�� ����� �����ؾ��Ѵ�.
		//4. D3D11_USAGE_STAGING : �ڿ��� �ڷḦ GPU �޸𸮿��� CPU�޸𸮷� ������ �� �־�� �� �� �����Ѵ�.
		//GPU �޸� -> CPU �޸𸮷��� �ڷ� ����� ���� �����̴�. �� �ʿ��� ��찡 �ƴ϶�� ���ؾ��Ѵ�.
	//D3D11_BUFFER_DESC�� CPUAccessFlags�� CPU�� ���ۿ� �����ϴ� ����� �����Ѵ�.
		//CPU�� ���ۿ� �������� �ʾƾ� �Ѵٸ� NULL(0)�� �����ϰ� ������ ��� ���� �뵵�� ���� Flag�� �����Ѵ�.
		//1. D3D11_CPU_ACCESS_WRITE : CPU�� �����Ͽ� ���۸� ����(���Ⱑ �����ؾ��ϹǷ� Usage�� DYNAMIC/STAGING�̾�� �Ѵ�)
		//2. D3D11_CPU_ACCESS_READ : CPU�� �����Ͽ� �ڷḦ ���� �� �ֵ��� �Ѵ�.(Usage�� STAGING�̾�� �Ѵ�.)

	//���� ����
	D3D11_BUFFER_DESC vbd{};
	vbd.Usage = D3D11_USAGE_IMMUTABLE;	//GPU �б� ������ �������� ����
	vbd.ByteWidth = sizeof(Vertex) * 8;
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;	//���� �������� �����ϱ� ���� Flag����
	vbd.CPUAccessFlags = 0;	//CPU�� ���ۿ� �����ϴ� ��� - CPU�� ���۸� �аų� ���� �ʱ� ������ 0���� ����.
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

	//���� ����
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
	//Windows SDK 10���� ȿ�� ����(FX ����)�� �����Ű�� ��� 2����.

	//1.fx������ ���� �������Ͽ� fxo������ �����Ų��.
	//����! FX�� ���� �������ϱ� ���� �����Ҷ��� ���ɾ fx������ ���� ������ ����η� �Է��ؾ�������
	//���ø����̼� ������ fxo������ �����ų ���� �ۼ��� �ڵ��� ��ġ�κ��� ����θ� �ۼ��ؾ��Ѵ�. (�ַ�� ��ġ�κ��� ���۵Ǵ� �͵� �ƴ�!!)
	std::ifstream fin("../FX/color.fxo", std::ios::binary);

	//������ ������ �����ͷ� �̵��Ͽ� ���� ũ�⸦ ����
	fin.seekg(0, std::ios_base::end);
	int size = (int)fin.tellg();
	//���� ũ�� ���� ���Ŀ��� �����͸� �� ������ �ٽ� �̵�
	fin.seekg(0, std::ios_base::beg);
	//������ ũ�⸸ŭ vector�� ũ�� ����
	std::vector<char> compiledShader(size);
	//vector�� ���̴� ������ ����
	fin.read(&compiledShader[0], size);
	//���� �ݱ�
	fin.close();

	//���� ���̴� ������ ������ ȿ�� �������̽� ����
	HR(D3DX11CreateEffectFromMemory(&compiledShader[0],size,0,md3dDevice, &mFX));
	
	/*
	//2. fx������ ���ø����̼� ���� �������� ������
	//
	DWORD shaderFlags = 0;
#if defined( DEBUG ) || defined( _DEBUG )
	shaderFlags |= D3D10_SHADER_DEBUG;
	shaderFlags |= D3D10_SHADER_SKIP_OPTIMIZATION;
#endif
	ID3D10Blob* compiledShader = 0;
	ID3D10Blob* compilationMsgs = 0;
	HRESULT hr = D3DCompileFromFile(TEXT("../FX/color.fx"), NULL, NULL, NULL, "fx_5_0", shaderFlags, 0, &compiledShader, &compilationMsgs);

	//���̴� �����Ͽ��� ������ �߻��ϸ�  compilationMsgs�� �޽����� �����
	if (compilationMsgs != 0)
	{
		MessageBoxA(0, (char*)compilationMsgs->GetBufferPointer(), 0, 0);
		ReleaseCOM(compilationMsgs);
	}

	//D3DCompileFromFile�� ������ ��� �ش� �޽����� ���
	if (FAILED(hr))
	{
		LPWSTR output = nullptr;
		FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, hr, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)&output, 0, NULL);
		MessageBox(NULL, output, TEXT("D3DCompileFromFile"), MB_OK);
	}

	HR(D3DX11CreateEffectFromMemory(compiledShader->GetBufferPointer(), compiledShader->GetBufferSize(),0, md3dDevice, &mFX));
	//�����ϵ� ���̴� ������
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