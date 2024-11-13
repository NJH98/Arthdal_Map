#include "..\Public\Texture.h"
#include "Shader.h"

#include "wincodec.h"
#include <fstream>
#include <iostream>
#include <cstdint>

#pragma pack(push, 1)
struct BMPHeaderTexture {
	uint16_t bfType = 0x4D42; // 'BM'
	uint32_t bfSize = 0;
	uint16_t bfReserved1 = 0;
	uint16_t bfReserved2 = 0;
	uint32_t bfOffBits = 54;
};

struct BMPInfoHeaderTexture {
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

CTexture::CTexture(ID3D11Device * pDevice, ID3D11DeviceContext * pContext)
	: CComponent { pDevice, pContext }
{

}

CTexture::CTexture(const CTexture & Prototype)
	: CComponent { Prototype }
	, m_iNumTextures { Prototype.m_iNumTextures }
	, m_SRVs { Prototype.m_SRVs }
	, m_OriginTexture{ Prototype.m_OriginTexture }
{
	for (auto& pSRV : m_SRVs)
		Safe_AddRef(pSRV);

	for (auto& pOriginTexture : m_OriginTexture)
		Safe_AddRef(pOriginTexture);
}

HRESULT CTexture::Initialize_Prototype(const _tchar * pTextureFilePath, _uint iNumTextures)
{
	m_iNumTextures = iNumTextures;

	m_SRVs.reserve(iNumTextures);

	_tchar			szFullPath[MAX_PATH] = TEXT("");
	_tchar			szExt[MAX_PATH] = TEXT("");

	_wsplitpath_s(pTextureFilePath, nullptr, 0, nullptr, 0, nullptr, 0, szExt, MAX_PATH);

	for (size_t i = 0; i < m_iNumTextures; i++)
	{
		wsprintf(szFullPath, pTextureFilePath, i);

		ID3D11ShaderResourceView* pSRV = { nullptr };
		ID3D11Resource* pTextureResource = nullptr;
		HRESULT		hr = { 0 };

		if (false == lstrcmp(TEXT(".dds"), szExt))			
		{
			hr = CreateDDSTextureFromFile(m_pDevice, szFullPath, &pTextureResource, &pSRV);
		}
		else if (false == lstrcmp(TEXT(".tga"), szExt))
			return E_FAIL;

		else
		{
			hr = CreateWICTextureFromFile(m_pDevice, szFullPath, &pTextureResource, &pSRV);
		}

		if (FAILED(hr))
			return E_FAIL;

		m_SRVs.emplace_back(pSRV);
		m_OriginTexture.emplace_back(pTextureResource);
	}

	return S_OK;
}

HRESULT CTexture::Initialize(void * pArg)
{
	return S_OK;
}

HRESULT CTexture::Bind_ShadeResource(CShader * pShader, const _char * pConstantName, _uint iTextureIndex)
{
	if (iTextureIndex >= m_iNumTextures)
		return E_FAIL;

	return pShader->Bind_SRV(pConstantName, m_SRVs[iTextureIndex]);	
}

HRESULT CTexture::Bind_ShadeResources(CShader * pShader, const _char * pConstantName)
{
	return pShader->Bind_SRVs(pConstantName, &m_SRVs.front(), m_iNumTextures);
}

HRESULT CTexture::Bind_ShadeResourcesMask(CShader* pShader, const _char* pConstantName, _uint iTextureNum)
{
	return pShader->Bind_SRVs(pConstantName, &m_SRVs.front(), iTextureNum);
}

HRESULT CTexture::Add_MaskTexture()
{
	// 텍스쳐 색상 입히는 용도의 subresource_data ( 흰색 불투명 )
	vector<UINT8> initData(256 * 256 * 4, 255); 
	D3D11_SUBRESOURCE_DATA SubresourceData = {};
	SubresourceData.pSysMem = initData.data();
	SubresourceData.SysMemPitch = 256 * 4;

	// CPU 접근용 Staging 텍스쳐
	D3D11_TEXTURE2D_DESC stagingDesc = {};
	stagingDesc.Width = 256;
	stagingDesc.Height = 256;
	stagingDesc.MipLevels = 1;
	stagingDesc.ArraySize = 1;
	stagingDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	stagingDesc.Usage = D3D11_USAGE_STAGING;
	stagingDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ | D3D11_CPU_ACCESS_WRITE;
	stagingDesc.BindFlags = 0;
	stagingDesc.MiscFlags = 0;
	stagingDesc.SampleDesc.Count = 1;
	stagingDesc.SampleDesc.Quality = 0;

	// 텍스처 생성
	ID3D11Texture2D* pStagingTexture = nullptr;
	if (FAILED(m_pDevice->CreateTexture2D(&stagingDesc, &SubresourceData, &pStagingTexture))) {
		MSG_BOX(TEXT("Failed to Mask Texture Stageing"));
		return E_FAIL;
	}
	m_StagingTexture.push_back(pStagingTexture);

	// GPU 접근용 텍스처 설명 설정 (셰이더 리소스로 사용)
	D3D11_TEXTURE2D_DESC textureDesc = {};
	textureDesc.Width = 256;
	textureDesc.Height = 256;
	textureDesc.MipLevels = 1;
	textureDesc.ArraySize = 1;
	textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	textureDesc.CPUAccessFlags = 0;
	textureDesc.MiscFlags = 0;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.SampleDesc.Quality = 0;

	// 텍스처 생성
	ID3D11Texture2D* pTexture = nullptr;
	if (FAILED(m_pDevice->CreateTexture2D(&textureDesc, &SubresourceData, &pTexture))) {
		MSG_BOX(TEXT("Failed to Mask Texture"));
		return E_FAIL;
	}
	m_ShaderTexture.push_back(pTexture);

	// ShaderResourceView 생성
	ID3D11ShaderResourceView* pShaderResourceView = nullptr;
	if (FAILED(m_pDevice->CreateShaderResourceView(pTexture, nullptr, &pShaderResourceView))) {
		MSG_BOX(TEXT("Failed to create Shader Resource View"));
		return E_FAIL;
	}
	m_SRVs.emplace_back(pShaderResourceView);

	m_iNumTextures++;

	return S_OK;
}

HRESULT CTexture::Delete_MaskTexture(_uint iChoiceTextures)
{
	if (m_iNumTextures == 0)
		return S_OK;

	if (iChoiceTextures > m_iNumTextures)
		return E_FAIL;

	auto iterStaging = m_StagingTexture.begin();
	auto iterShader = m_ShaderTexture.begin();
	auto iterSRV = m_SRVs.begin();

	for (_uint i = 0; i < iChoiceTextures; i++) {
		iterStaging++;
		iterShader++;
		iterSRV++;
	}

	Safe_Release(*iterStaging);
	m_StagingTexture.erase(iterStaging);

	Safe_Release(*iterShader);
	m_ShaderTexture.erase(iterShader);

	Safe_Release(*iterSRV);
	m_SRVs.erase(iterSRV);

	m_iNumTextures--;
	return S_OK;
}

HRESULT CTexture::Pick_ChangeMask(_float2 PickPos2d, _uint iChoiceTextures, _uint Range, _uint Value, _uint RGB)
{
	if (iChoiceTextures > m_iNumTextures)
		return E_FAIL;

	vector<Point> Point{};

	// 좌 하단
	_uint LD_Pixely = {};
	_uint LD_Pixelx = {};
	if (_uint(PickPos2d.y) > Range)
		LD_Pixely = _uint(PickPos2d.y) - Range;
	else
		LD_Pixely = 0;

	if (_uint(PickPos2d.x) > Range)
		LD_Pixelx = _uint(PickPos2d.x) - Range;
	else
		LD_Pixelx = 0;

	// 우 상단
	_uint RT_Pixely = {};
	_uint RT_Pixelx = {};
	RT_Pixely = _uint(PickPos2d.y) + Range;
	if (RT_Pixely > 255) {
		RT_Pixely = 255;
	}

	RT_Pixelx = _uint(PickPos2d.x) + Range;
	if (RT_Pixelx > 255) {
		RT_Pixelx = 255;
	}

	for (_uint x = LD_Pixelx; x <= RT_Pixelx; ++x) {
		for (_uint y = LD_Pixely; y <= RT_Pixely; ++y) {
			Point.push_back({ x, y});
		}
	}

	// CPU용 스테이징 텍스쳐를 맵핑
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	if (FAILED(m_pContext->Map(m_StagingTexture[iChoiceTextures], 0, D3D11_MAP_WRITE, 0, &mappedResource))) {
		MSG_BOX(TEXT("Failed to map Staging Texture"));
		return E_FAIL;
	}

	// 텍스처 데이터에 접근
	UINT8* pTexels = static_cast<UINT8*>(mappedResource.pData);


	for (auto& iter : Point)
	{
		UINT8* pixel = pTexels + iter.y * mappedResource.RowPitch + iter.x * 4;
		switch (RGB)
		{
		case 0: pixel[0] = Value; break;
		case 1: pixel[1] = Value; break;
		case 2: pixel[2] = Value; break;
		case 3: pixel[0] = Value; pixel[1] = Value; pixel[2] = Value; break;
		default:
			break;
		}
		//pixel[0] = Value; // R 값
		//pixel[1] = Value; // G 값
		//pixel[2] = Value; // B 값
		//pixel[3] = 0; // A 값(변경하지 않음)
	}

	// 언맵하여 변경사항 반영
	m_pContext->Unmap(m_StagingTexture[iChoiceTextures], 0);

	// pStagingTexture의 데이터를 pTexture로 복사
	m_pContext->CopyResource(m_ShaderTexture[iChoiceTextures], m_StagingTexture[iChoiceTextures]);

	return S_OK;
}

HRESULT CTexture::Swap_SRVs(_uint iFirst, _uint iSecond)
{
	ID3D11ShaderResourceView* pSave = nullptr;

	pSave = m_SRVs[iFirst];
	m_SRVs[iFirst] = nullptr;
	m_SRVs[iFirst] = m_SRVs[iSecond];
	m_SRVs[iSecond] = nullptr;
	m_SRVs[iSecond] = pSave;

	return S_OK;
}

HRESULT CTexture::Save_MaskTexture(const _tchar* pMaskFilePath, _uint iChoiceTextures)
{
	BMPHeaderTexture bmpHeader;
	BMPInfoHeaderTexture bmpInfoHeader;

	bmpInfoHeader.biWidth = 256;
	bmpInfoHeader.biHeight = 256;
	bmpInfoHeader.biSizeImage = 256 * 256 * 4;

	bmpHeader.bfSize = bmpHeader.bfOffBits + bmpInfoHeader.biSizeImage;

	ofstream ofs(pMaskFilePath, ios::binary | ios::out);
	if (!ofs) {
		MSG_BOX(TEXT("Failed to open file for writing"));
		return E_FAIL;
	}

	// Write headers
	ofs.write(reinterpret_cast<char*>(&bmpHeader), sizeof(bmpHeader));
	ofs.write(reinterpret_cast<char*>(&bmpInfoHeader), sizeof(bmpInfoHeader));

	// CPU용 스테이징 텍스쳐를 맵핑
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	if (FAILED(m_pContext->Map(m_StagingTexture[iChoiceTextures], 0, D3D11_MAP_WRITE, 0, &mappedResource))) {
		MSG_BOX(TEXT("Failed to map Staging Texture"));
		return E_FAIL;
	}

	// 텍스처 데이터에 접근
	UINT8* pTexels = static_cast<UINT8*>(mappedResource.pData);

	// Write image data
	for (_uint z = 0; z < 256; ++z) {
		for (_uint x = 0; x < 256; ++x) {
			_uint index = z * 256 + x;

			UINT8* pixel = pTexels + z * mappedResource.RowPitch + x * 4;
			uint8_t color[4] = { pixel[2], pixel[1], pixel[0], 255}; //Red, Green, Blue, Alpha
			ofs.write(reinterpret_cast<char*>(color), 4);
		}
	}

	ofs.close();
	if (!ofs.good()) {
		MSG_BOX(TEXT("Failed to open file for writing"));
		return E_FAIL;
	}

	m_pContext->Unmap(m_StagingTexture[iChoiceTextures], 0);


	return S_OK;
}

HRESULT CTexture::Load_MaskTexture(const _tchar* pMaskFilePath, _uint iChoiceTextures)
{
#pragma region 파일경로 파일열기
	_uint* pPixel{};
	_ulong			dwByte = {};

	HANDLE			hFile = CreateFile(
		pMaskFilePath,
		GENERIC_READ | GENERIC_WRITE,
		FILE_SHARE_READ | FILE_SHARE_WRITE,
		nullptr,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL, 0);

	if (0 == hFile) {
		MSG_BOX(TEXT("Failed to open file for writing"));
		return E_FAIL;
	}

	BITMAPFILEHEADER			fh{};
	BITMAPINFOHEADER			ih{};

	ReadFile(hFile, &fh, sizeof fh, &dwByte, nullptr);
	ReadFile(hFile, &ih, sizeof ih, &dwByte, nullptr);
	_uint m_iNumVerticesX = ih.biWidth;
	_uint m_iNumVerticesZ = ih.biHeight;

	pPixel = new _uint[m_iNumVerticesX * m_iNumVerticesZ];
	ReadFile(hFile, pPixel, sizeof(_uint) * m_iNumVerticesX * m_iNumVerticesZ, &dwByte, nullptr);

	CloseHandle(hFile);
#pragma endregion

	// CPU용 스테이징 텍스쳐를 맵핑
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	if (FAILED(m_pContext->Map(m_StagingTexture[iChoiceTextures], 0, D3D11_MAP_WRITE, 0, &mappedResource))) {
		MSG_BOX(TEXT("Failed to map Staging Texture"));
		return E_FAIL;
	}

	// 텍스처 데이터에 접근
	UINT8* pTexels = static_cast<UINT8*>(mappedResource.pData);

	for (_uint z = 0; z < m_iNumVerticesZ; z++)
	{
		for (_uint x = 0; x < m_iNumVerticesX; x++)
		{
			_uint			iIndex = z * m_iNumVerticesX + x;

			UINT8* pixel = pTexels + z * mappedResource.RowPitch + x * 4;
			pixel[0] = (pPixel[iIndex] & 0x00ff0000) >> 16; // R 빨강
			pixel[1] = (pPixel[iIndex] & 0x0000ff00) >> 8;	// G 초록
			pixel[2] = (pPixel[iIndex] & 0x000000ff);		// B 파랑
		}
	}

	// 언맵하여 변경사항 반영
	m_pContext->Unmap(m_StagingTexture[iChoiceTextures], 0);

	// pStagingTexture의 데이터를 pTexture로 복사
	m_pContext->CopyResource(m_ShaderTexture[iChoiceTextures], m_StagingTexture[iChoiceTextures]);

	Safe_Delete_Array(pPixel);

	return S_OK;
}

CTexture * CTexture::Create(ID3D11Device * pDevice, ID3D11DeviceContext * pContext, const _tchar * pTextureFilePath, _uint iNumTextures)
{
	CTexture*		pInstance = new CTexture(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype(pTextureFilePath, iNumTextures)))
	{
		MSG_BOX(TEXT("Failed to Created : CTexture"));
		Safe_Release(pInstance);
	}

	return pInstance;
}



CComponent * CTexture::Clone(void * pArg)
{
	CTexture*		pInstance = new CTexture(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed to Cloned : CTexture"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CTexture::Free()
{
	__super::Free();

	for (auto& pSRV : m_SRVs)
		Safe_Release(pSRV);
	m_SRVs.clear();

	for (auto& pOriginTexture : m_OriginTexture)
		Safe_Release(pOriginTexture);
	m_OriginTexture.clear();

	for (auto& pStagingTexture : m_StagingTexture)
		Safe_Release(pStagingTexture);
	m_StagingTexture.clear();

	for (auto& pShaderTexture : m_ShaderTexture)
		Safe_Release(pShaderTexture);
	m_ShaderTexture.clear();
}
