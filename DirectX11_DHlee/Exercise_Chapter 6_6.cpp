#include "d3dApp.h"
#include "MathHelper.h"
#include "d3dx11effect.h"

struct Vertex
{
	XMFLOAT3 Pos;
	XMFLOAT4 Color;
};

class Exercise_Chapter6_6 : public D3DApp
{
public:
	Exercise_Chapter6_6(HINSTANCE hInstance);
	~Exercise_Chapter6_6();

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
	ID3DX11EffectScalarVariable* mfxGameTime;

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

	Exercise_Chapter6_6 theApp(hInstance);

	if (!theApp.Init())
		return 0;

	return theApp.Run();
}
#pragma warning(pop)

Exercise_Chapter6_6::Exercise_Chapter6_6(HINSTANCE hInstance) :
	D3DApp(hInstance),
	mBoxVB(0),
	mBoxIB(0),
	mFX(0),
	mTech(0),
	mfxWorldViewProj(0),
	mfxGameTime(0),
	mInputLayout(0),
	mTheta(1.5f * MathHelper::Pi),
	mPhi(0.25f * MathHelper::Pi),
	mRadius(5.0f)
{
	mMainWndCaption = TEXT("연습문제 6장 6번");

	mLastMousePos.x = 0;
	mLastMousePos.y = 0;

	XMMATRIX I = XMMatrixIdentity();
	XMStoreFloat4x4(&mWorld, I);
	XMStoreFloat4x4(&mView, I);
	XMStoreFloat4x4(&mProj, I);

}

Exercise_Chapter6_6::~Exercise_Chapter6_6()
{
	ReleaseCOM(mBoxVB);
	ReleaseCOM(mBoxIB);
	ReleaseCOM(mFX);
	ReleaseCOM(mInputLayout);
}

bool Exercise_Chapter6_6::Init()
{
	if (!D3DApp::Init())
		return false;
	BuildGeometryBuffers();
	BuildFX();
	BuildVertexLayout();

	return true;
}

void Exercise_Chapter6_6::OnResize()
{
	D3DApp::OnResize();
	XMMATRIX P = XMMatrixPerspectiveFovLH(0.25f * MathHelper::Pi, AspectRatio(), 1.0f, 1000.0f);
	XMStoreFloat4x4(&mProj, P);
}

void Exercise_Chapter6_6::UpdateScene(float dt)
{
	float x = mRadius * sinf(mPhi) * cosf(mTheta);
	float z = mRadius * sinf(mPhi) * sinf(mTheta);
	float y = mRadius * cosf(mPhi);

	XMVECTOR pos = XMVectorSet(x, y, z, 1.0f);
	XMVECTOR target = XMVectorZero();
	XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

	XMMATRIX V = XMMatrixLookAtLH(pos, target, up);
	XMStoreFloat4x4(&mView, V);

	//

	//D3D11_MAPPED_SUBRESOURCE mappedData;
	//HR(md3dImmediateContext->Map(mBoxVB, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedData));

	//Vertex* v = reinterpret_cast<Vertex*>(mappedData.pData);
	//for (UINT i = 0; i < 36; i++)
	//{
	//	v[i].Pos = mCurVertexPositions[i];
	//}
	//md3dImmediateContext->Unmap(mBoxVB, 0);
}

