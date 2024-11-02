#pragma once

#include "Client_Defines.h"
#include "BlendObject.h"

BEGIN(Engine)
class CShader;
class CTexture;
class CVIBuffer_Rect;
END

BEGIN(Client)

class CEffect_Explosion final : public CBlendObject
{
private:
	CEffect_Explosion(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CEffect_Explosion(const CEffect_Explosion& Prototype);
	virtual ~CEffect_Explosion() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Priority_Update(_float fTimeDelta) override;
	virtual _int Update(_float fTimeDelta) override;
	virtual void Late_Update(_float fTimeDelta) override;
	virtual HRESULT Render() override;

private:
	class CShader*				m_pShaderCom = { nullptr };
	class CTexture*				m_pTextureCom = { nullptr };
	class CVIBuffer_Rect*		m_pVIBufferCom = { nullptr };

private:
	_float						m_fFrame = { 0.f };

private:
	HRESULT Ready_Components();

public:
	static CEffect_Explosion* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg);
	virtual void Free() override;

};

END