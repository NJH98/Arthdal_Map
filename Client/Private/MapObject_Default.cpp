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

	// TransformCom 셋팅
	if (FAILED(__super::Initialize(nullptr)))
		return E_FAIL;
	// 이외 Components 셋팅
	if (FAILED(Ready_Components()))
		return E_FAIL;
	
	// 위치,회전,크기 조정
	m_pTransformCom->Set_Scaled(m_GameObjDesc.Scale.x, m_GameObjDesc.Scale.y, m_GameObjDesc.Scale.z);
	m_pTransformCom->All_Rotation(m_GameObjDesc.Angle);
	m_pTransformCom->Set_State(CTransform::STATE_POSITION, XMLoadFloat3(&m_GameObjDesc.Pos));

	// 사용모델 번호, 깊이번호 저장
	m_iUseModel = m_GameObjDesc.ModelNum;

	list<CGameObject*>* GameObjectLayer = m_pGameInstance->Get_ObjectList(LEVEL_GAMEPLAY, m_GameObjDesc.LayerTag);
	if (GameObjectLayer != nullptr) {
		m_DepthNum = _float(GameObjectLayer->size());
	}
	else
		m_DepthNum = 0;
	
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
	//m_pGameInstance->Add_RenderObject(CRenderer::RG_NONBLEND, this);
	//m_pGameInstance->Add_RenderObject(CRenderer::RG_SHADOWOBJ, this);
	m_pGameInstance->Push_Instance_Object(TEXT("Layer_Test"), this);
}

HRESULT CMapObject_Default::Render()
{
	if (FAILED(m_pTransformCom->Bind_ShaderResource(m_pShaderCom, "g_WorldMatrix")))
		return E_FAIL;

	if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", &m_pGameInstance->Get_Transform_Float4x4(CPipeLine::D3DTS_VIEW))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", &m_pGameInstance->Get_Transform_Float4x4(CPipeLine::D3DTS_PROJ))))
		return E_FAIL;

	// 깊이 번호 지정
	if (FAILED(m_pShaderCom->Bind_RawValue("g_DepthNum", &m_DepthNum, sizeof(_float))))
		return E_FAIL;

	_uint		iNumMeshes = m_pModelCom[m_iUseModel]->Get_NumMeshes();

	for (_uint i = 0; i < iNumMeshes; i++)
	{
		if (FAILED(m_pModelCom[m_iUseModel]->Bind_Material(m_pShaderCom, "g_DiffuseTexture", aiTextureType_DIFFUSE, i)))
			return E_FAIL;

		if (FAILED(m_pModelCom[m_iUseModel]->Bind_Material(m_pShaderCom, "g_NormalTexture", aiTextureType_NORMALS, i)))
			return E_FAIL;

		if (FAILED(m_pShaderCom->Begin(m_iUseShader)))
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
	if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Shader_VtxMeshInstance"),
		TEXT("Com_Shader"), reinterpret_cast<CComponent**>(&m_pShaderCom))))
		return E_FAIL;

#pragma region 추가 모델

	/* FOR.Com_Model */
	/*if (FAILED(__super::Add_Component(LEVEL_GAMEPLAY, TEXT("Prototype_Component_Model_ForkLift"),
		TEXT("Com_MODEL_ForkLift"), reinterpret_cast<CComponent**>(&m_pModelCom[MAP_MODEL_ForkLift]))))
		return E_FAIL;
	
	if (FAILED(__super::Add_Component(LEVEL_GAMEPLAY, TEXT("Prototype_Component_Model_AgoVillage_Boss"),
		TEXT("Com_MODEL_AgoVillage_Boss"), reinterpret_cast<CComponent**>(&m_pModelCom[Map_MODEL_AgoVillage_Boss]))))
		return E_FAIL;

	if (FAILED(__super::Add_Component(LEVEL_GAMEPLAY, TEXT("Prototype_Component_Model_Plant_Berry"),
		TEXT("Com_MODEL_Plant_Berry"), reinterpret_cast<CComponent**>(&m_pModelCom[Map_MODEL_Plant_Berry]))))
		return E_FAIL;

	if (FAILED(__super::Add_Component(LEVEL_GAMEPLAY, TEXT("Prototype_Component_Model_Plant_Bush"),
		TEXT("Com_MODEL_Plant_Bush"), reinterpret_cast<CComponent**>(&m_pModelCom[Map_MODEL_Plant_Bush]))))
		return E_FAIL;*/

	if (FAILED(__super::Add_Component(LEVEL_GAMEPLAY, TEXT("Prototype_Component_Model_Plant_Weed"),
		TEXT("Com_Model"), reinterpret_cast<CComponent**>(&m_pModelCom[Map_MODEL_Plant_Weed]))))
		return E_FAIL;

	/*if (FAILED(__super::Add_Component(LEVEL_GAMEPLAY, TEXT("Prototype_Component_Model_Tree_Bamboo"),
		TEXT("Com_MODEL_Tree_Bamboo"), reinterpret_cast<CComponent**>(&m_pModelCom[Map_MODEL_Tree_Bamboo]))))
		return E_FAIL;*/

#pragma endregion

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
