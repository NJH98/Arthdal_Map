#pragma once

#include "Base.h"
#include "Transform.h"

/* ��� ���ӳ��� ���Ǵ� ���� ������Ʈ���� �θ� Ŭ������. */

BEGIN(Engine)

class ENGINE_DLL CGameObject abstract : public CBase
{
public:
	typedef struct : public CTransform::TRANSFORM_DESC
	{
		_float3		Angle = { 0.f, 0.f, 0.f };
		_float3		Scale = { 1.f, 1.f, 1.f };
		_float3		Pos = { 0.f, 0.f, 0.f };

		_uint		ModelNum = 0;

		_wstring	LayerTag = {};

	} GAMEOBJECT_DESC;

protected:
	CGameObject(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CGameObject(const CGameObject& Prototype);
	virtual ~CGameObject() = default;

public:
	virtual HRESULT Initialize_Prototype();
	virtual HRESULT Initialize(void* pArg = nullptr);
	virtual void Priority_Update(_float fTimeDelta);
	virtual _int Update(_float fTimeDelta);
	virtual void Late_Update(_float fTimeDelta);
	virtual HRESULT Render();
	virtual HRESULT Render_LightDepth() { return S_OK; }

public:
	_bool Get_Dead() { return m_bDead; }
	void Set_Dead(_bool bDead) { m_bDead = bDead; }

public:
	virtual class CComponent* Find_Component(const _wstring& strComponentTag, _uint iPartObjIndex = 0);
	class CTransform* Get_TranformCom() { return m_pTransformCom; }
	GAMEOBJECT_DESC Get_GameObjDesc() { return m_GameObjDesc; }
	void Set_GameObjDesc(GAMEOBJECT_DESC desc) { m_GameObjDesc = desc; }

protected:
	ID3D11Device*				m_pDevice = { nullptr };
	ID3D11DeviceContext*		m_pContext = { nullptr };

	class CTransform*			m_pTransformCom = { nullptr };
	class CGameInstance*		m_pGameInstance = { nullptr };
	GAMEOBJECT_DESC				m_GameObjDesc;

protected:
	map<const _wstring, class CComponent*>			m_Components;

protected:
	_bool m_bDead = { false };

protected:
	
	HRESULT Add_Component(_uint iLevelIndex, const _wstring& strPrototypeTag, const _wstring& strComponentTag, CComponent** ppOut, void* pArg = nullptr);


public:
	virtual CGameObject* Clone(void* pArg) = 0;
	virtual void Free() override;
};

END