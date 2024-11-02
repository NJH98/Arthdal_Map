#pragma once

#include "Base.h"

BEGIN(Engine)

/* 1. ID3D11Device, ID3D11DeviceContext라는 객체를 우선 생성한다. */
/* 2. IDXGISwapChain를 생성한다.(백버퍼(ID3D11Texture2D)도 같이 생성한거야.) */
/* 3. 백버퍼뷰를 생성한다. */
/* 4. 깊이버퍼 텍스쳐를 생성하고 깊이버퍼 뷰를 생성한다. */

class CGraphic_Device final : public CBase
{	
public:
	CGraphic_Device();
	virtual ~CGraphic_Device() = default;

public:
	ID3D11ShaderResourceView* Get_BackBuffer_SRV() const {
		return m_pBackBufferSRV;
	}

public:

	/* 그래픽 디바이스의 초기화. */
	HRESULT Initialize(HWND hWnd, _bool isWindowed, _uint iWinSizeX, _uint iWinSizeY,
		_Inout_ ID3D11Device** ppDevice, _Inout_ ID3D11DeviceContext** ppContext);

	/* 백버퍼를 지운다. */
	HRESULT Clear_BackBuffer_View(_float4 vClearColor);

	/* 깊이버퍼 + 스텐실버퍼를 지운다. */
	HRESULT Clear_DepthStencil_View();
	
	/* 후면 버퍼를 전면버퍼로 교체한다.(백버퍼를 화면에 직접 보여준다.) */
	HRESULT Present();

private:	
	// IDirect3DDevice9* == LPDIRECT3DDEVICE9 == ID3D11Device + ID3D11DeviceContext 

	/* 메모리 할당. (정점버퍼, 인덱스버퍼, 텍스쳐로드, 쉐이더객체를 생성한다. ) 컴객체의 생성과 관련된 역할 */
	/* 추가적으로 생성된 모든 스레드에서 사용하는데 전혀 문제가 없다. */
	ID3D11Device*				m_pDevice = { nullptr };
		
	/* 기능실행.(바인딩작업, 정점버퍼를 SetVertexBuffers(), SetIndexBuffer(), Apply() */
	/* 그린다. DrawIndexed() */
	/* 컨텍스트 객체를 생성해낸 스레드 외에 스레드에서는 사용해서는 안된다. */
	ID3D11DeviceContext*		m_pDeviceContext = { nullptr };

	/* 후면버퍼와 전면버퍼를 교체해가면서 화면에 보여주는 역할 */
	IDXGISwapChain*				m_pSwapChain = { nullptr };


	/* IDirect3DTexture9 */
	/* ID3D11Texture2D : 텍스쳐를 표현하는 사전객체 타입이다.
	why? 용도에 맞는 실질적으로 사용하기 위한 텍스쳐객체를 만들어내기위해.  */	

	/* ID3D11ShaderResourceView : 셰이더에 전달될 수 있는 텍스처 타입. */	
	/* ID3D11RenderTargetView : 렌더타겟용으로 사용될 수 있는 텍스처 타입. */
	/* ID3D11DepthStencilView : 깊이스텐실 버퍼로서 사용될 수 있는 타입.  */
	ID3D11RenderTargetView*		m_pBackBufferRTV = { nullptr };
	ID3D11ShaderResourceView*	m_pBackBufferSRV = { nullptr };
	ID3D11Texture2D*			m_pDepthTexture = { nullptr };
	ID3D11DepthStencilView*		m_pDepthStencilView = { nullptr };

private:
	/* 스왑체인에게 필수적으로 필요한 데이터는 백버퍼가 필요하여 백버퍼를 생성하기위한 정보를 던져준다. */
	/* 스왑체인을 만들었다 == 백버퍼(텍스쳐)가 생성된다. */
	HRESULT Ready_SwapChain(HWND hWnd, _bool isWindowed, _uint iWinCX, _uint iWinCY);
	HRESULT Ready_BackBufferRenderTargetView();	
	HRESULT Ready_DepthStencilView(_uint iWinCX, _uint iWinCY);

public:
	static CGraphic_Device* Create(HWND hWnd, _bool isWindowed, _uint iWinSizeX, _uint iWinSizeY, _Out_ ID3D11Device** ppDevice,
		_Out_ ID3D11DeviceContext** ppDeviceContextOut);
	virtual void Free() override;
};

END