#include "stdafx.h"

#include "MapObject_Default.h"
#include "GameInstance.h"

CMapObject_Default::CMapObject_Default(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	:CMapObject{ pDevice, pContext }
{
}

CMapObject_Default::CMapObject_Default(const CMapObject_Default& Prototype)
	:CMapObject(Prototype)
{
}

HRESULT CMapObject_Default::Initialize_Prototype()
{
	if (FAILED(__super::Initialize_Prototype()))
		return E_FAIL;

	return S_OK;
}

HRESULT CMapObject_Default::Initialize(void* pArg)
{
	if (pArg != nullptr)
	{
		CGameObject::GAMEOBJECT_DESC* pDesc = static_cast<CGameObject::GAMEOBJECT_DESC*>(pArg);
		m_GameObjDesc = *pDesc;
	}

	// TransformCom ����
	if (FAILED(__super::Initialize(nullptr)))
		return E_FAIL;
	// �̿� Components ����
	if (FAILED(Ready_Components()))
		return E_FAIL;

	m_pTransformCom->Set_Scaled(m_GameObjDesc.Scale.x, m_GameObjDesc.Scale.y, m_GameObjDesc.Scale.z);
	m_pTransformCom->All_Rotation(m_GameObjDesc.Angle);
	m_pTransformCom->Set_State(CTransform::STATE_POSITION, XMLoadFloat3(&m_GameObjDesc.Pos));

	return S_OK;
}

void CMapObject_Default::Priority_Update(_float fTimeDelta)
{
}

_int CMapObject_Default::Update(_float fTimeDelta)
{
	if (m_bDead)
		return OBJ_DEAD;

	return OBJ_NOEVENT;
}

void CMapObject_Default::Late_Update(_float fTimeDelta)
{
	m_pGameInstance->Add_RenderObject(CRenderer::RG_NONBLEND, this);
	//m_pGameInstance->Add_RenderObject(CRenderer::RG_SHADOWOBJ, this);
}

HRESULT CMapObject_Default::Render()
{
	if (FAILED(m_pTransformCom->Bind_ShaderResource(m_pShaderCom, "g_WorldMatrix")))
		return E_FAIL;

	if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", &m_pGameInstance->Get_Transform_Float4x4(CPipeLine::D3DTS_VIEW))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", &m_pGameInstance->Get_Transform_Float4x4(CPipeLine::D3DTS_PROJ))))
		return E_FAIL;

	_uint		iNumMeshes = m_pModelCom[m_iUseModel]->Get_NumMeshes();

	for (_uint i = 0; i < iNumMeshes; i++)
	{
		m_pModelCom[m_iUseModel]->Bind_MeshBoneMatrices(m_pShaderCom, "g_BoneMatrices", i);

		if (FAILED(m_pModelCom[m_iUseModel]->Bind_Material(m_pShaderCom, "g_DiffuseTexture", aiTextureType_DIFFUSE, i)))
			return E_FAIL;

		/*if (FAILED(m_pModelCom->Bind_Material(m_pShaderCom, "g_NormalTexture", aiTextureType_NORMALS, i)))
			return E_FAIL;*/

		if (FAILED(m_pShaderCom->Begin(1)))
			return E_FAIL;

		if (FAILED(m_pModelCom[m_iUseModel]->Render(i)))
			return E_FAIL;
	}
	return S_OK;
}

HRESULT CMapObject_Default::Render_LightDepth()
{
	if (FAILED(m_pTransformCom->Bind_ShaderResource(m_pShaderCom, "g_WorldMatrix")))
		return E_FAIL;

	_float4x4		ViewMatrix;
	XMStoreFloat4x4(&ViewMatrix, XMMatrixLookAtLH(XMVectorSet(0.f, 20.f, -15.f, 1.f), XMVectorSet(0.f, 0.f, 0.f, 1.f), XMVectorSet(0.f, 1.f, 0.f, 0.f)));

	if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", &ViewMatrix)))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", &m_pGameInstance->Get_Transform_Float4x4(CPipeLine::D3DTS_PROJ))))
		return E_FAIL;

	_uint		iNumMeshes = m_pModelCom[m_iUseModel]->Get_NumMeshes();

	for (_uint i = 0; i < iNumMeshes; i++)
	{
		m_pModelCom[m_iUseModel]->Bind_MeshBoneMatrices(m_pShaderCom, "g_BoneMatrices", i);

		if (FAILED(m_pModelCom[m_iUseModel]->Bind_Material(m_pShaderCom, "g_DiffuseTexture", aiTextureType_DIFFUSE, i)))
			return E_FAIL;
		if (FAILED(m_pShaderCom->Begin(1)))
			return E_FAIL;
		if (FAILED(m_pModelCom[m_iUseModel]->Render(i)))
			return E_FAIL;
	}

	return S_OK;
}

HRESULT CMapObject_Default::Ready_Components()
{
	/* FOR.Com_Shader */
	if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Shader_VtxModel"),
		TEXT("Com_Shader"), reinterpret_cast<CComponent**>(&m_pShaderCom))))
		return E_FAIL;

	/* FOR.Com_Model */
	if (FAILED(__super::Add_Component(LEVEL_GAMEPLAY, TEXT("Prototype_Component_Model_ForkLift"),
		TEXT("Com_Model"), reinterpret_cast<CComponent**>(&m_pModelCom[MAP_MODEL_TEST]))))
		return E_FAIL;
	m_SModelName[MAP_MODEL_TEST] = "Prototype_Component_Model_ForkLift";

	return S_OK;
}

CMapObject_Default* CMapObject_Default::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CMapObject_Default* pInstance = new CMapObject_Default(pDevice, pContext);
	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX(TEXT("Failed To Created : CMapObject_Default"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CMapObject_Default::Clone(void* pArg)
{
	CMapObject_Default* pInstance = new CMapObject_Default(*this);
	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Cloned : CMapObject_Default"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CMapObject_Default::Free()
{
	__super::Free();

	for (_uint i = 0; i < MAP_MODEL_END; i++) {
		Safe_Release(m_pModelCom[i]);
	}

	Safe_Release(m_pShaderCom);
}