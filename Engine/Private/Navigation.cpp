#include "..\Public\Navigation.h"
#include "Cell.h"

#include "Shader.h"
#include "GameInstance.h"

_float4x4 CNavigation::m_WorldMatrix = {};

CNavigation::CNavigation(ID3D11Device * pDevice, ID3D11DeviceContext * pContext)
	: CComponent { pDevice, pContext }
{
}

CNavigation::CNavigation(const CNavigation & Prototype)
	: CComponent{ Prototype }
	, m_Cells { Prototype.m_Cells}

#ifdef _DEBUG
	, m_pShader { Prototype.m_pShader }
#endif
{
	for (auto& pCell : m_Cells)
		Safe_AddRef(pCell);

#ifdef _DEBUG
	Safe_AddRef(m_pShader);
#endif
}

HRESULT CNavigation::Initialize_Prototype(const _wstring& strNavigationDataFile)
{
	XMStoreFloat4x4(&m_WorldMatrix, XMMatrixIdentity());

	_ulong			dwByte = {};

	HANDLE			hFile = CreateFile(strNavigationDataFile.c_str(), GENERIC_READ, 0, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
	if (0 == hFile)
		return E_FAIL;

	_float3			vPoints[3];

	while (true)
	{
		ReadFile(hFile, vPoints, sizeof(_float3) * 3, &dwByte, nullptr);

		if (0 == dwByte)
			break;

		CCell*		pCell = CCell::Create(m_pDevice, m_pContext, vPoints, _int(m_Cells.size()));
		if (nullptr == pCell)
			return E_FAIL;

		m_Cells.emplace_back(pCell);
	}	
	
	CloseHandle(hFile);

	if (FAILED(SetUp_Neighbors()))
		return E_FAIL;

#ifdef _DEBUG

	m_pShader = CShader::Create(m_pDevice, m_pContext, TEXT("../Bin/ShaderFiles/Shader_Cell.hlsl"), VTXPOS::Elements, VTXPOS::iNumElements);
	if (nullptr == m_pShader)
		return E_FAIL;

#endif

	return S_OK;
}

HRESULT CNavigation::Initialize(void * pArg)
{
	if (nullptr == pArg)
		return S_OK;

	NAVIGATION_DESC*		pDesc = static_cast<NAVIGATION_DESC*>(pArg);

	m_iCurrentCellIndex = pDesc->iCurrentIndex;

	return S_OK;
}

void CNavigation::Update(_fmatrix TerrainWorldMatrix)
{
	XMStoreFloat4x4(&m_WorldMatrix, TerrainWorldMatrix);

	if (m_pGameInstance->Get_RenderAreaChange()) {
		for (auto& pCell : m_Cells) {
			pCell->Set_Render(m_pGameInstance->IsInRenderArea(pCell->Get_AreaIndex(), CAreaManager::AREA_9X9));
		}
	}
}

_bool CNavigation::isMove(_fvector vPosition)
{
	_vector		vLocalPos = XMVector3TransformCoord(vPosition, XMMatrixInverse(nullptr, XMLoadFloat4x4(&m_WorldMatrix)));

	_int			iNeighborIndex = { -1 };

	/* 원래 있던 삼각형 안에서 움직인거야. */
	if (true == m_Cells[m_iCurrentCellIndex]->isIn(vLocalPos, &iNeighborIndex))
	{
		return true;
	}

	/* 원래 있던 삼각형을 벗어난거야. */
	else
	{
		/* 나간쪽에 이웃이 있다라면. */
		if (-1 != iNeighborIndex)
		{
			while (true)
			{
				if (-1 == iNeighborIndex)
					return false;

				if (true == m_Cells[iNeighborIndex]->isIn(vLocalPos, &iNeighborIndex))
					break;
			}
			
			
			m_iCurrentCellIndex = iNeighborIndex;
			return true;
		}

		/* 나간쪽에 이웃이 없다라면. */
		else
			return false;
	}		
}

void CNavigation::Clear_Cell()
{
	for (auto& pCell : m_Cells)
		Safe_Release(pCell);
	m_Cells.clear();
}

void CNavigation::Delete_Cell(_uint iter)
{
	auto iterator = m_Cells.begin();
	for (_uint i = 0; i < iter; i++) {
		iterator++;
	}
	Safe_Release(*iterator);
	m_Cells.erase(iterator);

	_int Index = 0;
	for (auto& Cell : m_Cells) {
		Cell->Set_Index(Index);
		Index++;
	}
}

void CNavigation::Add_Cell(_float3 PointA, _float3 PointB, _float3 PointC)
{
	// 카메라 위치
	Vector3 CamPos = XMVectorSet(0.f, 1.f, 0.f, 0.f);
	// 점들의 위치
	Vector3 Pa = PointA;
	Vector3 Pb = PointB;
	Vector3 Pc = PointC;

	// 벡터 AB와 AC를 계산
	Vector3 AB = Pb - Pa;
	Vector3 AC = Pc - Pa;

	// 두 벡터의 외적을 계산하여 법선 벡터를 구한다
	Vector3 crossProduct = XMVector3Cross(AB, AC);
	_float dotProduct = XMVectorGetX(XMVector3Dot(crossProduct, CamPos));
	/*
	법선벡터 dotProduct
	양수	: 카메라가 삼각형의 앞면을 보고 있음
	0	: 카메라가 삼각형의 정확한 측면을 보고있음 ( 1 자로 보이거나 안보임 )
	음수	: 카메라가 삼각형의 뒷면을 보고 있음 
	*/

	_float3			vPoints[3] = {};

	if (dotProduct > 0) {
		// 시계방향
		vPoints[0] = PointA;
		vPoints[1] = PointB;
		vPoints[2] = PointC;
	}
	else if (dotProduct < 0) {
		// 반시계방향
		vPoints[0] = PointA;
		vPoints[2] = PointB;
		vPoints[1] = PointC;
	}
	else {
		return;
	}

	CCell* pCell = CCell::Create(m_pDevice, m_pContext, vPoints, _int(m_Cells.size()));
	if (nullptr == pCell)
		return;

	m_Cells.emplace_back(pCell);
}

void CNavigation::Add_Cell_NoneCheck(_float3 PointA, _float3 PointB, _float3 PointC)
{
	_float3			vPoints[3]{};
	vPoints[0] = PointA;
	vPoints[1] = PointB;
	vPoints[2] = PointC;

	CCell* pCell = CCell::Create(m_pDevice, m_pContext, vPoints, _int(m_Cells.size()));

	m_Cells.emplace_back(pCell);
}

void CNavigation::Add_Bin_Cell(CELL_DESC CellDesc)
{
	_float3			vPoints[3]{};
	vPoints[0] = CellDesc.PointA;
	vPoints[1] = CellDesc.PointB;
	vPoints[2] = CellDesc.PointC;

	CCell* pCell = CCell::Create(m_pDevice, m_pContext, vPoints, _int(m_Cells.size()));
	if (nullptr == pCell)
		return;
	pCell->Set_Bin_Neighbor(CCell::LINE_AB, CellDesc.NeighborIndex_AB);
	pCell->Set_Bin_Neighbor(CCell::LINE_BC, CellDesc.NeighborIndex_BC);
	pCell->Set_Bin_Neighbor(CCell::LINE_CA, CellDesc.NeighborIndex_CA);

	pCell->Set_Ride(CellDesc.IsRide);

	m_Cells.emplace_back(pCell);
}

#ifdef _DEBUG

HRESULT CNavigation::Render()
{

	if (FAILED(m_pShader->Bind_Matrix("g_ViewMatrix", &m_pGameInstance->Get_Transform_Float4x4(CPipeLine::D3DTS_VIEW))))
		return E_FAIL;
	if (FAILED(m_pShader->Bind_Matrix("g_ProjMatrix", &m_pGameInstance->Get_Transform_Float4x4(CPipeLine::D3DTS_PROJ))))
		return E_FAIL;

	_float4		vColor = -1 == m_iCurrentCellIndex ? _float4(0.f, 1.f, 0.f, 1.f) : _float4(1.f, 0.f, 0.f, 1.f);
	_float4x4	WorldMatrix = m_WorldMatrix;
	
	if (-1 != m_iCurrentCellIndex)
		WorldMatrix._42 += 0.1f;	

	if (-1 == m_iCurrentCellIndex)
	{
		for (auto& pCell : m_Cells) 
		{
			if (pCell->Get_Render() == false)
				continue;

			if (pCell->Get_PickCell()) {
				vColor = _float4(1.f, 0.f, 0.f, 1.f);
				WorldMatrix._42 += 0.1f;

				if (FAILED(m_pShader->Bind_Matrix("g_WorldMatrix", &WorldMatrix)))
					return E_FAIL;
			}
			else {
				vColor = _float4(0.f, 1.f, 0.f, 1.f);
				
				if (FAILED(m_pShader->Bind_Matrix("g_WorldMatrix", &m_WorldMatrix)))
					return E_FAIL;
			}

			m_pShader->Bind_RawValue("g_vColor", &vColor, sizeof(_float4));

			m_pShader->Begin(0);

			pCell->Render();
		}
		
	}
	else
	{
		m_Cells[m_iCurrentCellIndex]->Render();
	}



	return S_OK;
}

#endif

HRESULT CNavigation::SetUp_Neighbors()
{
	for (auto& pSourCell : m_Cells)
	{
		for (auto& pDestCell : m_Cells)
		{
			if (pSourCell == pDestCell)
				continue;

			if (true == pDestCell->Compare_Points(pSourCell->Get_Point(CCell::POINT_A), pSourCell->Get_Point(CCell::POINT_B)))
			{
				pSourCell->Set_Neighbor(CCell::LINE_AB, pDestCell);
			}

			if (true == pDestCell->Compare_Points(pSourCell->Get_Point(CCell::POINT_B), pSourCell->Get_Point(CCell::POINT_C)))
			{
				pSourCell->Set_Neighbor(CCell::LINE_BC, pDestCell);
			}

			if (true == pDestCell->Compare_Points(pSourCell->Get_Point(CCell::POINT_C), pSourCell->Get_Point(CCell::POINT_A)))
			{
				pSourCell->Set_Neighbor(CCell::LINE_CA, pDestCell);
			}
		}
	}

	return S_OK;
}

CNavigation * CNavigation::Create(ID3D11Device * pDevice, ID3D11DeviceContext * pContext, const _wstring & strNavigationDataFile)
{
	CNavigation*		pInstance = new CNavigation(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype(strNavigationDataFile)))
	{
		MSG_BOX(TEXT("Failed to Created : CNavigation"));
		Safe_Release(pInstance);
	}

	return pInstance;
}



CComponent * CNavigation::Clone(void * pArg)
{
	CNavigation*		pInstance = new CNavigation(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed to Cloned : CNavigation"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CNavigation::Free()
{
	__super::Free();

	for (auto& pCell : m_Cells)
		Safe_Release(pCell);

	m_Cells.clear();

#ifdef _DEBUG
	Safe_Release(m_pShader);
#endif
}
