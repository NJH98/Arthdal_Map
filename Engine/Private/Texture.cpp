#include "..\Public\Texture.h"
#include "Shader.h"

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

HRESULT CTexture::Add_MaskTexture()
{
	// �ؽ��� ���� ������ �뵵�� subresource_data ( ��� ������ )
	vector<UINT8> initData(256 * 256 * 4, 255); 
	D3D11_SUBRESOURCE_DATA SubresourceData = {};
	SubresourceData.pSysMem = initData.data();
	SubresourceData.SysMemPitch = 256 * 4;

	// CPU ���ٿ� Staging �ؽ���
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

	// �ؽ�ó ����
	ID3D11Texture2D* pStagingTexture = nullptr;
	if (FAILED(m_pDevice->CreateTexture2D(&stagingDesc, &SubresourceData, &pStagingTexture))) {
		MSG_BOX(TEXT("Failed to Mask Texture Stageing"));
		return E_FAIL;
	}
	m_StagingTexture.push_back(pStagingTexture);

	// GPU ���ٿ� �ؽ�ó ���� ���� (���̴� ���ҽ��� ���)
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

	// �ؽ�ó ����
	ID3D11Texture2D* pTexture = nullptr;
	if (FAILED(m_pDevice->CreateTexture2D(&textureDesc, &SubresourceData, &pTexture))) {
		MSG_BOX(TEXT("Failed to Mask Texture"));
		return E_FAIL;
	}
	m_ShaderTexture.push_back(pTexture);

	// ShaderResourceView ����
	ID3D11ShaderResourceView* pShaderResourceView = nullptr;
	if (FAILED(m_pDevice->CreateShaderResourceView(pTexture, nullptr, &pShaderResourceView))) {
		MSG_BOX(TEXT("Failed to create Shader Resource View"));
		return E_FAIL;
	}
	m_SRVs.emplace_back(pShaderResourceView);

	m_iNumTextures++;

	return S_OK;
}

HRESULT CTexture::Pick_ChangeMask(_float2 PickPos2d, _uint iChoiceTextures)
{
	if (iChoiceTextures > m_iNumTextures)
		return E_FAIL;

	// CPU�� ������¡ �ؽ��ĸ� ����
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	if (FAILED(m_pContext->Map(m_StagingTexture[iChoiceTextures], 0, D3D11_MAP_WRITE, 0, &mappedResource))) {
		MSG_BOX(TEXT("Failed to map Staging Texture"));
		return E_FAIL;
	}

	// �ؽ�ó �����Ϳ� �����Ͽ� R ���� 255�� ����
	UINT8* pTexels = static_cast<UINT8*>(mappedResource.pData);

	UINT8* pixel = pTexels + _uint(PickPos2d.y) * mappedResource.RowPitch + _uint(PickPos2d.x) * 4;
	pixel[0] = 0; // R ��
	pixel[1] = 0; // G ��(�������� ����)
	pixel[2] = 0; // B ��(�������� ����)
	//pixel[3] = 0; // A ��(�������� ����)

	// ����Ͽ� ������� �ݿ�
	m_pContext->Unmap(m_StagingTexture[iChoiceTextures], 0);

	// pStagingTexture�� �����͸� pTexture�� ����
	m_pContext->CopyResource(m_ShaderTexture[iChoiceTextures], m_StagingTexture[iChoiceTextures]);

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
}
