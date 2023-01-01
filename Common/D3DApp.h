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
	//�ڽ� Ŭ�������� �ش� �Լ����� ������ �°� ����� �� �ֵ��� �����Լ� ����.
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
	HINSTANCE mhAppInst;    //���� ���α׷� �ν��Ͻ� �ڵ�
	HWND mhMainWnd;    //�� â �ڵ�
	bool mAppPaused;    //���� ���α׷��� �Ͻ������� �������� Ȯ��.
	bool mMinimized;    //�������α׷� �ּ�ȭ���� Ȯ��.
	bool mMaximized;    //���� ���α׷� �ִ�ȭ ���� Ȯ��.
	bool mResizing;    //����� ũ�� ������ �׵θ��� ��������� Ȯ��.
	UINT m4xMsaaQuality;    //4XMSAA ǰ������.

	//�ð� ������ ���� �������
	GameTimer mTimer;

	//D3D11��ġ
	ID3D11Device* md3dDevice;
	ID3D11DeviceContext* md3dImmediateContext;
	//��ȯ �罽
	IDXGISwapChain* mSwapChain;
	//����, ���Ľ� ���۸� ���� 2���� �ؽ�ó.
	ID3D11Texture2D* mDepthStencilBuffer;
	//���� Ÿ�� ��(2���� �ؽ�ó)
	ID3D11RenderTargetView* mRenderTargetView;
	//���� ���Ľ� ��
	ID3D11DepthStencilView* mDepthStencilView;
	//����Ʈ
	D3D11_VIEWPORT mScreenViewport;

	//â�� ����
	std::wstring mMainWndCaption;

	//������ ����
	D3D_DRIVER_TYPE md3dDriverType;

	//â�� Ŭ���̾�Ʈ ���� �ʱ� ũ��.(800x600 based)
	int mClientWidth;
	int mClientHeight;

	//4XMSAA ���� true.
	bool mEnable4xMsaa;

	//�޴� �߰��� �ش� ��������� ������ �޴��� �������ν� ó���� �� �ְ���.
	LPCWSTR mMenuName;

};




#endif // !D3DAPP_H