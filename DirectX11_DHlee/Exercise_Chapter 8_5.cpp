#include "d3dApp.h"
#include "GeometryGenerator.h"
#include "Effects.h"
#include "Vertex.h"
#include "DirectXTex.h"
using std::wstring;

class Exercise_Chapter8_5 : public D3DApp
{
public:
	Exercise_Chapter8_5(HINSTANCE hInstance);
	~Exercise_Chapter8_5();

	bool Init();
	void OnResize();
	void UpdateScene(float dt);
	void DrawScene();

	void OnMouseDown(WPARAM btnState, int x, int y);
	void OnMouseUp(WPARAM btnState, int x, int y);
	void OnMouseMove(WPARAM btnState, int x, int y);

private:
	void BuildGeometryBuffers();
	void LoadTexture(wstring path, wstring name, wstring extention, int maxAnimFrame);
	void UpdateAnimation(float playRate);

private:
	ID3D11Buffer* mBoxVB;
	ID3D11Buffer* mBoxIB;

	ID3D11ShaderResourceView* mTexture;

	DirectionalLight mDirLights[3];
	Material mBoxMat;

	XMFLOAT4X4 mTexTransform;
	XMFLOAT4X4 mBoxWorld;

	XMFLOAT4X4 mView;
	XMFLOAT4X4 mProj;

	int mBoxVertexOffset;
	UINT mBoxIndexOffset;
	UINT mBoxIndexCount;

	XMFLOAT3 mEyePosW;

	float mTheta;
	float mPhi;
	float mRadius;

	POINT mLastMousePos;

	ScratchImage* image;
	int fireAnimCount;
	float fireAnimFrame;
	int fireAnimMaxCount;
};

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE prevInstance,
	PSTR cmdLine, int showCmd)
{
	// Enable run-time memory check for debug builds.
#if defined(DEBUG) | defined(_DEBUG)
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

	Exercise_Chapter8_5 theApp(hInstance);

	if (!theApp.Init())
		return 0;

	return theApp.Run();
}


Exercise_Chapter8_5::Exercise_Chapter8_5(HINSTANCE hInstance)
	: D3DApp(hInstance), mBoxVB(0), mBoxIB(0), mEyePosW(0.0f, 0.0f, 0.0f),
	mTheta(1.3f * MathHelper::Pi), mPhi(0.4f * MathHelper::Pi), mRadius(2.5f),
	fireAnimMaxCount(120)
{
	mMainWndCaption = TEXT("8�� �������� 5��");

	mLastMousePos.x = 0;
	mLastMousePos.y = 0;

	XMMATRIX I = XMMatrixIdentity();
	XMStoreFloat4x4(&mBoxWorld, I);
	XMStoreFloat4x4(&mTexTransform, I);
	XMStoreFloat4x4(&mView, I);
	XMStoreFloat4x4(&mProj, I);

	mDirLights[0].Ambient = XMFLOAT4(0.3f, 0.3f, 0.3f, 1.0f);
	mDirLights[0].Diffuse = XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f);
	mDirLights[0].Specular = XMFLOAT4(0.6f, 0.6f, 0.6f, 16.0f);
	mDirLights[0].Direction = XMFLOAT3(0.707f, -0.707f, 0.0f);

	mDirLights[1].Ambient = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
	mDirLights[1].Diffuse = XMFLOAT4(1.4f, 1.4f, 1.4f, 1.0f);
	mDirLights[1].Specular = XMFLOAT4(0.3f, 0.3f, 0.3f, 16.0f);
	mDirLights[1].Direction = XMFLOAT3(-0.707f, 0.0f, 0.707f);

	mBoxMat.Ambient = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	mBoxMat.Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	mBoxMat.Specular = XMFLOAT4(0.6f, 0.6f, 0.6f, 16.0f);
}

Exercise_Chapter8_5::~Exercise_Chapter8_5()
{
	ReleaseCOM(mBoxVB);
	ReleaseCOM(mBoxIB);
	ReleaseCOM(mTexture);
	Effects::DestroyAll();
	InputLayouts::DestroyAll();
}

