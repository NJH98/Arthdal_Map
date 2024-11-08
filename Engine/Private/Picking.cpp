#include "..\Public\Picking.h"

#include "GameInstance.h"

CPicking::CPicking(ID3D11Device * pDevice, ID3D11DeviceContext * pContext)
	: m_pDevice { pDevice }
	, m_pContext { pContext }
	, m_pGameInstance { CGameInstance::Get_Instance() }
{
	Safe_AddRef(m_pGameInstance);
	Safe_AddRef(m_pDevice);
	Safe_AddRef(m_pContext);
}

HRESULT CPicking::Initialize(HWND hWnd)
{
	m_hWnd = hWnd;
	_uint			iNumViewports = { 1 };

	D3D11_VIEWPORT	ViewportDesc{};

	m_pContext->RSGetViewports(&iNumViewports, &ViewportDesc);

	D3D11_TEXTURE2D_DESC	TextureDesc{};
	ZeroMemory(&TextureDesc, sizeof(D3D11_TEXTURE2D_DESC));

	TextureDesc.Width = m_iViewportWidth = _uint(ViewportDesc.Width);
	TextureDesc.Height = m_iViewportHeight = _uint(ViewportDesc.Height);
	TextureDesc.MipLevels = 1;
	TextureDesc.ArraySize = 1;
	TextureDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;

	TextureDesc.SampleDesc.Quality = 0;
	TextureDesc.SampleDesc.Count = 1;

	TextureDesc.Usage = D3D11_USAGE_STAGING;
	/* 추후에 어떤 용도로 바인딩 될 수 있는 View타입의 텍스쳐를 만들기위한 Texture2D입니까? */
	TextureDesc.BindFlags = 0;

	TextureDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ | D3D11_CPU_ACCESS_WRITE;
	TextureDesc.MiscFlags = 0;	

	if (FAILED(m_pDevice->CreateTexture2D(&TextureDesc, nullptr, &m_pPickDepthTexture)))
		return E_FAIL;

	return S_OK;
}

_bool CPicking::Picking(_float3 * pPickPos, _uint* pDepthNum)
{
	/*m_pContext->CopyResource(m_pPickDepthTexture, );*/

	if (FAILED(m_pGameInstance->Copy_RenderTarget(TEXT("Target_PickDepth"), m_pPickDepthTexture)))
		return false;

	POINT			ptMouse{};
	GetCursorPos(&ptMouse);
	ScreenToClient(m_hWnd, &ptMouse);

	if (ptMouse.x >= _long(m_iViewportWidth) ||
		ptMouse.x <= _long(0) ||
		ptMouse.y >= _long(m_iViewportHeight) ||
		ptMouse.y <= _long(0)
		)
		return false;

	_uint		iPixelIndex = ptMouse.y * m_iViewportWidth + ptMouse.x;

	D3D11_MAPPED_SUBRESOURCE		SubResource{};

	m_pContext->Map(m_pPickDepthTexture, 0, D3D11_MAP_READ, 0, &SubResource);

	_float		fProjZ = ((_float4*)SubResource.pData)[iPixelIndex].x;
	_bool		isPicked = static_cast<_bool>(((_float4*)SubResource.pData)[iPixelIndex].w);

	if (pDepthNum != nullptr) {
		// 뎁스 번호 받아오기
		*pDepthNum = static_cast<_uint>(((_float4*)SubResource.pData)[iPixelIndex].z);
	}

	m_pContext->Unmap(m_pPickDepthTexture, 0);

	if (false == isPicked)
		return false;

	_float4 vProjPos;

	/* 투영공간상의 위치. */
	vProjPos.x = ptMouse.x / (m_iViewportWidth * 0.5f) - 1.f;
	vProjPos.y = ptMouse.y / -(m_iViewportHeight * 0.5f) + 1.f;
	vProjPos.z = fProjZ;
	vProjPos.w = 1.f;

	/* 뷰공간상의 위치. */
	_vector	vViewPos = XMVector3TransformCoord(XMLoadFloat4(&vProjPos), m_pGameInstance->Get_Transform_Inverse_Matrix(CPipeLine::D3DTS_PROJ));

	_vector vWorldPos = XMVector3TransformCoord(vViewPos, m_pGameInstance->Get_Transform_Inverse_Matrix(CPipeLine::D3DTS_VIEW));

	XMStoreFloat3(pPickPos, vWorldPos);

	return true;
}

_vector CPicking::Compute_Height(_fvector vWorldPos, _fmatrix ViewMatrix, _fmatrix ProjMatrix)
{
	if (FAILED(m_pGameInstance->Copy_RenderTarget(TEXT("Target_Height"), m_pPickDepthTexture)))
		return XMVectorZero();

	_vector		vTargetProjPos = XMVector3TransformCoord(vWorldPos, ViewMatrix);
	vTargetProjPos = XMVector3TransformCoord(vTargetProjPos, ProjMatrix);

	/*-1 -> 0
	1 -> viewport.width

	1 -> 0
	-1 -> Viewport.height*/

	_float2		vTargetPos = _float2(XMVectorGetX(vTargetProjPos) * m_iViewportWidth * 0.5f + m_iViewportWidth * 0.5f,
		XMVectorGetY(vTargetProjPos) * m_iViewportHeight * -0.5f + m_iViewportHeight * 0.5f);


	_uint		iPixelIndex = _uint(vTargetPos.y) * m_iViewportWidth + _uint(vTargetPos.x);

	D3D11_MAPPED_SUBRESOURCE		SubResource{};

	m_pContext->Map(m_pPickDepthTexture, 0, D3D11_MAP_READ, 0, &SubResource);

	_float		fProjZ = ((_float4*)SubResource.pData)[iPixelIndex].x;
	_bool		isPicked = static_cast<_bool>(((_float4*)SubResource.pData)[iPixelIndex].w);

	m_pContext->Unmap(m_pPickDepthTexture, 0);

	if (false == isPicked)
		return XMVectorZero();

	_float4 vProjPos;

	/* 투영공간상의 위치. */
	vProjPos.x = vTargetPos.x / (m_iViewportWidth * 0.5f) - 1.f;
	vProjPos.y = vTargetPos.y / -(m_iViewportHeight * 0.5f) + 1.f;
	vProjPos.z = fProjZ;
	vProjPos.w = 1.f;

	/* 뷰공간상의 위치. */
	_vector	vViewPos = XMVector3TransformCoord(XMLoadFloat4(&vProjPos), XMMatrixInverse(nullptr, ProjMatrix));

	_vector vResultPos = XMVector3TransformCoord(vViewPos, XMMatrixInverse(nullptr, ViewMatrix));
	
	return vResultPos;
}

CPicking * CPicking::Create(ID3D11Device * pDevice, ID3D11DeviceContext * pContext, HWND hWnd)
{
	CPicking*		pInstance = new CPicking(pDevice, pContext);

	if (FAILED(pInstance->Initialize(hWnd)))
	{
		MSG_BOX(TEXT("Failed to Created : CPicking"));
		Safe_Release(pInstance);
	}

	return pInstance;
}


void CPicking::Free()
{
	__super::Free();


	Safe_Release(m_pGameInstance);
	Safe_Release(m_pContext);
	Safe_Release(m_pDevice);
	Safe_Release(m_pPickDepthTexture);
}
