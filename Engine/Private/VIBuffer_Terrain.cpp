#include "..\Public\VIBuffer_Terrain.h"
#include "GameInstance.h"

#include "QuadTree.h"

CVIBuffer_Terrain::CVIBuffer_Terrain(ID3D11Device * pDevice, ID3D11DeviceContext * pContext)
	: CVIBuffer { pDevice, pContext }
{
}

CVIBuffer_Terrain::CVIBuffer_Terrain(const CVIBuffer_Terrain & Prototype)
	: CVIBuffer{ Prototype }
	, m_iNumVerticesX { Prototype.m_iNumVerticesX }
	, m_iNumVerticesZ { Prototype.m_iNumVerticesZ }
	, m_pQuadTree { Prototype.m_pQuadTree}

{
	Safe_AddRef(m_pQuadTree);
}

HRESULT CVIBuffer_Terrain::Initialize_Prototype(const _tchar* pHeightMapFilePath)
{
//	_ulong			dwByte = {};
//
//	HANDLE			hFile = CreateFile(pHeightMapFilePath, GENERIC_READ, 0, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
//	if (0 == hFile)
//		return E_FAIL;
//
//	BITMAPFILEHEADER			fh{};
//	BITMAPINFOHEADER			ih{};
//
//	ReadFile(hFile, &fh, sizeof fh, &dwByte, nullptr);
//	ReadFile(hFile, &ih, sizeof ih, &dwByte, nullptr);
//	m_iNumVerticesX = ih.biWidth;
//	m_iNumVerticesZ = ih.biHeight;
//
//	_uint*						pPixel = new _uint[m_iNumVerticesX * m_iNumVerticesZ];
//	ReadFile(hFile, pPixel, sizeof(_uint) * m_iNumVerticesX * m_iNumVerticesZ, &dwByte, nullptr);
//
//	CloseHandle(hFile); 
//
//	m_iNumVertexBuffers = 1;
//	m_iNumVertices = m_iNumVerticesX * m_iNumVerticesZ;
//	m_iVertexStride = sizeof(VTXNORTEX);
//	m_iNumIndices = (m_iNumVerticesX - 1) * (m_iNumVerticesZ - 1) * 2 * 3;
//	m_iIndexStride = 4;
//	m_eIndexFormat = DXGI_FORMAT_R32_UINT;
//	m_eTopology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
//	
//#pragma region VERTEX_BUFFER
//	/* 정점버퍼에 채워줄 값들을 만들기위해서 임시적으로 공간을 할당한다. */
//	VTXNORTEX*			pVertices = new VTXNORTEX[m_iNumVertices];
//	ZeroMemory(pVertices, sizeof(VTXNORTEX) * m_iNumVertices);
//
//	m_pVertexPositions = new _float3[m_iNumVertices];
//	ZeroMemory(m_pVertexPositions, sizeof(_float3) * m_iNumVertices);
//
//	for (_uint i = 0; i < m_iNumVerticesZ; i++)
//	{
//		for (_uint j = 0; j < m_iNumVerticesX; j++)
//		{
//			_uint			iIndex = i * m_iNumVerticesX + j;
//
//			pVertices[iIndex].vPosition = m_pVertexPositions[iIndex] = _float3(_float(j), (pPixel[iIndex] & 0x000000ff) / 10.0f, _float(i));
//			pVertices[iIndex].vNormal = _float3(0.f, 0.f, 0.f);
//			pVertices[iIndex].vTexcoord = _float2(j / (m_iNumVerticesX - 1.f), i / (m_iNumVerticesZ - 1.f));
//		}
//	}
//#pragma endregion
//
//#pragma region INDEX_BUFFER
//	
//	_uint*		pIndices = new _uint[m_iNumIndices];
//	ZeroMemory(pIndices, sizeof(_uint) * m_iNumIndices);
//
//	_uint		iNumIndices = { 0 };
//
//	for (size_t i = 0; i < m_iNumVerticesZ - 1; i++)
//	{
//		for (size_t j = 0; j < m_iNumVerticesX - 1; j++)
//		{
//			_uint			iIndex = _uint(i) * m_iNumVerticesX + _uint(j);
//			
//			_uint			iIndices[] = {
//				iIndex + m_iNumVerticesX, 
//				iIndex + m_iNumVerticesX + 1,
//				iIndex + 1,
//				iIndex
//			};
//
//			_vector		vSourDir, vDestDir, vNormal;
//
//			pIndices[iNumIndices++] = iIndices[0];
//			pIndices[iNumIndices++] = iIndices[1];
//			pIndices[iNumIndices++] = iIndices[2];
//
//			vSourDir = XMLoadFloat3(&pVertices[iIndices[1]].vPosition) - XMLoadFloat3(&pVertices[iIndices[0]].vPosition);
//			vDestDir = XMLoadFloat3(&pVertices[iIndices[2]].vPosition) - XMLoadFloat3(&pVertices[iIndices[1]].vPosition);
//			vNormal = XMVector3Normalize(XMVector3Cross(vSourDir, vDestDir));
//
//			XMStoreFloat3(&pVertices[iIndices[0]].vNormal, XMLoadFloat3(&pVertices[iIndices[0]].vNormal) + vNormal);
//			XMStoreFloat3(&pVertices[iIndices[1]].vNormal, XMLoadFloat3(&pVertices[iIndices[1]].vNormal) + vNormal);
//			XMStoreFloat3(&pVertices[iIndices[2]].vNormal, XMLoadFloat3(&pVertices[iIndices[2]].vNormal) + vNormal);
//			
//			pIndices[iNumIndices++] = iIndices[0];
//			pIndices[iNumIndices++] = iIndices[2];
//			pIndices[iNumIndices++] = iIndices[3];
//
//			vSourDir = XMLoadFloat3(&pVertices[iIndices[2]].vPosition) - XMLoadFloat3(&pVertices[iIndices[0]].vPosition);
//			vDestDir = XMLoadFloat3(&pVertices[iIndices[3]].vPosition) - XMLoadFloat3(&pVertices[iIndices[2]].vPosition);
//			vNormal = XMVector3Normalize(XMVector3Cross(vSourDir, vDestDir));
//
//			XMStoreFloat3(&pVertices[iIndices[0]].vNormal, XMLoadFloat3(&pVertices[iIndices[0]].vNormal) + vNormal);
//			XMStoreFloat3(&pVertices[iIndices[2]].vNormal, XMLoadFloat3(&pVertices[iIndices[2]].vNormal) + vNormal);
//			XMStoreFloat3(&pVertices[iIndices[3]].vNormal, XMLoadFloat3(&pVertices[iIndices[3]].vNormal) + vNormal);
//		}
//	}
//
//	for (size_t i = 0; i < m_iNumVertices; i++)
//		XMStoreFloat3(&pVertices[i].vNormal, XMVector3Normalize(XMLoadFloat3(&pVertices[i].vNormal)));	
//#pragma endregion
//
//	ZeroMemory(&m_BufferDesc, sizeof m_BufferDesc);
//	m_BufferDesc.ByteWidth = m_iVertexStride * m_iNumVertices;
//	m_BufferDesc.Usage = D3D11_USAGE_DEFAULT; /* 정적버퍼로 생성한다. */
//	m_BufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
//	m_BufferDesc.CPUAccessFlags = 0;
//	m_BufferDesc.MiscFlags = 0;
//	m_BufferDesc.StructureByteStride = m_iVertexStride;
//
//	ZeroMemory(&m_InitialData, sizeof m_InitialData);
//	m_InitialData.pSysMem = pVertices;
//
//	/* 정점버퍼를 생성한다. */
//	if (FAILED(__super::Create_Buffer(&m_pVB)))
//		return E_FAIL;
//
//	/* 인덱스버퍼의 내용을 채워주곡 */
//	ZeroMemory(&m_BufferDesc, sizeof m_BufferDesc);
//	m_BufferDesc.ByteWidth = m_iIndexStride * m_iNumIndices;
//	m_BufferDesc.Usage = D3D11_USAGE_DYNAMIC; /* 정적버퍼로 생성한다. */
//	m_BufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
//	m_BufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
//	m_BufferDesc.MiscFlags = 0;
//	m_BufferDesc.StructureByteStride = m_iIndexStride;
//
//	ZeroMemory(&m_InitialData, sizeof m_InitialData);
//	m_InitialData.pSysMem = pIndices;
//
//	/* 인덱스버퍼를 생성한다. */
//	if (FAILED(__super::Create_Buffer(&m_pIB)))
//		return E_FAIL;
//
//	Safe_Delete_Array(pIndices);
//	Safe_Delete_Array(pVertices);
//	Safe_Delete_Array(pPixel);
//
//	m_pQuadTree = CQuadTree::Create(m_iNumVerticesX * m_iNumVerticesZ - m_iNumVerticesX, 
//		m_iNumVerticesX * m_iNumVerticesZ - 1, 
//		m_iNumVerticesX - 1, 
//		0);
//
//	m_pQuadTree->Make_Neighbors();

	return S_OK;
}

