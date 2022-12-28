#include "d3dApp.h"
#include "MathHelper.h"
#include "d3dx11effect.h"

struct Vertex
{
	XMFLOAT3 Pos;
	XMCOLOR Color;
};
//연습문제 7~10번
class Exercise_Chapter6_7 : public D3DApp
{
public:
	Exercise_Chapter6_7(HINSTANCE hInstance);
	~Exercise_Chapter6_7();

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
	ID3D11Buffer* mPrimitiveVB;
	ID3D11Buffer* mPrimitiveIB;

	ID3DX11Effect* mFX;
	ID3DX11EffectTechnique* mTech;
	ID3DX11EffectMatrixVariable* mfxWorldViewProj;

	ID3D11InputLayout* mInputLayout;

	ID3D11RasterizerState* mRS;

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

	Exercise_Chapter6_7 theApp(hInstance);

	if (!theApp.Init())
		return 0;

	return theApp.Run();
}
#pragma warning(pop)

Exercise_Chapter6_7::Exercise_Chapter6_7(HINSTANCE hInstance) :
	D3DApp(hInstance),
	mPrimitiveVB(0),
	mPrimitiveIB(0),
	mFX(0),
	mTech(0),
	mfxWorldViewProj(0),
	mInputLayout(0),
	mRS(0),
	mTheta(1.5f * MathHelper::Pi),
	mPhi(0.25f * MathHelper::Pi),
	mRadius(5.0f)
{
	mMainWndCaption = TEXT("연습문제 6장 7번");

	mLastMousePos.x = 0;
	mLastMousePos.y = 0;

	XMMATRIX I = XMMatrixIdentity();
	XMStoreFloat4x4(&mWorld, I);
	XMStoreFloat4x4(&mView, I);
	XMStoreFloat4x4(&mProj, I);

}

Exercise_Chapter6_7::~Exercise_Chapter6_7()
{
	ReleaseCOM(mPrimitiveVB);
	ReleaseCOM(mPrimitiveIB);
	ReleaseCOM(mFX);
	ReleaseCOM(mInputLayout);
	ReleaseCOM(mRS);
}

bool Exercise_Chapter6_7::Init()
{
	if (!D3DApp::Init())
		return false;
	BuildGeometryBuffers();
	BuildFX();
	BuildVertexLayout();

	D3D11_RASTERIZER_DESC rsDesc;
	ZeroMemory(&rsDesc, sizeof(D3D11_RASTERIZER_DESC));
	rsDesc.FillMode = D3D11_FILL_SOLID;
	rsDesc.CullMode = D3D11_CULL_NONE;
	rsDesc.FrontCounterClockwise = false;
	rsDesc.DepthClipEnable = true;

	HR(md3dDevice->CreateRasterizerState(&rsDesc, &mRS));

	return true;
}

void Exercise_Chapter6_7::OnResize()
{
	D3DApp::OnResize();
	XMMATRIX P = XMMatrixPerspectiveFovLH(0.25f * MathHelper::Pi, AspectRatio(), 1.0f, 1000.0f);
	XMStoreFloat4x4(&mProj, P);
}

void Exercise_Chapter6_7::UpdateScene(float dt)
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