bool Exercise_Chapter8_5::Init()
{
	if (!D3DApp::Init())
		return false;

	// Must init Effects first since InputLayouts depend on shader signatures.
	Effects::InitAll(md3dDevice, TEXT("Basic.fxo"));
	InputLayouts::InitAll(md3dDevice);

	LoadTexture(TEXT("../Textures/FireAnim/"), TEXT("Fire"), TEXT(".bmp"), 120);

	BuildGeometryBuffers();

	return true;
}

void Exercise_Chapter8_5::OnResize()
{
	D3DApp::OnResize();

	XMMATRIX P = XMMatrixPerspectiveFovLH(0.25f * MathHelper::Pi, AspectRatio(), 1.0f, 1000.0f);
	XMStoreFloat4x4(&mProj, P);
}

void Exercise_Chapter8_5::UpdateScene(float dt)
{
	// Convert Spherical to Cartesian coordinates.
	float x = mRadius * sinf(mPhi) * cosf(mTheta);
	float z = mRadius * sinf(mPhi) * sinf(mTheta);
	float y = mRadius * cosf(mPhi);

	mEyePosW = XMFLOAT3(x, y, z);

	// Build the view matrix.
	XMVECTOR pos = XMVectorSet(x, y, z, 1.0f);
	XMVECTOR target = XMVectorZero();
	XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

	XMMATRIX V = XMMatrixLookAtLH(pos, target, up);
	XMStoreFloat4x4(&mView, V);
	UpdateAnimation(4);

}

void Exercise_Chapter8_5::DrawScene()
{
	md3dImmediateContext->ClearRenderTargetView(mRenderTargetView, reinterpret_cast<const float*>(&Colors::LightSteelBlue));
	md3dImmediateContext->ClearDepthStencilView(mDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	md3dImmediateContext->IASetInputLayout(InputLayouts::Basic32);
	md3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	UINT stride = sizeof(Vertex::Basic32);
	UINT offset = 0;

	XMMATRIX view = XMLoadFloat4x4(&mView);
	XMMATRIX proj = XMLoadFloat4x4(&mProj);
	XMMATRIX viewProj = view * proj;

	// Set per frame constants.
	Effects::BasicFX->SetDirLights(mDirLights);
	Effects::BasicFX->SetEyePosW(mEyePosW);

	ID3DX11EffectTechnique* activeTech = Effects::BasicFX->Light2TexTech;

	D3DX11_TECHNIQUE_DESC techDesc;
	activeTech->GetDesc(&techDesc);
	for (UINT p = 0; p < techDesc.Passes; ++p)
	{
		md3dImmediateContext->IASetVertexBuffers(0, 1, &mBoxVB, &stride, &offset);
		md3dImmediateContext->IASetIndexBuffer(mBoxIB, DXGI_FORMAT_R32_UINT, 0);

		// Draw the box.
		XMMATRIX world = XMLoadFloat4x4(&mBoxWorld);
		XMMATRIX worldInvTranspose = MathHelper::InverseTranspose(world);
		XMMATRIX worldViewProj = world * view * proj;

		Effects::BasicFX->SetWorld(world);
		Effects::BasicFX->SetWorldInvTranspose(worldInvTranspose);
		Effects::BasicFX->SetWorldViewProj(worldViewProj);
		Effects::BasicFX->SetTexTransform(XMLoadFloat4x4(&mTexTransform));
		Effects::BasicFX->SetMaterial(mBoxMat);
		Effects::BasicFX->SetDiffuseMap(mTexture);

		activeTech->GetPassByIndex(p)->Apply(0, md3dImmediateContext);
		md3dImmediateContext->DrawIndexed(mBoxIndexCount, mBoxIndexOffset, mBoxVertexOffset);
	}


	HR(mSwapChain->Present(0, 0));
}

void Exercise_Chapter8_5::OnMouseDown(WPARAM btnState, int x, int y)
{
	mLastMousePos.x = x;
	mLastMousePos.y = y;

	SetCapture(mhMainWnd);
}

void Exercise_Chapter8_5::OnMouseUp(WPARAM btnState, int x, int y)
{
	ReleaseCapture();
}

void Exercise_Chapter8_5::OnMouseMove(WPARAM btnState, int x, int y)
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
		// Make each pixel correspond to 0.01 unit in the scene.
		float dx = 0.01f * static_cast<float>(x - mLastMousePos.x);
		float dy = 0.01f * static_cast<float>(y - mLastMousePos.y);

		// Update the camera radius based on input.
		mRadius += dx - dy;

		// Restrict the radius.
		mRadius = MathHelper::Clamp(mRadius, 1.0f, 15.0f);
	}

	mLastMousePos.x = x;
	mLastMousePos.y = y;
}

