#ifndef D3DUTIL_H
#define D3DUTIL_H

#if defined(DEBUG) || defined(_DEBUG)
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif

#include <string>
#include <d3d11.h>
#include <d3dcommon.h>
#include <DirectXMath.h>
#include <DirectXColors.h>
#include <cassert>
#include <ctime>
#include <algorithm>
#include <string>
#include <sstream>
#include <fstream>
#include <vector>
using namespace DirectX;



template<size_t Alignment>
class AlignedAllocationPolicy
{
public:
	static void* operator new(size_t size)
	{
		return _aligned_malloc(size, Alignment);
	}

	static void operator delete(void* memory)
	{
		_aligned_free(memory);
	}
};

//D3D 함수들이 돌려주는 HRESULT 형식의 반환값을 점검
//DXTrace를 사용하기 위해 dxerr 포함. 오류가 발생한 파일과 행번호를 보여주는 메시지 상자를 띄우는 함수.
//L#x는 매크로의 인수 토큰을 유니코드 문자열로 반환.
#if defined(DEBUG) | defined(_DEBUG)		
	#ifndef HR(x)												
	#define	HR(x)											\
	{														\
		HRESULT hr = (x);										\
		if (FAILED(hr))										\
		{													\
			LPWSTR output;									\
			FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, hr, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR) &output, 0, NULL);	\
			MessageBox(NULL, output, TEXT("ERROR"), MB_OK);	\
		}													\
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


#endif // !D3DUTIL_H