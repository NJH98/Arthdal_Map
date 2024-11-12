#include "..\Public\VIBuffer_Terrain.h"
#include "GameInstance.h"

#include "QuadTree.h"

#include "wincodec.h"
#include <fstream>
#include <iostream>
#include <cstdint>

#pragma pack(push, 1)
struct BMPHeader {
	uint16_t bfType = 0x4D42; // 'BM'
	uint32_t bfSize = 0;
	uint16_t bfReserved1 = 0;
	uint16_t bfReserved2 = 0;
	uint32_t bfOffBits = 54;
};

struct BMPInfoHeader {
	uint32_t biSize = 40;
	int32_t biWidth = 0;
	int32_t biHeight = 0;
	uint16_t biPlanes = 1;
	uint16_t biBitCount = 32;
	uint32_t biCompression = 0;
	uint32_t biSizeImage = 0;
	int32_t biXPelsPerMeter = 0;
	int32_t biYPelsPerMeter = 0;
	uint32_t biClrUsed = 0;
	uint32_t biClrImportant = 0;
};
#pragma pack(pop)

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
//			pVertices[iIndex].vPosition = m_pVertexPositions[iIndex] = _float3(_float(j), _float((pPixel[iIndex] & 0x000000ff)), _float(i));
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
	_uint* pPixel{};
	TERRAIN_BUFFER_DESC* Desc = nullptr;

	if (pArg != nullptr) 
	{
		Desc = static_cast<TERRAIN_BUFFER_DESC*>(pArg);

		if (Desc->pHeightMapFilePath == nullptr) 
		{
			m_iNumVerticesX = _uint(Desc->TerrainXZ.x);
			m_iNumVerticesZ = _uint(Desc->TerrainXZ.y);

			if (m_iNumVerticesX == 0 ||
				m_iNumVerticesZ == 0) {

				m_iNumVerticesX = 64;
				m_iNumVerticesZ = 64;
			}

			pPixel = new _uint[m_iNumVerticesX * m_iNumVerticesZ];
		}
		else
		{
			_ulong			dwByte = {};
			
			HANDLE			hFile = CreateFile(
										Desc->pHeightMapFilePath, 
										GENERIC_READ | GENERIC_WRITE,
										FILE_SHARE_READ | FILE_SHARE_WRITE, 
										nullptr, 
										OPEN_EXISTING, 
										FILE_ATTRIBUTE_NORMAL, 0);
			if (0 == hFile)
				return E_FAIL;
			
			BITMAPFILEHEADER			fh{};
			BITMAPINFOHEADER			ih{};
			
			ReadFile(hFile, &fh, sizeof fh, &dwByte, nullptr);
			ReadFile(hFile, &ih, sizeof ih, &dwByte, nullptr);
			m_iNumVerticesX = ih.biWidth;
			m_iNumVerticesZ = ih.biHeight;
			
			pPixel = new _uint[m_iNumVerticesX * m_iNumVerticesZ];
			ReadFile(hFile, pPixel, sizeof(_uint) * m_iNumVerticesX * m_iNumVerticesZ, &dwByte, nullptr);
			
			CloseHandle(hFile);
		}
	}
	else 
	{
		m_iNumVerticesX = 64;
		m_iNumVerticesZ = 64;
	
		pPixel = new _uint[m_iNumVerticesX * m_iNumVerticesZ];
	}

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

			if (Desc != nullptr && Desc->pHeightMapFilePath != nullptr){
				pVertices[iIndex].vPosition = m_pVertexPositions[iIndex] = _float3(_float(j), _float((pPixel[iIndex] & 0x000000ff)), _float(i));
			}
			else {
				pVertices[iIndex].vPosition = m_pVertexPositions[iIndex] = _float3(_float(j), 0.f, _float(i));
			}
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

