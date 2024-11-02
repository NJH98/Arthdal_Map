#include "..\Public\PartObject.h"
#include "Shader.h"

CPartObject::CPartObject(ID3D11Device * pDevice, ID3D11DeviceContext * pContext)
	: CGameObject {pDevice, pContext }
{
}

CPartObject::CPartObject(const CPartObject & Prototype)
	: CGameObject { Prototype }
{
}

HRESULT CPartObject::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CPartObject::Initialize(void * pArg)
{
	PARTOBJ_DESC*		pDesc = static_cast<PARTOBJ_DESC*>(pArg);

	m_pParentMatrix = pDesc->pParentWorldMatrix;

	if (FAILED(__super::Initialize(pDesc)))
		return E_FAIL;

	return S_OK;
}

void CPartObject::Priority_Update(_float fTimeDelta)
{
}

_int CPartObject::Update(_float fTimeDelta)
{

	return OBJ_NOEVENT;
}

void CPartObject::Late_Update(_float fTimeDelta)
{
}

HRESULT CPartObject::Render()
{
	return S_OK;
}

HRESULT CPartObject::Bind_WorldMatrix(CShader * pShader, const _char * pContantName)
{
	return pShader->Bind_Matrix(pContantName, &m_WorldMatrix);	
}

void CPartObject::Free()
{
	__super::Free();

}
