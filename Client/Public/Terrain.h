#pragma once

#include "Client_Defines.h"
#include "GameObject.h"

BEGIN(Engine)
class CShader;
class CTexture;
//class CNavigation;
class CVIBuffer_Terrain;
END

BEGIN(Client)

class CTerrain final : public CGameObject
{
public:
	enum TEXTURE { TEXTURE_DIFFUSE, TEXTURE_NORMAL, TEXTURE_BRUSH, TEXTURE_MASK, TEXTURE_END };
private:
	CTerrain(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CTerrain(const CTerrain& Prototype);
	virtual ~CTerrain() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Priority_Update(_float fTimeDelta) override;
	virtual _int Update(_float fTimeDelta) override;
	virtual void Late_Update(_float fTimeDelta) override;
	virtual HRESULT Render() override;

public:
	CVIBuffer_Terrain*	Get_VIBuffer() { return m_pVIBufferCom; }
	CTexture* Get_Texture(_uint Texture) { return m_pTextureCom[Texture]; }

	_uint Get_BaseTextureNum() { return m_iBaseTextureNum; }
	void Set_BaseTextureNum(_uint TextureNum) { m_iBaseTextureNum = TextureNum; }

	CNavigation* Get_NavigationCom() { return m_pNavigationCom; }

	void Change_Wire() {
		if (m_iUseShader == 0)
			m_iUseShader = 1;
		else
			m_iUseShader = 0;
	}

private:
	CShader*				m_pShaderCom = { nullptr };
	CTexture*				m_pTextureCom[TEXTURE_END] = { nullptr };
	CNavigation*			m_pNavigationCom = { nullptr };
	CVIBuffer_Terrain*		m_pVIBufferCom = { nullptr };

	_uint					m_iBaseTextureNum = 0;

	_uint					m_iUseShader = 0;
private:
	HRESULT Ready_Components(void* pArg);

public:
	static CTerrain* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg);
	virtual void Free() override;

};

END