#pragma once

#include "Client_Defines.h"
#include "ContainerObject.h"

BEGIN(Engine)
class CNavigation;
class CCollider;
class CShader;
class CModel;

class CVIBuffer_Terrain;
END

BEGIN(Client)

class CPlayer final : public CContainerObject
{
public:
	enum PARTID { PART_BODY, PART_WEAPON, PART_EFFECT, PART_END };
	enum STATE {
		STATE_RESET = 0x00000000,
		STATE_IDLE = 0x00000001,	// 0000 0001
		STATE_WALK = 0x00000002,	// 0000 0010 
		STATE_ATTACK = 0x00000004,	// 0000 0100
		//STATE_ATTACK = 0x00000008,	// 0000 1000
		//STATE_ATTACK = 0x00000010,	// 0001 0000
		//STATE_ATTACK = 0x00000020,	// 0010 0000
		//STATE_ATTACK = 0x00000040,	// 0100 0000
		//STATE_ATTACK = 0x00000080,	// 1000 0000
		STATE_END = 0xffffffff,
	};
private:
	CPlayer(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CPlayer(const CPlayer& Prototype);
	virtual ~CPlayer() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Priority_Update(_float fTimeDelta) override;
	virtual _int Update(_float fTimeDelta) override;
	virtual void Late_Update(_float fTimeDelta) override;
	virtual HRESULT Render() override;

private:
	// 지형타기 향후 상위 부모에 넣을수 있다
	HRESULT Ready_Terrain();
	HRESULT Terrain_Landing(CTransform* pTransform, _float fOffsetY = 0.f);
	CVIBuffer_Terrain* m_pTerrainVIBuffer = { nullptr };
	CTransform* m_pTerrainTransform = { nullptr };

private:
	_uint				m_iState = {  };
	
	CModel*			m_pModelCom = { nullptr };
	CNavigation*	m_pNavigationCom = { nullptr };
	CShader*		m_pShaderCom = { nullptr };
	CCollider*		m_pColliderCom = { nullptr };

private:
	HRESULT Ready_Components();
	HRESULT Ready_PartObjects();

public:
	static CPlayer* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg);
	virtual void Free() override;

};


END