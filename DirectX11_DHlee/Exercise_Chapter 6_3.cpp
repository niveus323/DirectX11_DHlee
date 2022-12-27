#include "d3dApp.h"
#include "d3dUtil.h"
#include "d3dx11effect.h"
#include "MathHelper.h"
#include "GeometryGenerator.h"
#include "resource.h"

class Exercise_Chapter6_3 : public D3DApp
{
public:
	Exercise_Chapter6_3(HINSTANCE hInstance);
	~Exercise_Chapter6_3();

	bool Init();
	void OnResize();
	void UpdateScene(float dt);
	void DrawScene();
	LRESULT MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);


	void OnMouseDown(WPARAM btnState, int x, int y);
	void OnMouseUp(WPARAM btnState, int x, int y);
	void OnMouseMove(WPARAM btnState, int x, int y);
private:
	void BuildGeometryBuffers();
	void BuildFX();
	void BuildVertexLayout();

private:
	ID3D11Buffer* mPointsVB;
	ID3D11Buffer* mPointsIB;

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
	D3D11_PRIMITIVE_TOPOLOGY drawType;


	std::vector<XMFLOAT3> mvPoints;	//기본도형을 그리기 위해서는 사용자가 지정한 점 목록을 저장할 필요가 있음.
};


#pragma warning(push)
#pragma warning(disable : 28251)
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE prevInstance, LPSTR cmdLine, int showCmd)
{
#if defined(DEBUG) | defined(_DEBUG)
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

	Exercise_Chapter6_3 theApp(hInstance);
	if (!theApp.Init())
		return 0;

	return theApp.Run();
}
#pragma warning(pop)

Exercise_Chapter6_3::Exercise_Chapter6_3(HINSTANCE hInstance) :
	D3DApp(hInstance),
	mFX(0),
	mTech(0),
	mfxWorldViewProj(0),
	mInputLayout(0),
	mTheta(1.5f * MathHelper::Pi),
	mPhi(0.25f * MathHelper::Pi),
	mRadius(5.0f)
{
	mMainWndCaption = TEXT("6장 연습문제 - 3");

	mMenuName = MAKEINTRESOURCE(IDR_MENU2);

	mLastMousePos.x = 0;
	mLastMousePos.y = 0;

	XMMATRIX I = XMMatrixIdentity();
	XMStoreFloat4x4(&mWorld, I);
	XMStoreFloat4x4(&mView, I);
	XMStoreFloat4x4(&mProj, I);
}

Exercise_Chapter6_3::~Exercise_Chapter6_3()
{
	ReleaseCOM(mPointsVB);
	ReleaseCOM(mPointsIB);
	ReleaseCOM(mFX);
	ReleaseCOM(mInputLayout);
}

bool Exercise_Chapter6_3::Init()
{
	if (!D3DApp::Init())
		return false;

	//BuildGeometryBuffers();
	BuildFX();
	BuildVertexLayout();
	return true;
}

void Exercise_Chapter6_3::OnResize()
{
	D3DApp::OnResize();
	XMMATRIX P = XMMatrixPerspectiveFovLH(0.25f * MathHelper::Pi, AspectRatio(), 1.0f, 1000.0f);
	XMStoreFloat4x4(&mProj, P);
}

void Exercise_Chapter6_3::UpdateScene(float dt)
{
	float x = mRadius * sinf(mPhi) * cosf(mTheta);
	float z = mRadius * sinf(mPhi) * sinf(mTheta);
	float y = mRadius * cosf(mPhi);

	XMVECTOR pos = XMVectorSet(x, y, z, 1.0f);
	//XMVECTOR pos = XMVectorSet(0, 0, 1.0f, 1.0f);
	XMVECTOR target = XMVectorZero();
	XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

	XMMATRIX V = XMMatrixLookAtLH(pos, target, up);
	XMStoreFloat4x4(&mView, V);
}

void Exercise_Chapter6_3::DrawScene()
{
	//Draw에서는 저장했던 Point들을 표현한다.
	md3dImmediateContext->ClearRenderTargetView(mRenderTargetView, reinterpret_cast<const float*>(&Colors::LightSteelBlue));
	md3dImmediateContext->ClearDepthStencilView(mDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
	md3dImmediateContext->IASetInputLayout(mInputLayout);
	md3dImmediateContext->IASetPrimitiveTopology(drawType);


	UINT stride = sizeof(XMFLOAT3);
	UINT offset = 0;

	XMMATRIX view = XMLoadFloat4x4(&mView);
	XMMATRIX proj = XMLoadFloat4x4(&mProj);

	D3DX11_TECHNIQUE_DESC techDesc;
	mTech->GetDesc(&techDesc);
	for (UINT p = 0; p < techDesc.Passes; ++p)
	{
		md3dImmediateContext->IASetVertexBuffers(0, 1, &mPointsVB, &stride, &offset);
		md3dImmediateContext->IASetIndexBuffer(mPointsIB, DXGI_FORMAT_R32_UINT, 0);

		XMMATRIX world = XMLoadFloat4x4(&mWorld);
		XMMATRIX worldViewProj = world * view * proj;
		mfxWorldViewProj->SetMatrix(reinterpret_cast<float*>(&worldViewProj));
		mTech->GetPassByIndex(p)->Apply(0, md3dImmediateContext);
		md3dImmediateContext->DrawIndexed(mvPoints.size(), 0, 0);
		md3dImmediateContext->RSSetState(0);
	}

	HR(mSwapChain->Present(0, 0));

}

LRESULT Exercise_Chapter6_3::MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_COMMAND:
		if (wParam < ID_TYPE_POINTLIST || wParam > ID_TYPE_TRIANGLESTRIP)
			break;

		drawType = (D3D11_PRIMITIVE_TOPOLOGY) (D3D11_PRIMITIVE_TOPOLOGY_POINTLIST + (wParam - ID_TYPE_POINTLIST));
		mvPoints.clear();

		return 0;
	}
	

	return D3DApp::MsgProc(hwnd, msg, wParam, lParam);
}

