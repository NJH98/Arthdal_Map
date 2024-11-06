#include "stdafx.h"
#include "MapObject.h"

#include "GameInstance.h"

CMapObject::CMapObject(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CGameObject{ pDevice, pContext }
{
}

CMapObject::CMapObject(const CMapObject& Prototype)
    : CGameObject(Prototype)
{
}

HRESULT CMapObject::Initialize_Prototype()
{
    if (FAILED(__super::Initialize_Prototype()))
        return E_FAIL;

    return S_OK;
}

HRESULT CMapObject::Initialize(void* pArg)
{
    if (FAILED(__super::Initialize(pArg)))
        return E_FAIL;

    return S_OK;
}

void CMapObject::Priority_Update(_float fTimeDelta)
{
    __super::Priority_Update(fTimeDelta);
}

_int CMapObject::Update(_float fTimeDelta)
{
    __super::Update(fTimeDelta);

    return OBJ_NOEVENT;
}

void CMapObject::Late_Update(_float fTimeDelta)
{
    __super::Late_Update(fTimeDelta);
}

void CMapObject::Free()
{
    __super::Free();
}
