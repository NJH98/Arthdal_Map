#include "..\Public\Cell.h"
#include "GameInstance.h"
#ifdef _DEBUG
#include "VIBuffer_Cell.h"
#endif

CCell::CCell(ID3D11Device * pDevice, ID3D11DeviceContext * pContext)
	: m_pGameInstance{ CGameInstance::Get_Instance() }
	, m_pDevice { pDevice }
	, m_pContext { pContext }
{
	Safe_AddRef(m_pGameInstance);
	Safe_AddRef(m_pDevice);
	Safe_AddRef(m_pContext);
}

HRESULT CCell::Initialize(const _float3 * pPoints, _int iIndex)
{
	memcpy(m_vPoints, pPoints, sizeof(_float3) * POINT_END);

	m_iIndex = iIndex;

	m_iAreaIndex = m_pGameInstance->AreaIndexSet(m_vPoints[POINT_A]);

	if (m_pGameInstance->IsInRenderArea(m_iAreaIndex, CAreaManager::AREA_5X5)) {
		m_bIsRender = true;
	}

#ifdef _DEBUG
	m_pVIBuffer = CVIBuffer_Cell::Create(m_pDevice, m_pContext, m_vPoints);
	if (nullptr == m_pVIBuffer)
		return E_FAIL;
#endif

	return S_OK;
}

_bool CCell::Compare_Points(_fvector vSour, _fvector vDest)
{	
	if (true == XMVector3Equal(vSour, XMLoadFloat3(&m_vPoints[POINT_A])))
	{
		if (true == XMVector3Equal(vDest, XMLoadFloat3(&m_vPoints[POINT_B])))
			return true;		
		if (true == XMVector3Equal(vDest, XMLoadFloat3(&m_vPoints[POINT_C])))
			return true;
	}

	if (true == XMVector3Equal(vSour, XMLoadFloat3(&m_vPoints[POINT_B])))
	{
		if (true == XMVector3Equal(vDest, XMLoadFloat3(&m_vPoints[POINT_C])))
			return true;
		if (true == XMVector3Equal(vDest, XMLoadFloat3(&m_vPoints[POINT_A])))
			return true;
	}

	if (true == XMVector3Equal(vSour, XMLoadFloat3(&m_vPoints[POINT_C])))
	{
		if (true == XMVector3Equal(vDest, XMLoadFloat3(&m_vPoints[POINT_A])))
			return true;
		if (true == XMVector3Equal(vDest, XMLoadFloat3(&m_vPoints[POINT_B])))
			return true;
	}

	return false;
}

_bool CCell::isIn(_fvector vPosition, _int* pNeighborIndex)
{
	for (size_t i = 0; i < LINE_END; i++)
	{
		_vector		vSour, vDest;

		vSour = XMVector3Normalize(vPosition - XMLoadFloat3(&m_vPoints[i]));

		_vector		vLine = XMLoadFloat3(&m_vPoints[(i + 1) % 3]) - XMLoadFloat3(&m_vPoints[i]);
		vDest = XMVectorSet(XMVectorGetZ(vLine) * -1.f, 0.f, XMVectorGetX(vLine), 0.f);

		if (0 < XMVectorGetX(XMVector3Dot(vSour, vDest)))
		{
			*pNeighborIndex = m_iNeighborIndices[i];
			return false;
		}
			
	}

	return true;	
}

void CCell::Cell_Landing(CTransform* pTransform)
{
	Vector3 Pos = pTransform->Get_State(CTransform::STATE_POSITION);

	Vector3 PointA = m_vPoints[POINT_A];
	Vector3 PointB = m_vPoints[POINT_B];
	Vector3 PointC = m_vPoints[POINT_C];
	
	// 평면을 생성
	_vector Plane = XMPlaneFromPoints(PointA, PointB, PointC);

	// 평면 방정식의 계수를 추출
	_float a = XMVectorGetX(Plane);
	_float b = XMVectorGetY(Plane);
	_float c = XMVectorGetZ(Plane);
	_float d = XMVectorGetW(Plane);

	// y 값을 계산
	_float ComputeY =  (-a * Pos.x - c * Pos.z - d) / b;

	Pos.y = ComputeY;

	pTransform->Set_State(CTransform::STATE_POSITION, Pos);
}

#ifdef _DEBUG

HRESULT CCell::Render()
{
	m_pVIBuffer->Bind_Buffers();

	m_pVIBuffer->Render();

	return S_OK;
}
#endif

CCell * CCell::Create(ID3D11Device * pDevice, ID3D11DeviceContext * pContext, const _float3 * pPoints, _int iIndex)
{
	CCell*		pInstance = new CCell(pDevice, pContext);

	if (FAILED(pInstance->Initialize(pPoints, iIndex)))
	{
		MSG_BOX(TEXT("Failed to Created : CCell"));
		Safe_Release(pInstance);
	}

	return pInstance;
}


void CCell::Free()
{
	__super::Free();

#ifdef _DEBUG
	Safe_Release(m_pVIBuffer);
#endif

	Safe_Release(m_pContext);
	Safe_Release(m_pDevice);	
	Safe_Release(m_pGameInstance);
}
