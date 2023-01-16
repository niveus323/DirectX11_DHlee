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

//D3D 함수들이 돌려주는 HRESULT 형식의 반환값을 점검
//DXTrace를 사용하기 위해 dxerr 포함. 오류가 발생한 파일과 행번호를 보여주는 메시지 상자를 띄우는 함수.
//L#x는 매크로의 인수 토큰을 유니코드 문자열로 반환.
// -> DXTrace는 DirectX SDK에 포함되는 함수이며 Windows SDK에는 포함되지 않음.
//FormatMessage로 운영체제 에러를 받아서 MessageBox를 만드는 방식으로 대체.
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