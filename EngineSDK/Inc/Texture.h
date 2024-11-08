#pragma once

#include "Component.h"
#include <VIBuffer_Terrain.h>

/* DirectXTex Lib */ /* .dds, .tga, wic(.jpg, .png, .bmp etc) */
/* DirectXTK Lib ToolKit */ /* .dds, wic(.jpg, .png, .bmp etc) */

BEGIN(Engine)

class ENGINE_DLL CTexture final : public CComponent
{	
public:
	struct Point {
		_uint x, y;
	};

private:
	CTexture(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CTexture(const CTexture& Prototype);
	virtual ~CTexture() = default;

public:
	virtual HRESULT Initialize_Prototype(const _tchar* pTextureFilePath, _uint iNumTextures);
	virtual HRESULT Initialize(void* pArg) override;

public:
	HRESULT Bind_ShadeResource(class CShader* pShader, const _char* pConstantName, _uint iTextureIndex);
	HRESULT Bind_ShadeResources(class CShader* pShader, const _char* pConstantName);
	HRESULT Bind_ShadeResourcesMask(class CShader* pShader, const _char* pConstantName, _uint iTextureNum);

	_uint Get_TextureNum() const { return m_iNumTextures; }

	HRESULT Add_MaskTexture();
	HRESULT Delete_MaskTexture(_uint iChoiceTextures);
	HRESULT Pick_ChangeMask(_float2 PickPos2d, _uint iChoiceTextures, _uint Range, _uint Value, _uint RGB);
	HRESULT Swap_SRVs(_uint iFirst, _uint iSecond);
	HRESULT Save_MaskTexture(const _tchar* pMaskFilePath, _uint iChoiceTextures);
	HRESULT Load_MaskTexture(const _tchar* pMaskFilePath, _uint iChoiceTextures);
	ID3D11ShaderResourceView* Get_ShaderResourceView(_uint iChoiceTextures) { return m_SRVs[iChoiceTextures]; }

private:
	vector<ID3D11ShaderResourceView*>		m_SRVs;
	vector<ID3D11Resource*>					m_OriginTexture;
	_uint									m_iNumTextures = { 0 };

	// Mask 텍스쳐 수정용
	vector<ID3D11Texture2D*>				m_StagingTexture;
	vector<ID3D11Texture2D*>				m_ShaderTexture;


public:
	static CTexture* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, const _tchar* pTextureFilePath, _uint iNumTextures);
	virtual CComponent* Clone(void* pArg) override;
	virtual void Free() override;
};

END