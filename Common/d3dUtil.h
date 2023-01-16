#ifndef D3DUTIL_H
#define D3DUTIL_H

#include <vld.h>
#include <string>
#include <d3d11.h>
#include "d3dx11effect.h"
#include <d3dcompiler.h>
#include <DirectXMath.h>
#include <DirectXPackedVector.h>
#include <cassert>
#include <ctime>
#include <algorithm>
#include <string>
#include <sstream>
#include <fstream>
#include <vector>
#include "MathHelper.h"
#include "DDSTextureLoader11.h"
#include "LightHelper.h"
using namespace DirectX;

//D3D �Լ����� �����ִ� HRESULT ������ ��ȯ���� ����
//DXTrace�� ����ϱ� ���� dxerr ����. ������ �߻��� ���ϰ� ���ȣ�� �����ִ� �޽��� ���ڸ� ���� �Լ�.
//L#x�� ��ũ���� �μ� ��ū�� �����ڵ� ���ڿ��� ��ȯ.
// -> DXTrace�� DirectX SDK�� ���ԵǴ� �Լ��̸� Windows SDK���� ���Ե��� ����.
//FormatMessage�� �ü�� ������ �޾Ƽ� MessageBox�� ����� ������� ��ü.
#if defined(DEBUG) | defined(_DEBUG)
#ifndef HR(x)
#define HR(x)											\
{													\
	HRESULT hr = (x);									\
	if (FAILED(hr))									\
	{												\
		LPWSTR output;								\
		FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, hr, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR) &output, 0, NULL);\
		MessageBox(NULL, output, TEXT("ERROR"), MB_OK);	\
	}												\
}														
#endif // !HR(x)
#else
	#ifdef HR
	#define HR(x) (x)
	#endif // HR
#endif

#define ReleaseCOM(x) { if(x){ x->Release(); x = 0;	}	}
#define SafeDelete(x) { delete x; x = 0; }

#if defined(DEBUG) | defined(_DEBUG)
#ifndef DEBUGCOUT(str)
#define DEBUGCOUT(str)				\
{								\
	std::wstring wstr = TEXT("***");\
	wstr += str;					\
	OutputDebugString(wstr.c_str());\
}

#endif // !DEBUGCOUT
#endif // defined(DEBUG) | defined(_DEBUG)

// Order: left, right, bottom, top, near, far.
void ExtractFrustumPlanes(XMFLOAT4 planes[6], CXMMATRIX M);

namespace Colors
{
	XMGLOBALCONST XMVECTORF32 White = { 1.0f, 1.0f, 1.0f, 1.0f };
	XMGLOBALCONST XMVECTORF32 Black = { 0.0f, 0.0f, 0.0f, 1.0f };
	XMGLOBALCONST XMVECTORF32 Red = { 1.0f, 0.0f, 0.0f, 1.0f };
	XMGLOBALCONST XMVECTORF32 Green = { 0.0f, 1.0f, 0.0f, 1.0f };
	XMGLOBALCONST XMVECTORF32 Blue = { 0.0f, 0.0f, 1.0f, 1.0f };
	XMGLOBALCONST XMVECTORF32 Yellow = { 1.0f, 1.0f, 0.0f, 1.0f };
	XMGLOBALCONST XMVECTORF32 Cyan = { 0.0f, 1.0f, 1.0f, 1.0f };
	XMGLOBALCONST XMVECTORF32 Magenta = { 1.0f, 0.0f, 1.0f, 1.0f };

	XMGLOBALCONST XMVECTORF32 Silver = { 0.75f, 0.75f, 0.75f, 1.0f };
	XMGLOBALCONST XMVECTORF32 LightSteelBlue = { 0.69f, 0.77f, 0.87f, 1.0f };
}

class Convert
{
public:
	///<summary>
	/// Converts XMVECTOR to XMCOLOR, where XMVECTOR represents a color.
	///</summary>
	static inline XMCOLOR ToXmColor(FXMVECTOR v)
	{
		XMCOLOR dest;
		XMStoreColor(&dest, v);
		return dest;
	}

	///<summary>
	/// Converts XMVECTOR to XMFLOAT4, where XMVECTOR represents a color.
	///</summary>
	static inline XMFLOAT4 ToXmFloat4(FXMVECTOR v)
	{
		XMFLOAT4 dest;
		XMStoreFloat4(&dest, v);
		return dest;
	}

	static inline UINT ArgbToAbgr(UINT argb)
	{
		BYTE A = (argb >> 24) & 0xff;
		BYTE R = (argb >> 16) & 0xff;
		BYTE G = (argb >> 8) & 0xff;
		BYTE B = (argb >> 0) & 0xff;

		return (A << 24) | (B << 16) | (G << 8) | (R << 0);
	}
};
#endif // !D3DUTIL_H