HRESULT CVIBuffer_Terrain::Initialize(void * pArg)
{
	if (pArg != nullptr) 
	{
		_float2 TerrainXZ = *(_float2*)(pArg);

		m_iNumVerticesX = _uint(TerrainXZ.x);
		m_iNumVerticesZ = _uint(TerrainXZ.y);

		if (m_iNumVerticesX == 0 ||
			m_iNumVerticesZ == 0) {
		
			m_iNumVerticesX = 100;
			m_iNumVerticesZ = 100;
		}
	}
	else 
	{
		m_iNumVerticesX = 100;
		m_iNumVerticesZ = 100;
	}

	_uint* pPixel = new _uint[m_iNumVerticesX * m_iNumVerticesZ];

	m_iNumVertexBuffers = 1;
	m_iNumVertices = m_iNumVerticesX * m_iNumVerticesZ;
	m_iVertexStride = sizeof(VTXNORTEX);
	m_iNumIndices = (m_iNumVerticesX - 1) * (m_iNumVerticesZ - 1) * 2 * 3;
	m_iIndexStride = 4;
	m_eIndexFormat = DXGI_FORMAT_R32_UINT;
	m_eTopology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

#pragma region VERTEX_BUFFER
	/* 정점버퍼에 채워줄 값들을 만들기위해서 임시적으로 공간을 할당한다. */
	VTXNORTEX* pVertices = new VTXNORTEX[m_iNumVertices];
	ZeroMemory(pVertices, sizeof(VTXNORTEX) * m_iNumVertices);

	m_pVertexPositions = new _float3[m_iNumVertices];
	ZeroMemory(m_pVertexPositions, sizeof(_float3) * m_iNumVertices);

	for (_uint i = 0; i < m_iNumVerticesZ; i++)
	{
		for (_uint j = 0; j < m_iNumVerticesX; j++)
		{
			_uint			iIndex = i * m_iNumVerticesX + j;

			pVertices[iIndex].vPosition = m_pVertexPositions[iIndex] = _float3(_float(j), 0.f, _float(i));
			pVertices[iIndex].vNormal = _float3(0.f, 0.f, 0.f);
			pVertices[iIndex].vTexcoord = _float2(j / (m_iNumVerticesX - 1.f), i / (m_iNumVerticesZ - 1.f));
		}
	}
#pragma endregion

#pragma region INDEX_BUFFER

	_uint* pIndices = new _uint[m_iNumIndices];
	ZeroMemory(pIndices, sizeof(_uint) * m_iNumIndices);

	_uint		iNumIndices = { 0 };

	for (size_t i = 0; i < m_iNumVerticesZ - 1; i++)
	{
		for (size_t j = 0; j < m_iNumVerticesX - 1; j++)
		{
			_uint			iIndex = _uint(i) * m_iNumVerticesX + _uint(j);

			_uint			iIndices[] = {
				iIndex + m_iNumVerticesX,
				iIndex + m_iNumVerticesX + 1,
				iIndex + 1,
				iIndex
			};

			_vector		vSourDir, vDestDir, vNormal;

			pIndices[iNumIndices++] = iIndices[0];
			pIndices[iNumIndices++] = iIndices[1];
			pIndices[iNumIndices++] = iIndices[2];

			vSourDir = XMLoadFloat3(&pVertices[iIndices[1]].vPosition) - XMLoadFloat3(&pVertices[iIndices[0]].vPosition);
			vDestDir = XMLoadFloat3(&pVertices[iIndices[2]].vPosition) - XMLoadFloat3(&pVertices[iIndices[1]].vPosition);
			vNormal = XMVector3Normalize(XMVector3Cross(vSourDir, vDestDir));

			XMStoreFloat3(&pVertices[iIndices[0]].vNormal, XMLoadFloat3(&pVertices[iIndices[0]].vNormal) + vNormal);
			XMStoreFloat3(&pVertices[iIndices[1]].vNormal, XMLoadFloat3(&pVertices[iIndices[1]].vNormal) + vNormal);
			XMStoreFloat3(&pVertices[iIndices[2]].vNormal, XMLoadFloat3(&pVertices[iIndices[2]].vNormal) + vNormal);

			pIndices[iNumIndices++] = iIndices[0];
			pIndices[iNumIndices++] = iIndices[2];
			pIndices[iNumIndices++] = iIndices[3];

			vSourDir = XMLoadFloat3(&pVertices[iIndices[2]].vPosition) - XMLoadFloat3(&pVertices[iIndices[0]].vPosition);
			vDestDir = XMLoadFloat3(&pVertices[iIndices[3]].vPosition) - XMLoadFloat3(&pVertices[iIndices[2]].vPosition);
			vNormal = XMVector3Normalize(XMVector3Cross(vSourDir, vDestDir));

			XMStoreFloat3(&pVertices[iIndices[0]].vNormal, XMLoadFloat3(&pVertices[iIndices[0]].vNormal) + vNormal);
			XMStoreFloat3(&pVertices[iIndices[2]].vNormal, XMLoadFloat3(&pVertices[iIndices[2]].vNormal) + vNormal);
			XMStoreFloat3(&pVertices[iIndices[3]].vNormal, XMLoadFloat3(&pVertices[iIndices[3]].vNormal) + vNormal);
		}
	}

	for (size_t i = 0; i < m_iNumVertices; i++)
		XMStoreFloat3(&pVertices[i].vNormal, XMVector3Normalize(XMLoadFloat3(&pVertices[i].vNormal)));
#pragma endregion
	/* 버텍스버퍼의 내용을 채워주곡 */
	ZeroMemory(&m_BufferDesc, sizeof m_BufferDesc);
	m_BufferDesc.ByteWidth = m_iVertexStride * m_iNumVertices;
	m_BufferDesc.Usage = D3D11_USAGE_DYNAMIC; /* 동적버퍼로 생성한다. */
	m_BufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	m_BufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	m_BufferDesc.MiscFlags = 0;
	m_BufferDesc.StructureByteStride = m_iVertexStride;

	ZeroMemory(&m_InitialData, sizeof m_InitialData);
	m_InitialData.pSysMem = pVertices;

	/* 정점버퍼를 생성한다. */
	if (FAILED(__super::Create_Buffer(&m_pVB)))
		return E_FAIL;

	/* 인덱스버퍼의 내용을 채워주곡 */
	ZeroMemory(&m_BufferDesc, sizeof m_BufferDesc);
	m_BufferDesc.ByteWidth = m_iIndexStride * m_iNumIndices;
	m_BufferDesc.Usage = D3D11_USAGE_DYNAMIC; /* 동적버퍼로 생성한다. */
	m_BufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	m_BufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	m_BufferDesc.MiscFlags = 0;
	m_BufferDesc.StructureByteStride = m_iIndexStride;

	ZeroMemory(&m_InitialData, sizeof m_InitialData);
	m_InitialData.pSysMem = pIndices;

	/* 인덱스버퍼를 생성한다. */
	if (FAILED(__super::Create_Buffer(&m_pIB)))
		return E_FAIL;

	Safe_Delete_Array(pIndices);
	Safe_Delete_Array(pVertices);
	Safe_Delete_Array(pPixel);

	m_pQuadTree = CQuadTree::Create(m_iNumVerticesX * m_iNumVerticesZ - m_iNumVerticesX,
		m_iNumVerticesX * m_iNumVerticesZ - 1,
		m_iNumVerticesX - 1,
		0);

	m_pQuadTree->Make_Neighbors();

	return S_OK;
}

