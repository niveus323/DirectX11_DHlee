#include "d3dApp.h"
#include "GeometryGenerator.h"
#include "Effects.h"
#include "Vertex.h"
#include "RenderStates.h"
#include "Waves.h"

enum RenderOptions
{
	Lighting = 0,
	Textures = 1,
	TexturesAndFog = 2
};

class SubdivisionEffect : public Effect
{
public:
	SubdivisionEffect(ID3D11Device* device, const std::wstring& filename)
		: Effect(device, filename)
	{
		Light3Tech = mFX->GetTechniqueByName("Light3");
		Light3TexTech = mFX->GetTechniqueByName("Light3Tex");
		Light3TexAlphaClipTech = mFX->GetTechniqueByName("Light3TexAlphaClip");
		Light3FogTech = mFX->GetTechniqueByName("Light3Fog");
		Light3TexFogTech = mFX->GetTechniqueByName("Light3TexFog");
		Light3TexAlphaClipFogTech = mFX->GetTechniqueByName("Light3TexAlphaClipFog");

		WorldViewProj = mFX->GetVariableByName("gWorldViewProj")->AsMatrix();
		World = mFX->GetVariableByName("gWorld")->AsMatrix();
		WorldInvTranspose = mFX->GetVariableByName("gWorldInvTranspose")->AsMatrix();
		TexTransform = mFX->GetVariableByName("gTexTransform")->AsMatrix();
		EyePosW = mFX->GetVariableByName("gEyePosW")->AsVector();
		FogColor = mFX->GetVariableByName("gFogColor")->AsVector();
		FogStart = mFX->GetVariableByName("gFogStart")->AsScalar();
		FogRange = mFX->GetVariableByName("gFogRange")->AsScalar();
		DirLights = mFX->GetVariableByName("gDirLights");
		Mat = mFX->GetVariableByName("gMaterial");
		DiffuseMap = mFX->GetVariableByName("gDiffuseMap")->AsShaderResource();
	}
	~SubdivisionEffect() {};

	void SetWorldViewProj(CXMMATRIX M) { WorldViewProj->SetMatrix(reinterpret_cast<const float*>(&M)); }
	void SetWorld(CXMMATRIX M) { World->SetMatrix(reinterpret_cast<const float*>(&M)); }
	void SetWorldInvTranspose(CXMMATRIX M) { WorldInvTranspose->SetMatrix(reinterpret_cast<const float*>(&M)); }
	void SetTexTransform(CXMMATRIX M) { TexTransform->SetMatrix(reinterpret_cast<const float*>(&M)); }
	void SetEyePosW(const XMFLOAT3& v) { EyePosW->SetRawValue(&v, 0, sizeof(XMFLOAT3)); }
	void SetFogColor(const FXMVECTOR v) { FogColor->SetFloatVector(reinterpret_cast<const float*>(&v)); }
	void SetFogStart(float f) { FogStart->SetFloat(f); }
	void SetFogRange(float f) { FogRange->SetFloat(f); }
	void SetDirLights(const DirectionalLight* lights) { DirLights->SetRawValue(lights, 0, 3 * sizeof(DirectionalLight)); }
	void SetMaterial(const Material& mat) { Mat->SetRawValue(&mat, 0, sizeof(Material)); }
	void SetDiffuseMap(ID3D11ShaderResourceView* tex) { DiffuseMap->SetResource(tex); }

	ID3DX11EffectTechnique* Light3Tech;
	ID3DX11EffectTechnique* Light3TexTech;
	ID3DX11EffectTechnique* Light3TexAlphaClipTech;
	ID3DX11EffectTechnique* Light3FogTech;
	ID3DX11EffectTechnique* Light3TexFogTech;
	ID3DX11EffectTechnique* Light3TexAlphaClipFogTech;

	ID3DX11EffectMatrixVariable* WorldViewProj;
	ID3DX11EffectMatrixVariable* World;
	ID3DX11EffectMatrixVariable* WorldInvTranspose;
	ID3DX11EffectMatrixVariable* TexTransform;
	ID3DX11EffectVectorVariable* EyePosW;
	ID3DX11EffectVectorVariable* FogColor;
	ID3DX11EffectScalarVariable* FogStart;
	ID3DX11EffectScalarVariable* FogRange;
	ID3DX11EffectVariable* DirLights;
	ID3DX11EffectVariable* Mat;

	ID3DX11EffectShaderResourceVariable* DiffuseMap;
};

class Exercise_Chapter_11_2 : public D3DApp
{
public:

public:
	Exercise_Chapter_11_2(HINSTANCE hInstance);
	~Exercise_Chapter_11_2();

	bool Init();
	void OnResize();
	void UpdateScene(float dt);
	void DrawScene();

	void OnMouseDown(WPARAM btnState, int x, int y);
	void OnMouseUp(WPARAM btnState, int x, int y);
	void OnMouseMove(WPARAM btnState, int x, int y);

private:
	void BuildGeometryBuffers();

private:
	DirectionalLight mDirLights[3];

	XMFLOAT4X4 mView;
	XMFLOAT4X4 mProj;