_float CVIBuffer_Terrain::Compute_Height(_float3& vLocalPos)
{
	// 객체가 존재하는 네모 영역의 왼쪽 하단 인덱스
	_uint iIndex = _uint(vLocalPos.z) * m_iNumVerticesX + _uint(vLocalPos.x);
	// 구한 인덱스 기준으로 네모 영역의 인덱스 들을 구한다
	_uint iIndices[4] = {
		iIndex + m_iNumVerticesX,		// 좌 상단 
		iIndex + m_iNumVerticesX + 1,	// 우 상단
		iIndex + 1,						// 우 하단
		iIndex							// 좌 하단
	};

	_float		fWidth = vLocalPos.x - m_pVertexPositions[iIndices[0]].x;
	_float		fDepth = m_pVertexPositions[iIndices[0]].z - vLocalPos.z;

	XMVECTOR Plane;

	// 오른쪽 위 삼각형에 있는 경우
	if (fWidth > fDepth) {
		Plane = XMPlaneFromPoints(XMLoadFloat3(&m_pVertexPositions[iIndices[0]]),
			XMLoadFloat3(&m_pVertexPositions[iIndices[1]]),
			XMLoadFloat3(&m_pVertexPositions[iIndices[2]]));
	}
	// 왼쪽 아래 삼각형에 있는 경우
	else {
		Plane = XMPlaneFromPoints(XMLoadFloat3(&m_pVertexPositions[iIndices[0]]),
			XMLoadFloat3(&m_pVertexPositions[iIndices[2]]),
			XMLoadFloat3(&m_pVertexPositions[iIndices[3]]));
	}

	// 평면 방정식의 계수를 추출
	_float a = XMVectorGetX(Plane);
	_float b = XMVectorGetY(Plane);
	_float c = XMVectorGetZ(Plane);
	_float d = XMVectorGetW(Plane);

	// y 값을 계산
	return (-a * vLocalPos.x - c * vLocalPos.z - d) / b;
}

void CVIBuffer_Terrain::Change_Height(_float Range, _float HowMuch)
{
	D3D11_MAPPED_SUBRESOURCE		SubResource{};

	if (SUCCEEDED(m_pContext->Map(m_pVB, 0, D3D11_MAP_WRITE_DISCARD, 0, &SubResource)))
	{
		// 정점 버퍼 데이터에 접근합니다.
		VTXNORTEX* pVertices = reinterpret_cast<VTXNORTEX*>(SubResource.pData);

		// 예시: 정점의 위치나 노멀 값을 갱신하는 코드
		for (_uint i = 0; i < m_iNumVertices; ++i)
		{
			Vector3 VertexPos = m_pVertexPositions[i];
			Vector3 MousePos = m_pGameInstance->Get_GlobalData()->Pick_Pos;

			_float Length = (VertexPos - MousePos).Length();

			if(Length < Range)
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

HRESULT CVIBuffer_Terrain::Save_HeightMap(const _tchar* pHeightMapFilePath)
{
	BMPHeader bmpHeader;
	BMPInfoHeader bmpInfoHeader;

	bmpInfoHeader.biWidth = m_iNumVerticesX;
	bmpInfoHeader.biHeight = m_iNumVerticesZ;
	bmpInfoHeader.biSizeImage = m_iNumVerticesX * m_iNumVerticesZ * 4;

	bmpHeader.bfSize = bmpHeader.bfOffBits + bmpInfoHeader.biSizeImage;

	ofstream ofs(pHeightMapFilePath, ios::binary | ios::out);
	if (!ofs) {
		cout << "Failed to open file for writing: " << pHeightMapFilePath << endl;
		return E_FAIL;
	}

	// Write headers
	ofs.write(reinterpret_cast<char*>(&bmpHeader), sizeof(bmpHeader));
	ofs.write(reinterpret_cast<char*>(&bmpInfoHeader), sizeof(bmpInfoHeader));

	// Write image data
	for (_uint z = 0; z < m_iNumVerticesZ; ++z) {
		for (_uint x = 0; x < m_iNumVerticesX; ++x) {
			int index = z * m_iNumVerticesX + x;
			uint8_t height = static_cast<uint8_t>(m_pVertexPositions[index].y); // Scale height to [0, 255]
			uint8_t color[4] = { height, height, height, 255 }; // Blue, Green, Red, Alpha
			ofs.write(reinterpret_cast<char*>(color), 4);
		}
	}

	ofs.close();
	if (!ofs.good()) {
		cout << "Error occurred at writing time!" << endl;
	}

	return S_OK;
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