void Exercise_Chapter6_3::OnMouseDown(WPARAM btnState, int x, int y)
{
	//화면을 이동하지는 않을 것임
	//좌클릭을 하면 마우스 좌표 (x,y)를 월드 위치(x1, y1, z1)으로 변경한 후 그 위치를 저장.

	float pointX = ((float) x / mClientWidth - 0.5f)*2;
	float pointY = (0.5f - (float) y / mClientHeight)*2;
	std::wstringstream woss;
	woss << "(" << pointX << "," << pointY << ")" << "\n";
	OutputDebugString(woss.str().c_str());

	XMVECTOR screenPos = XMVectorSet(pointX, pointY, 0.0f, 1.0f);
	XMMATRIX world = XMLoadFloat4x4(&mWorld);
	XMMATRIX view = XMLoadFloat4x4(&mView);
	XMMATRIX proj = XMLoadFloat4x4(&mProj);
	XMMATRIX worldViewProj = world * view * proj;
	
	XMMATRIX iWorldViewProj = XMMatrixInverse(&XMMatrixDeterminant(worldViewProj), worldViewProj);
	XMFLOAT4 worldPos;
	XMStoreFloat4(&worldPos, XMVector4Transform(screenPos, iWorldViewProj));
	
	mvPoints.push_back(XMFLOAT3(worldPos.x / worldPos.w, worldPos.y / worldPos.w, worldPos.z / worldPos.w));

	BuildGeometryBuffers();

	//정점이 추가되었으므로 버퍼 갱신이 필요.
	D3D11_MAPPED_SUBRESOURCE mappedData;
	HR(md3dImmediateContext->Map(mPointsVB, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedData));

	XMFLOAT3* v = reinterpret_cast<XMFLOAT3*>(mappedData.pData);
	for (UINT i = 0; i < mvPoints.size(); ++i)
	{
		v[i] = mvPoints[i];
	}

	md3dImmediateContext->Unmap(mPointsVB, 0);
}

void Exercise_Chapter6_3::OnMouseUp(WPARAM btnState, int x, int y)
{
}

void Exercise_Chapter6_3::OnMouseMove(WPARAM btnState, int x, int y)
{

}

void Exercise_Chapter6_3::BuildGeometryBuffers()
{
	//동적으로 찍은 점들을 버퍼에 기록할 수 있어야한다.
	D3D11_BUFFER_DESC vbd{};
	vbd.Usage = D3D11_USAGE_DYNAMIC;
	vbd.ByteWidth = sizeof(XMFLOAT3)*mvPoints.size();
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	vbd.MiscFlags = 0;
	HR(md3dDevice->CreateBuffer(&vbd, 0, &mPointsVB));

	std::vector<UINT> indices(mvPoints.size());

	for (UINT i = 0; i < mvPoints.size(); ++i)
	{
		indices[i] = i;
	}

	D3D11_BUFFER_DESC ibd{};
	ibd.Usage = D3D11_USAGE_DYNAMIC;
	ibd.ByteWidth = sizeof(UINT) * indices.size();
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	ibd.MiscFlags = 0;
	D3D11_SUBRESOURCE_DATA iinitData{};
	iinitData.pSysMem = &indices[0];
	HR(md3dDevice->CreateBuffer(&ibd, &iinitData, &mPointsIB));
}

void Exercise_Chapter6_3::BuildFX()
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

void Exercise_Chapter6_3::BuildVertexLayout()
{
	D3D11_INPUT_ELEMENT_DESC vertexDesc[] =
	{
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 0, D3D11_INPUT_PER_VERTEX_DATA, 0}
	};

	D3DX11_PASS_DESC passDesc;
	mTech->GetPassByIndex(0)->GetDesc(&passDesc);
	HR(md3dDevice->CreateInputLayout(vertexDesc, 2, passDesc.pIAInputSignature, passDesc.IAInputSignatureSize, &mInputLayout));
}