void Exercise_Chapter6_7::DrawScene()
{
	md3dImmediateContext->ClearRenderTargetView(mRenderTargetView, reinterpret_cast<const float*>(&Colors::LightSteelBlue));
	md3dImmediateContext->ClearDepthStencilView(mDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
	md3dImmediateContext->IASetInputLayout(mInputLayout);
	md3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	md3dImmediateContext->RSSetState(mRS);

	UINT stride = sizeof(Vertex);
	UINT offset = 0;
	md3dImmediateContext->IASetVertexBuffers(0, 1, &mPrimitiveVB, &stride, &offset);
	md3dImmediateContext->IASetIndexBuffer(mPrimitiveIB, DXGI_FORMAT_R32_UINT, 0);

	XMMATRIX world = XMMatrixTranslation(-1.0f,0.0f,0.0f);
	XMMATRIX pyramidWorld = XMMatrixTranslation(2.0f, 0.0f, 0.0f);
	XMMATRIX view = XMLoadFloat4x4(&mView);
	XMMATRIX proj = XMLoadFloat4x4(&mProj);
	XMMATRIX worldViewProj = world * view * proj;

	mfxWorldViewProj->SetMatrix(reinterpret_cast<float*>(&worldViewProj));

	D3DX11_TECHNIQUE_DESC techDesc;
	mTech->GetDesc(&techDesc);
	for (UINT p = 0; p < techDesc.Passes; ++p)
	{
		mTech->GetPassByIndex(p)->Apply(0, md3dImmediateContext);
		md3dImmediateContext->DrawIndexed(36, 0, 0);

		mfxWorldViewProj->SetMatrix(reinterpret_cast<float*>(&(pyramidWorld * view * proj)));
		mTech->GetPassByIndex(p)->Apply(0, md3dImmediateContext);
		md3dImmediateContext->DrawIndexed(18, 36, 8);

	}

	HR(mSwapChain->Present(0, 0));
}

void Exercise_Chapter6_7::OnMouseDown(WPARAM btnState, int x, int y)
{
	mLastMousePos.x = x;
	mLastMousePos.y = y;

	SetCapture(mhMainWnd);
}

void Exercise_Chapter6_7::OnMouseUp(WPARAM btnState, int x, int y)
{
	ReleaseCapture();
}

void Exercise_Chapter6_7::OnMouseMove(WPARAM btnState, int x, int y)
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

void Exercise_Chapter6_7::BuildGeometryBuffers()
{
	Vertex vertices[] =
	{
		//Cube
		{ XMFLOAT3(-1.0f, -1.0f, -1.0f), Convert::ArgbToAbgr(XMCOLOR((const float *)Colors::Red))   },
		{ XMFLOAT3(-1.0f, +1.0f, -1.0f), Convert::ArgbToAbgr(XMCOLOR((const float*)Colors::Black)) },
		{ XMFLOAT3(+1.0f, +1.0f, -1.0f), Convert::ArgbToAbgr(XMCOLOR((const float *)Colors::Blue)) },
		{ XMFLOAT3(+1.0f, -1.0f, -1.0f), Convert::ArgbToAbgr(XMCOLOR((const float *)Colors::Cyan)) },
		{ XMFLOAT3(-1.0f, -1.0f, +1.0f), Convert::ArgbToAbgr(XMCOLOR((const float *)Colors::Green)) },
		{ XMFLOAT3(-1.0f, +1.0f, +1.0f), Convert::ArgbToAbgr(XMCOLOR((const float *)Colors::Silver)) },
		{ XMFLOAT3(+1.0f, +1.0f, +1.0f), Convert::ArgbToAbgr(XMCOLOR((const float *)Colors::Magenta)) },
		{ XMFLOAT3(+1.0f, -1.0f, +1.0f), Convert::ArgbToAbgr(XMCOLOR((const float *)Colors::Yellow)) },
		//Pyramid
		{ XMFLOAT3(-1.0f, -1.0f, -1.0f), Convert::ArgbToAbgr(XMCOLOR((const float *)Colors::Green))   },
		{ XMFLOAT3(-1.0f, -1.0f, +1.0f), Convert::ArgbToAbgr(XMCOLOR((const float *)Colors::Green)) },
		{ XMFLOAT3(+1.0f, -1.0f, +1.0f), Convert::ArgbToAbgr(XMCOLOR((const float *)Colors::Green)) },
		{ XMFLOAT3(+1.0f, -1.0f, -1.0f), Convert::ArgbToAbgr(XMCOLOR((const float *)Colors::Green)) },
		{ XMFLOAT3(0.0f,  1.0f, 0.0f),	 Convert::ArgbToAbgr(XMCOLOR((const float *)Colors::Red)) }

	};

	D3D11_BUFFER_DESC vbd{};
	vbd.Usage = D3D11_USAGE_IMMUTABLE;
	vbd.ByteWidth = sizeof(Vertex) * 13;
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = 0;
	vbd.MiscFlags = 0;
	vbd.StructureByteStride = 0;
	D3D11_SUBRESOURCE_DATA vinitData{};
	vinitData.pSysMem = vertices;
	HR(md3dDevice->CreateBuffer(&vbd, &vinitData, &mPrimitiveVB));

	UINT indices[] = {
		0, 1, 2,
		0, 2, 3,
		4, 6, 5,
		4, 7, 6,
		4, 5, 1,
		4, 1, 0,
		3, 2, 6,
		3, 6, 7,
		1, 5, 6,
		1, 6, 2,
		4, 0, 3,
		4, 3, 7,

		0, 1, 2,
		0, 2, 3,
		4, 1, 0,
		4, 2, 1,
		4, 3, 2,
		4, 0, 3
	};

	D3D11_BUFFER_DESC ibd{};
	ibd.Usage = D3D11_USAGE_IMMUTABLE;
	ibd.ByteWidth = sizeof(UINT) * 54;
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibd.CPUAccessFlags = 0;
	ibd.MiscFlags = 0;
	ibd.StructureByteStride = 0;
	D3D11_SUBRESOURCE_DATA iinitData{};
	iinitData.pSysMem = indices;
	HR(md3dDevice->CreateBuffer(&ibd, &iinitData, &mPrimitiveIB));
}

void Exercise_Chapter6_7::BuildFX()
{
	std::ifstream fin("../FX/color.fxo", std::ios::binary);
	fin.seekg(0, std::ios_base::end);
	int size = (int)fin.tellg();
	fin.seekg(0, std::ios_base::beg);
	std::vector<char> compiledShader(size);
	fin.read(&compiledShader[0], size);
	fin.close();

	HR(D3DX11CreateEffectFromMemory(&compiledShader[0], size, 0, md3dDevice, &mFX));

	mTech = mFX->GetTechniqueByName("ColorTech");
	mfxWorldViewProj = mFX->GetVariableByName("gWorldViewProj")->AsMatrix();
}

void Exercise_Chapter6_7::BuildVertexLayout()
{
	D3D11_INPUT_ELEMENT_DESC vertexDesc[] =
	{
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"COLOR",    0, DXGI_FORMAT_R8G8B8A8_UNORM, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0}
	};

	D3DX11_PASS_DESC passDesc;
	mTech->GetPassByIndex(0)->GetDesc(&passDesc);
	HR(md3dDevice->CreateInputLayout(vertexDesc, 2, passDesc.pIAInputSignature, passDesc.IAInputSignatureSize, &mInputLayout));
}