void Exercise_Chapter8_5::BuildGeometryBuffers()
{
	GeometryGenerator::MeshData box;

	GeometryGenerator geoGen;
	geoGen.CreateBox(1.0f, 1.0f, 1.0f, box);

	// Cache the vertex offsets to each object in the concatenated vertex buffer.
	mBoxVertexOffset = 0;

	// Cache the index count of each object.
	mBoxIndexCount = box.Indices.size();

	// Cache the starting index for each object in the concatenated index buffer.
	mBoxIndexOffset = 0;

	UINT totalVertexCount = box.Vertices.size();

	UINT totalIndexCount = mBoxIndexCount;

	//
	// Extract the vertex elements we are interested in and pack the
	// vertices of all the meshes into one vertex buffer.
	//

	std::vector<Vertex::Basic32> vertices(totalVertexCount);

	UINT k = 0;
	for (size_t i = 0; i < box.Vertices.size(); ++i, ++k)
	{
		vertices[k].Pos = box.Vertices[i].Position;
		vertices[k].Normal = box.Vertices[i].Normal;
		vertices[k].Tex = box.Vertices[i].TexC;
	}

	D3D11_BUFFER_DESC vbd{};
	vbd.Usage = D3D11_USAGE_IMMUTABLE;
	vbd.ByteWidth = sizeof(Vertex::Basic32) * totalVertexCount;
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = 0;
	vbd.MiscFlags = 0;
	D3D11_SUBRESOURCE_DATA vinitData{};
	vinitData.pSysMem = &vertices[0];
	HR(md3dDevice->CreateBuffer(&vbd, &vinitData, &mBoxVB));

	//
	// Pack the indices of all the meshes into one index buffer.
	//

	std::vector<UINT> indices;
	indices.insert(indices.end(), box.Indices.begin(), box.Indices.end());

	D3D11_BUFFER_DESC ibd{};
	ibd.Usage = D3D11_USAGE_IMMUTABLE;
	ibd.ByteWidth = sizeof(UINT) * totalIndexCount;
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibd.CPUAccessFlags = 0;
	ibd.MiscFlags = 0;
	D3D11_SUBRESOURCE_DATA iinitData{};
	iinitData.pSysMem = &indices[0];
	HR(md3dDevice->CreateBuffer(&ibd, &iinitData, &mBoxIB));
}

void Exercise_Chapter8_5::LoadTexture(wstring path, wstring name , wstring extention, int maxAnimFrame)
{
	image = new ScratchImage[maxAnimFrame];

	HR(CoInitializeEx(nullptr, COINITBASE_MULTITHREADED));

	for (int i = 1; i <= maxAnimFrame; i++)
	{
		wstring numStr = ((i < 100) ? (i < 10 ? TEXT("00") : TEXT("0")) : TEXT("")) + std::to_wstring(i);
		wstring fileStr = path + name + numStr + extention;

		LoadFromWICFile(fileStr.c_str(), WIC_FLAGS_NONE, nullptr, image[i - 1]);
	}

	CreateShaderResourceView(md3dDevice, image[0].GetImages(), image[0].GetImageCount(), image[0].GetMetadata(), &mTexture);

}

void Exercise_Chapter8_5::UpdateAnimation(float playRate)
{
	float frameInterval = playRate/ fireAnimMaxCount;
	fireAnimFrame += mTimer.DeltaTime();
	
	if (fireAnimFrame >= frameInterval)
	{
		fireAnimFrame = 0;
		CreateShaderResourceView(md3dDevice, image[fireAnimCount].GetImages(), image[fireAnimCount].GetImageCount(), image[fireAnimCount].GetMetadata(), &mTexture);
		++fireAnimCount;
		if (fireAnimCount == fireAnimMaxCount)	fireAnimCount = 0;
	}
}