void Exercise_Chapter6_6::DrawScene()
{
	md3dImmediateContext->ClearRenderTargetView(mRenderTargetView, reinterpret_cast<const float*>(&Colors::LightSteelBlue));
	md3dImmediateContext->ClearDepthStencilView(mDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
	md3dImmediateContext->IASetInputLayout(mInputLayout);
	md3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	UINT stride = sizeof(Vertex);
	UINT offset = 0;

	XMMATRIX world = XMLoadFloat4x4(&mWorld);
	XMMATRIX view = XMLoadFloat4x4(&mView);
	XMMATRIX proj = XMLoadFloat4x4(&mProj);
	XMMATRIX worldViewProj = world * view * proj;

	mfxWorldViewProj->SetMatrix(reinterpret_cast<float*>(&worldViewProj));
	mfxGameTime->SetFloat(mTimer.GameTime());

	D3DX11_TECHNIQUE_DESC techDesc;
	mTech->GetDesc(&techDesc);
	for (UINT p = 0; p < techDesc.Passes; ++p)
	{
		md3dImmediateContext->IASetVertexBuffers(0, 1, &mBoxVB, &stride, &offset);
		md3dImmediateContext->IASetIndexBuffer(mBoxIB, DXGI_FORMAT_R32_UINT, 0);
		mTech->GetPassByIndex(p)->Apply(0, md3dImmediateContext);
		md3dImmediateContext->DrawIndexed(36, 0, 0);
	}

	HR(mSwapChain->Present(0, 0));
}

void Exercise_Chapter6_6::OnMouseDown(WPARAM btnState, int x, int y)
{
	mLastMousePos.x = x;
	mLastMousePos.y = y;

	SetCapture(mhMainWnd);
}

void Exercise_Chapter6_6::OnMouseUp(WPARAM btnState, int x, int y)
{
	ReleaseCapture();
}

void Exercise_Chapter6_6::OnMouseMove(WPARAM btnState, int x, int y)
{
	if ((btnState & MK_LBUTTON) != 0)
	{
		float dx = XMConvertToRadians(0.25f * static_cast<float>(x - mLastMousePos.x));
		float dy = XMConvertToRadians(0.25f * static_cast<float>(y - mLastMousePos.y));

		mTheta += dx;
		mPhi += dy;

		mPhi = MathHelper::Clamp(mPhi, 0.1f, MathHelper::Pi - 0.1f);
	}
	else if ((btnState & MK_RBUTTON) != 0)
	{
		float dx = 0.005f * static_cast<float>(x - mLastMousePos.x);
		float dy = 0.005f * static_cast<float>(y - mLastMousePos.y);

		mRadius += dx - dy;

		mRadius = MathHelper::Clamp(mRadius, 3.0f, 15.0f);
	}

	mLastMousePos.x = x;
	mLastMousePos.y = y;
}

void Exercise_Chapter6_6::BuildGeometryBuffers()
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

	//정점 버퍼
	D3D11_BUFFER_DESC vbd{};
	vbd.Usage = D3D11_USAGE_DYNAMIC;	//GPU 읽기 전용의 버퍼임을 명시
	vbd.ByteWidth = sizeof(Vertex) * 8;
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;	//정점 버퍼임을 명시하기 위한 Flag설정
	vbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;	//CPU가 버퍼에 접근하는 방식 - CPU가 버퍼를 읽거나 쓰지 않기 때문에 0으로 설정.
	vbd.MiscFlags = 0;
	vbd.StructureByteStride = 0;
	D3D11_SUBRESOURCE_DATA vinitData{};
	vinitData.pSysMem = vertices;
	HR(md3dDevice->CreateBuffer(&vbd, &vinitData, &mBoxVB));
	//HR(md3dDevice->CreateBuffer(&vbd, 0, &mBoxVB));

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

void Exercise_Chapter6_6::BuildFX()
{
	std::ifstream fin("../FX/animBox.fxo", std::ios::binary);
	fin.seekg(0, std::ios_base::end);
	int size = (int)fin.tellg();
	fin.seekg(0, std::ios_base::beg);
	std::vector<char> compiledShader(size);
	fin.read(&compiledShader[0], size);
	fin.close();
	HR(D3DX11CreateEffectFromMemory(&compiledShader[0], size, 0, md3dDevice, &mFX));

	mTech = mFX->GetTechniqueByName("ColorTech");

	mfxWorldViewProj = mFX->GetVariableByName("gWorldViewProj")->AsMatrix();
	mfxGameTime = mFX->GetVariableByName("gTime")->AsScalar();
}

void Exercise_Chapter6_6::BuildVertexLayout()
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