void CVIBuffer_Terrain::Culling(_fmatrix WorldMatrix)
{
	m_pGameInstance->Transform_ToLocalSpace(WorldMatrix);
	_uint			iNumIndices = { 0 };

	D3D11_MAPPED_SUBRESOURCE		SubResource{};

	m_pContext->Map(m_pIB, 0, D3D11_MAP_WRITE_DISCARD, 0, &SubResource);

	m_pQuadTree->Culling(m_pGameInstance, m_pVertexPositions, (_uint*)SubResource.pData, &iNumIndices);

	/*
	for (size_t i = 0; i < m_iNumVerticesZ - 1; i++)
	{
		for (size_t j = 0; j < m_iNumVerticesX - 1; j++)
		{
			_uint			iIndex = i * m_iNumVerticesX + j;

			_uint			iIndices[] = {
				iIndex + m_iNumVerticesX,
				iIndex + m_iNumVerticesX + 1,
				iIndex + 1,
				iIndex
			};			

			_bool			isIn[4] = {
				m_pGameInstance->isIn_Frustum_LocalSpace(XMLoadFloat3(&m_pVertexPositions[iIndices[0]]), 0.f),
				m_pGameInstance->isIn_Frustum_LocalSpace(XMLoadFloat3(&m_pVertexPositions[iIndices[1]]), 0.f),
				m_pGameInstance->isIn_Frustum_LocalSpace(XMLoadFloat3(&m_pVertexPositions[iIndices[2]]), 0.f),
				m_pGameInstance->isIn_Frustum_LocalSpace(XMLoadFloat3(&m_pVertexPositions[iIndices[3]]), 0.f),
			};

			

			/ * 오른쪽 위 삼각형이 절두체 안에 있는지? * /
			if (true == isIn[0] &&
				true == isIn[1] &&
				true == isIn[2])
			{
				((_uint*)SubResource.pData)[iNumIndices++] = iIndices[0];
				((_uint*)SubResource.pData)[iNumIndices++] = iIndices[1];
				((_uint*)SubResource.pData)[iNumIndices++] = iIndices[2];
			}			

			/ * 왼쪽 아래 삼각형이 절두체 안에 있는지? * /
			if (true == isIn[0] &&
				true == isIn[2] &&
				true == isIn[3])
			{
				((_uint*)SubResource.pData)[iNumIndices++] = iIndices[0];
				((_uint*)SubResource.pData)[iNumIndices++] = iIndices[2];
				((_uint*)SubResource.pData)[iNumIndices++] = iIndices[3];
			}			
		}
	}
	*/

	m_pContext->Unmap(m_pIB, 0);

	m_iNumIndices = iNumIndices;
}