	RenderOptions mRenderOptions;

	XMFLOAT3 mEyePosW;

	float mTheta;
	float mPhi;
	float mRadius;

	POINT mLastMousePos;

	SubdivisionEffect* subdivisionFX; 
	ID3D11Buffer* mSphereVB;
	ID3D11Buffer* mSphereIB;

	Material mSphereMat;

	XMFLOAT4X4 mSphereWorld;
};

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE prevInstance,
	PSTR cmdLine, int showCmd)
{
	// Enable run-time memory check for debug builds.
#if defined(DEBUG) | defined(_DEBUG)
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

	Exercise_Chapter_11_2 theApp(hInstance);

	if (!theApp.Init())
		return 0;

	return theApp.Run();
}

Exercise_Chapter_11_2::Exercise_Chapter_11_2(HINSTANCE hInstance)
	: D3DApp(hInstance), mEyePosW(0.0f, 0.0f, 0.0f), mRenderOptions(RenderOptions::TexturesAndFog),
	mTheta(1.3f * MathHelper::Pi), mPhi(0.4f * MathHelper::Pi), mRadius(80.0f)
{
	mMainWndCaption = TEXT("???????? 11?? 2??");
	mEnable4xMsaa = true;

	mLastMousePos.x = 0;
	mLastMousePos.y = 0;

	XMMATRIX I = XMMatrixIdentity();
	XMStoreFloat4x4(&mView, I);
	XMStoreFloat4x4(&mProj, I);

	mDirLights[0].Ambient = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
	mDirLights[0].Diffuse = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	mDirLights[0].Specular = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	mDirLights[0].Direction = XMFLOAT3(0.57735f, -0.57735f, 0.57735f);

	mDirLights[1].Ambient = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	mDirLights[1].Diffuse = XMFLOAT4(0.20f, 0.20f, 0.20f, 1.0f);
	mDirLights[1].Specular = XMFLOAT4(0.25f, 0.25f, 0.25f, 1.0f);
	mDirLights[1].Direction = XMFLOAT3(-0.57735f, -0.57735f, 0.57735f);

	mDirLights[2].Ambient = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	mDirLights[2].Diffuse = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
	mDirLights[2].Specular = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	mDirLights[2].Direction = XMFLOAT3(0.0f, -0.707f, -0.707f);

	XMMATRIX sphereScale = XMMatrixScaling(2.0f, 2.0f, 2.0f);
	XMStoreFloat4x4(&mSphereWorld, sphereScale);

	mSphereMat.Ambient = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	mSphereMat.Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 0.5f);
	mSphereMat.Specular = XMFLOAT4(0.8f, 0.8f, 0.8f, 32.0f);
}

Exercise_Chapter_11_2::~Exercise_Chapter_11_2()
{
	md3dImmediateContext->ClearState();
	ReleaseCOM(mSphereVB);
	ReleaseCOM(mSphereIB);

	Effects::DestroyAll();
	InputLayouts::DestroyAll();
	RenderStates::DestroyAll();
	SafeDelete(subdivisionFX);
}

bool Exercise_Chapter_11_2::Init()
{
	if (!D3DApp::Init())
		return false;

	// Must init Effects first since InputLayouts depend on shader signatures.
	Effects::InitAll(md3dDevice, TEXT("Basic.fxo"));
	Effects::InitTreeFX(md3dDevice, TEXT("TreeSprite.fxo"));

	subdivisionFX = new SubdivisionEffect(md3dDevice, TEXT("../FX/Subdivision.fxo"));

	BuildGeometryBuffers();

	InputLayouts::InitAll(md3dDevice);
	RenderStates::InitAll(md3dDevice);

	return true;
}

void Exercise_Chapter_11_2::OnResize()
{
	D3DApp::OnResize();

	XMMATRIX P = XMMatrixPerspectiveFovLH(0.25f * MathHelper::Pi, AspectRatio(), 1.0f, 1000.0f);
	XMStoreFloat4x4(&mProj, P);
}

void Exercise_Chapter_11_2::UpdateScene(float dt)
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

	//
	// Switch the render mode based in key input.
	//
	if (GetAsyncKeyState('1') & 0x8000)
		mRenderOptions = RenderOptions::Lighting;

	if (GetAsyncKeyState('2') & 0x8000)
		mRenderOptions = RenderOptions::Textures;

	if (GetAsyncKeyState('3') & 0x8000)
		mRenderOptions = RenderOptions::TexturesAndFog;
}

