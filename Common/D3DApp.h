#pragma once
#ifndef D3DAPP_H
#define D3DAPP_H

#include "GameTimer.h"
#include "d3dUtil.h"

class D3DApp
{
public:
	D3DApp(HINSTANCE hInstance);
	virtual ~D3DApp();

	HINSTANCE AppInst() const;
	HWND MainWnd() const;
	float AspectRatio() const;

	int Run();

	//Framework methods
	//자식 클래스에서 해당 함수들을 목적에 맞게 사용할 수 있도록 가상함수 적용.
	virtual bool Init();
	virtual void OnResize();
	virtual void UpdateScene(float dt) = 0;
	virtual void DrawScene() = 0;
	virtual LRESULT MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);


	//Mouse Input
	virtual void OnMouseDown(WPARAM btnState, int x, int y) {}
	virtual void OnMouseUp(WPARAM btnState, int x, int y) {}
	virtual void OnMouseMove(WPARAM btnState, int x, int y) {}

protected:
	bool InitMainWindow();
	bool InitDirect3D();
	void CalculateFrameStats();

protected:
	HINSTANCE mhAppInst;    //응용 프로그램 인스턴스 핸들
	HWND mhMainWnd;    //주 창 핸들
	bool mAppPaused;    //응용 프로그램이 일시정지된 상태인지 확인.
	bool mMinimized;    //응용프로그램 최소화상태 확인.
	bool mMaximized;    //응용 프로그램 최대화 상태 확인.
	bool mResizing;    //사용자 크기 조정용 테두리를 사용중인지 확인.
	UINT m4xMsaaQuality;    //4XMSAA 품질수준.

	//시간 측정을 위한 멤버변수
	GameTimer mTimer;

	//D3D11장치
	ID3D11Device* md3dDevice;
	ID3D11DeviceContext* md3dImmediateContext;
	//교환 사슬
	IDXGISwapChain* mSwapChain;
	//깊이, 스탠실 버퍼를 위한 2차원 텍스처.
	ID3D11Texture2D* mDepthStencilBuffer;
	//렌더 타겟 뷰(2차원 텍스처)
	ID3D11RenderTargetView* mRenderTargetView;
	//깊이 스탠실 뷰
	ID3D11DepthStencilView* mDepthStencilView;
	//뷰포트
	D3D11_VIEWPORT mScreenViewport;

	//창의 제목
	std::wstring mMainWndCaption;

	//구동기 종류
	D3D_DRIVER_TYPE md3dDriverType;

	//창의 클라이언트 영역 초기 크기.(800x600 based)
	int mClientWidth;
	int mClientHeight;

	//4XMSAA 사용시 true.
	bool mEnable4xMsaa;

	//메뉴 추가시 해당 멤버변수에 생성한 메뉴를 넣음으로써 처리할 수 있게함.
	LPCWSTR mMenuName;

};




#endif // !D3DAPP_H