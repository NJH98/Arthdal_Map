#include "..\Public\Instancing_Buffer.h"
#include "VertexBuffer.h"
#include "GameInstance.h"


CInstancing_Buffer::CInstancing_Buffer(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
}

void CInstancing_Buffer::ClearData()
{
	m_vecData.clear();
}

void CInstancing_Buffer::AddData(InstancingData& data)
{
	m_vecData.push_back(data);
}

void CInstancing_Buffer::PushData()
{
	const _uint32 dataCount = Get_Count();
	if (dataCount > m_iMaxCount)
		CreateBuffer(dataCount);

	D3D11_MAPPED_SUBRESOURCE SubResource;

	GET_DC->Map(_instanceBuffer->GetComPtr().Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &SubResource);
	{
		::memcpy(SubResource.pData, m_vecData.data(), sizeof(InstancingData) * dataCount);
		//VTXANIMMODELINSTANCE* pVertices = static_cast<VTXANIMMODELINSTANCE*>(SubResource.pData);

		//for (size_t i = 0; i < 10; i++)
		//{
		//	/*XMStoreFloat4(&pVertices[i].vRight, m_vecData[i].world.Right());
		//	XMStoreFloat4(&pVertices[i].vUp, m_vecData[i].world.Up());
		//	XMStoreFloat4(&pVertices[i].vLook, m_vecData[i].world.Forward());
		//	pVertices[i].iID = i;
		//	XMStoreFloat4(&pVertices[i].vLook, m_vecData[i].world.Translation());*/
		//}
	}
	GET_DC->Unmap(_instanceBuffer->GetComPtr().Get(), 0);

	_instanceBuffer->PushData();
}

void CInstancing_Buffer::CreateBuffer(_uint32 maxCount)
{
	m_iMaxCount = maxCount;
	_instanceBuffer = make_shared<VertexBuffer>();

	vector<InstancingData> temp(maxCount);
	_instanceBuffer->Create(temp, /*slot*/1, /*cpuWrite*/true);
}


HRESULT CInstancing_Buffer::Initialize()
{
	CreateBuffer();
	return S_OK;
}


CInstancing_Buffer* CInstancing_Buffer::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CInstancing_Buffer* pInstance = new CInstancing_Buffer(pDevice, pContext);

	if (FAILED(pInstance->Initialize()))
	{
		MSG_BOX(TEXT("Failed to Created : CInstancing_Buffer"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CInstancing_Buffer::Free()
{
	__super::Free();
}
