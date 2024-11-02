#include "..\Public\VIBuffer_Instancing.h"

#include "GameInstance.h"

CVIBuffer_Instancing::CVIBuffer_Instancing(ID3D11Device * pDevice, ID3D11DeviceContext * pContext)
	: CVIBuffer { pDevice, pContext }
{
}

CVIBuffer_Instancing::CVIBuffer_Instancing(const CVIBuffer_Instancing & Prototype)
	: CVIBuffer { Prototype }
	, m_InstanceBufferDesc { Prototype.m_InstanceBufferDesc }
	, m_InstanceInitialData { Prototype.m_InstanceInitialData }
	, m_iNumInstance { Prototype.m_iNumInstance }
	, m_iIndexCountPerInstance { Prototype.m_iIndexCountPerInstance }
	, m_iInstanceStride { Prototype.m_iInstanceStride }
	, m_vCenterPos{ Prototype.m_vCenterPos }
	, m_vPivotPos{ Prototype.m_vPivotPos }
	, m_vRange{ Prototype.m_vRange }
	, m_pInstanceVertices { Prototype.m_pInstanceVertices }
	, m_pSpeed { Prototype.m_pSpeed }
	, m_vLifeTime { Prototype.m_vLifeTime }
	, m_isLoop { Prototype.m_isLoop }

{
}

HRESULT CVIBuffer_Instancing::Initialize_Prototype(const INSTANCE_DESC& Desc)
{	
	m_iNumInstance = Desc.iNumInstance;
	m_vCenterPos = Desc.vCenter;
	m_vRange = Desc.vRange;
	m_vSize = Desc.vSize;
	m_vPivotPos = Desc.vPivot;
	m_vLifeTime = Desc.vLifeTime;
	m_isLoop = Desc.isLoop;

	m_pSpeed = new _float[m_iNumInstance];

	for (size_t i = 0; i < m_iNumInstance; i++)	
		m_pSpeed[i] = m_pGameInstance->Get_Random(Desc.vSpeed.x, Desc.vSpeed.y);
	

	return S_OK;
}

HRESULT CVIBuffer_Instancing::Initialize(void * pArg)
{
	if (FAILED(m_pDevice->CreateBuffer(&m_InstanceBufferDesc, &m_InstanceInitialData, &m_pVBInstance)))
		return E_FAIL;

	return S_OK;
}

HRESULT CVIBuffer_Instancing::Bind_Buffers()
{
	ID3D11Buffer*		pVertexBuffers[] = {
		m_pVB,
		m_pVBInstance, 
	};

	_uint		iVertexStrides[] = {
		m_iVertexStride,
		m_iInstanceStride,
	};

	_uint		iOffsets[] = {
		0,
		0, 
	};

	m_pContext->IASetVertexBuffers(0, m_iNumVertexBuffers, pVertexBuffers, iVertexStrides, iOffsets);
	m_pContext->IASetIndexBuffer(m_pIB, m_eIndexFormat, 0);
	m_pContext->IASetPrimitiveTopology(m_eTopology);

	return S_OK;
}

HRESULT CVIBuffer_Instancing::Render()
{
	m_pContext->DrawIndexedInstanced(m_iIndexCountPerInstance, m_iNumInstance, 0, 0, 0);

	return S_OK;
}


void CVIBuffer_Instancing::Free()
{
	__super::Free();

	if (false == m_isCloned)
	{
		Safe_Delete_Array(m_pSpeed);
		Safe_Delete_Array(m_pInstanceVertices);
	}

	Safe_Release(m_pVBInstance);
}

