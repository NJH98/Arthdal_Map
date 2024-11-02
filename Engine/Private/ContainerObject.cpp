#include "..\Public\ContainerObject.h"
#include "GameInstance.h"
#include "PartObject.h"

CContainerObject::CContainerObject(ID3D11Device * pDevice, ID3D11DeviceContext * pContext)
	: CGameObject { pDevice, pContext }
{
}

CContainerObject::CContainerObject(const CContainerObject & Prototype)
	: CGameObject{ Prototype }
{
}

CComponent * CContainerObject::Find_Component(const _wstring & strComponentTag, _uint iPartObjIndex)
{
	if (iPartObjIndex >= m_Parts.size())
		return nullptr;

	return m_Parts[iPartObjIndex]->Find_Component(strComponentTag);	
}

HRESULT CContainerObject::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CContainerObject::Initialize(void * pArg)
{
	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	return S_OK;
}

void CContainerObject::Priority_Update(_float fTimeDelta)
{
}

_int CContainerObject::Update(_float fTimeDelta)
{

	return OBJ_NOEVENT;
}

void CContainerObject::Late_Update(_float fTimeDelta)
{
}

HRESULT CContainerObject::Render()
{
	return S_OK;
}

HRESULT CContainerObject::Add_PartObject(_uint iPartID, const _wstring & strPrototypeTag, void * pArg)
{
	CGameObject*		pPartObject = m_pGameInstance->Clone_GameObject(strPrototypeTag, pArg);

	if (nullptr == pPartObject)
		return E_FAIL;

	m_Parts[iPartID] = dynamic_cast<CPartObject*>(pPartObject);

	return S_OK;
}

void CContainerObject::Free()
{
	__super::Free();

	for (auto& pPartObject : m_Parts)
		Safe_Release(pPartObject);

	m_Parts.clear();
}