void Exercise_Chapter_11_2::DrawScene()
{
	md3dImmediateContext->ClearRenderTargetView(mRenderTargetView, reinterpret_cast<const float*>(&Colors::Silver));
	md3dImmediateContext->ClearDepthStencilView(mDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	float blendFactor[] = { 0.0f, 0.0f, 0.0f, 0.0f };

	XMMATRIX view = XMLoadFloat4x4(&mView);
	XMMATRIX proj = XMLoadFloat4x4(&mProj);
	XMMATRIX viewProj = view * proj;

	md3dImmediateContext->IASetInputLayout(InputLayouts::Basic32);
	md3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	UINT stride = sizeof(Vertex::Basic32);
	UINT offset = 0;

	// FX ????
	subdivisionFX->SetDirLights(mDirLights);
	subdivisionFX->SetEyePosW(mEyePosW);
	subdivisionFX->SetFogColor(Colors::Silver);
	subdivisionFX->SetFogStart(15.0f);
	subdivisionFX->SetFogRange(75.0f);

	// Technique ????
	ID3DX11EffectTechnique* activeTech{};

	switch (mRenderOptions)
	{
	case RenderOptions::Lighting:
		activeTech = subdivisionFX->Light3Tech;
		break;
	case RenderOptions::Textures:
		activeTech = subdivisionFX->Light3TexTech;
		break;
	case RenderOptions::TexturesAndFog:
		activeTech = subdivisionFX->Light3TexFogTech;
		break;
	}

	D3DX11_TECHNIQUE_DESC techDesc;

	//
	// Draw the sphere.
	// 

	activeTech->GetDesc(&techDesc);
	for (UINT p = 0; p < techDesc.Passes; ++p)
	{
		md3dImmediateContext->IASetVertexBuffers(0, 1, &mSphereVB, &stride, &offset);
		md3dImmediateContext->IASetIndexBuffer(mSphereIB, DXGI_FORMAT_R32_UINT, 0);

		// Set per object constants.
		XMMATRIX world = XMLoadFloat4x4(&mSphereWorld);
		XMMATRIX worldInvTranspose = MathHelper::InverseTranspose(world);
		XMMATRIX worldViewProj = world * view * proj;

		subdivisionFX->SetWorld(world);
		subdivisionFX->SetWorldInvTranspose(worldInvTranspose);
		subdivisionFX->SetWorldViewProj(worldViewProj);
		subdivisionFX->SetTexTransform(XMMatrixIdentity());
		subdivisionFX->SetMaterial(mSphereMat);

		md3dImmediateContext->RSSetState(RenderStates::WireframeRS);
		activeTech->GetPassByIndex(p)->Apply(0, md3dImmediateContext);
		md3dImmediateContext->DrawIndexed(60, 0, 0);

		md3dImmediateContext->RSSetState(0);
	}

	HR(mSwapChain->Present(0, 0));
}

void Exercise_Chapter_11_2::OnMouseDown(WPARAM btnState, int x, int y)
{
	mLastMousePos.x = x;
	mLastMousePos.y = y;

	SetCapture(mhMainWnd);
}

void Exercise_Chapter_11_2::OnMouseUp(WPARAM btnState, int x, int y)
{
	ReleaseCapture();
}

void Exercise_Chapter_11_2::OnMouseMove(WPARAM btnState, int x, int y)
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
		float dx = 0.1f * static_cast<float>(x - mLastMousePos.x);
		float dy = 0.1f * static_cast<float>(y - mLastMousePos.y);

		// Update the camera radius based on input.
		mRadius += dx - dy;

		// Restrict the radius.
		mRadius = MathHelper::Clamp(mRadius, 20.0f, 500.0f);
	}

	mLastMousePos.x = x;
	mLastMousePos.y = y;
}

void Exercise_Chapter_11_2::BuildGeometryBuffers()
{
	GeometryGenerator::MeshData sphere;

	// Create icosahedron, which we will tessellate into a sphere.
	GeometryGenerator geoGen;
	geoGen.CreateGeosphere(1.0f, 0, sphere);

	//
	// Extract the vertex elements we are interested in and pack the
	// vertices of all the meshes into one vertex buffer.
	//

	std::vector<Vertex::Basic32> vertices(sphere.Vertices.size());

	for (UINT i = 0; i < sphere.Vertices.size(); ++i)
	{
		vertices[i].Pos = sphere.Vertices[i].Position;
		vertices[i].Normal = sphere.Vertices[i].Normal;
		vertices[i].Tex = sphere.Vertices[i].TexC;
	}

	D3D11_BUFFER_DESC vbd{};
	vbd.Usage = D3D11_USAGE_IMMUTABLE;
	vbd.ByteWidth = sizeof(Vertex::Basic32) * sphere.Vertices.size();
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = 0;
	vbd.MiscFlags = 0;
	D3D11_SUBRESOURCE_DATA vinitData{};
	vinitData.pSysMem = &vertices[0];
	HR(md3dDevice->CreateBuffer(&vbd, &vinitData, &mSphereVB));

	//
	// Pack the indices of all the meshes into one index buffer.
	//

	D3D11_BUFFER_DESC ibd{};
	ibd.Usage = D3D11_USAGE_IMMUTABLE;
	ibd.ByteWidth = sizeof(UINT) * sphere.Indices.size();
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibd.CPUAccessFlags = 0;
	ibd.MiscFlags = 0;
	D3D11_SUBRESOURCE_DATA iinitData{};
	iinitData.pSysMem = &sphere.Indices[0];
	HR(md3dDevice->CreateBuffer(&ibd, &iinitData, &mSphereIB));
}