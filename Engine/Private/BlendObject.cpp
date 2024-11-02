#include "..\Public\BlendObject.h"


CBlendObject::CBlendObject(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CGameObject { pDevice, pContext }
{
}

CBlendObject::CBlendObject(const CBlendObject & Prototype)
	: CGameObject{ Prototype }
{
}

HRESULT CBlendObject::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CBlendObject::Initialize(void * pArg)
{

	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	return S_OK;
}

void CBlendObject::Priority_Update(_float fTimeDelta)
{
}

_int CBlendObject::Update(_float fTimeDelta)
{

	return OBJ_NOEVENT;
}

void CBlendObject::Late_Update(_float fTimeDelta)
{
}

HRESULT CBlendObject::Render()
{
	return S_OK;
}

HRESULT CBlendObject::Compute_ViewZ(const _wstring & strTransformComTag)
{
	/*CTransform*		pTransformCom = dynamic_cast<CTransform*>(Find_Component(strTransformComTag));
	if (nullptr == pTransformCom)
		return E_FAIL;

	_float3			vWorldPos = pTransformCom->Get_State(CTransform::STATE_POSITION);

	_float4x4		ViewMatrix{};

	m_pGraphic_Device->GetTransform(D3DTS_VIEW, &ViewMatrix);

	m_fViewZ = (*D3DXVec3TransformCoord(&vWorldPos, &vWorldPos, &ViewMatrix)).z;
*/
	return S_OK;
}

void CBlendObject::Free()
{
	__super::Free();

}