void CVIBuffer_Terrain::Change_Height(_float HowMuch)
{
	D3D11_MAPPED_SUBRESOURCE		SubResource{};

	if (SUCCEEDED(m_pContext->Map(m_pVB, 0, D3D11_MAP_WRITE_DISCARD, 0, &SubResource)))
	{
		// 정점 버퍼 데이터에 접근합니다.
		VTXNORTEX* pVertices = reinterpret_cast<VTXNORTEX*>(SubResource.pData);

		// 예시: 정점의 위치나 노멀 값을 갱신하는 코드
		for (_uint i = 0; i < m_iNumVertices/2; ++i)
		{
			m_pVertexPositions[i].y += HowMuch;
		}

		for (_uint i = 0; i < m_iNumVerticesZ; i++)
		{
			for (_uint j = 0; j < m_iNumVerticesX; j++)
			{
				_uint			iIndex = i * m_iNumVerticesX + j;
				
				pVertices[iIndex].vPosition = m_pVertexPositions[iIndex];
				pVertices[iIndex].vNormal = _float3(0.f, 0.f, 0.f);
				pVertices[iIndex].vTexcoord = _float2(j / (m_iNumVerticesX - 1.f), i / (m_iNumVerticesZ - 1.f));
			}
		}

		m_pContext->Unmap(m_pVB, 0);
	}
}

CVIBuffer_Terrain * CVIBuffer_Terrain::Create(ID3D11Device * pDevice, ID3D11DeviceContext * pContext, const _tchar* pHeightMapFilePath)
{
	CVIBuffer_Terrain*		pInstance = new CVIBuffer_Terrain(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype(pHeightMapFilePath)))
	{
		MSG_BOX(TEXT("Failed to Created : CVIBuffer_Terrain"));
		Safe_Release(pInstance);
	}

	return pInstance;
}



CComponent * CVIBuffer_Terrain::Clone(void * pArg)
{
	CVIBuffer_Terrain*		pInstance = new CVIBuffer_Terrain(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed to Cloned : CVIBuffer_Terrain"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CVIBuffer_Terrain::Free()
{
	__super::Free();

	Safe_Release(m_pQuadTree);
	Safe_Delete_Array(m_pVertexPositions);
}
