#pragma once

#include "Base.h"

/* 객체들을 보관한다. */

BEGIN(Engine)

class CLayer final : public CBase
{
private:
	CLayer();
	virtual ~CLayer() = default;

public:
	HRESULT Add_GameObject(class CGameObject* pGameObject);
	HRESULT Priority_Update(_float fTimeDelta);
	HRESULT Update(_float fTimeDelta);
	HRESULT Late_Update(_float fTimeDelta);

public:
	class CGameObject* Get_Object(_uint iIndex = 0);
	list<class CGameObject*>* Get_ObjectList() { return &m_GameObjects; };

public:
	class CComponent* Find_Component(const _wstring& strComponentTag, _uint iIndex, _uint iPartObjIndex);

private:
	list<class CGameObject*>			m_GameObjects;

public:
	static CLayer* Create();
	virtual void Free() override;
};

END