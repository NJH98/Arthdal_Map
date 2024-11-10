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
	MAPOBJECT_DESC MapDesc{};
	if (pArg != nullptr){
		MAPOBJECT_DESC* pDesc = static_cast<MAPOBJECT_DESC*>(pArg);
		MapDesc = *pDesc;
	}

	m_iUseModel = MapDesc.ModelNum;

	// TransformCom 셋팅
	if (FAILED(__super::Initialize(nullptr)))
		return E_FAIL;
	// 이외 Components 셋팅
	if (FAILED(Ready_Components()))
		return E_FAIL;
	
	// 위치,회전,크기 조정
	m_pTransformCom->Set_WorldMatrix(MapDesc.WorldMatrix);

	// 피킹객체를 알기위한 DepthNum 지정
	list<CGameObject*>* GameObjectLayer = m_pGameInstance->Get_ObjectList(LEVEL_GAMEPLAY, MapDesc.LayerTag);
	if (GameObjectLayer != nullptr) {
		m_iDepthNum = _uint(GameObjectLayer->size());
	}
	else
		m_iDepthNum = 0;

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

	if(m_bIsRenderInstance)
		m_pGameInstance->Push_Instance_Object(m_InstnaceLayer, this);
	else
		m_pGameInstance->Add_RenderObject(CRenderer::RG_NONBLEND, this);
}

HRESULT CMapObject_Default::Render()
{
	if (FAILED(m_pTransformCom->Bind_ShaderResource(m_pSubShaderCom, "g_WorldMatrix")))
		return E_FAIL;

	if (FAILED(m_pSubShaderCom->Bind_Matrix("g_ViewMatrix", &m_pGameInstance->Get_Transform_Float4x4(CPipeLine::D3DTS_VIEW))))
		return E_FAIL;
	if (FAILED(m_pSubShaderCom->Bind_Matrix("g_ProjMatrix", &m_pGameInstance->Get_Transform_Float4x4(CPipeLine::D3DTS_PROJ))))
		return E_FAIL;

	// 깊이 번호 지정
	if (FAILED(m_pSubShaderCom->Bind_RawValue("g_DepthNum", &m_iDepthNum, sizeof(_uint))))
		return E_FAIL;

	_uint		iNumMeshes = m_pModelCom->Get_NumMeshes();

	for (_uint i = 0; i < iNumMeshes; i++)
	{
		if (FAILED(m_pModelCom->Bind_Material(m_pSubShaderCom, "g_DiffuseTexture", aiTextureType_DIFFUSE, i)))
			return E_FAIL;

		if (FAILED(m_pModelCom->Bind_Material(m_pSubShaderCom, "g_NormalTexture", aiTextureType_NORMALS, i)))
			return E_FAIL;

		if (FAILED(m_pSubShaderCom->Begin(m_iUseShader)))
			return E_FAIL;

		if (FAILED(m_pModelCom->Render(i)))
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

	_uint		iNumMeshes = m_pModelCom->Get_NumMeshes();

	for (_uint i = 0; i < iNumMeshes; i++)
	{
		m_pModelCom->Bind_MeshBoneMatrices(m_pShaderCom, "g_BoneMatrices", i);

		if (FAILED(m_pModelCom->Bind_Material(m_pShaderCom, "g_DiffuseTexture", aiTextureType_DIFFUSE, i)))
			return E_FAIL;
		if (FAILED(m_pShaderCom->Begin(1)))
			return E_FAIL;
		if (FAILED(m_pModelCom->Render(i)))
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
	if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Shader_VtxModel"),
		TEXT("Com_SubShader"), reinterpret_cast<CComponent**>(&m_pSubShaderCom))))
		return E_FAIL;

#pragma region 추가 모델

	/* FOR.Com_Model */
	switch (m_iUseModel)
	{
	case MAP_MODEL_ForkLift:
		if (FAILED(__super::Add_Component(LEVEL_GAMEPLAY, TEXT("Prototype_Component_Model_ForkLift"),
			TEXT("Com_Model"), reinterpret_cast<CComponent**>(&m_pModelCom))))
			return E_FAIL;
		m_InstnaceLayer = TEXT("Prototype_Component_Model_ForkLift");
		break;
	case Map_MODEL_AgoVillage_Boss:
		if (FAILED(__super::Add_Component(LEVEL_GAMEPLAY, TEXT("Prototype_Component_Model_AgoVillage_Boss"),
			TEXT("Com_Model"), reinterpret_cast<CComponent**>(&m_pModelCom))))
			return E_FAIL;
		m_InstnaceLayer = TEXT("Prototype_Component_Model_AgoVillage_Boss");
		break;
	case Map_MODEL_Plant_Berry:
		if (FAILED(__super::Add_Component(LEVEL_GAMEPLAY, TEXT("Prototype_Component_Model_Plant_Berry"),
			TEXT("Com_Model"), reinterpret_cast<CComponent**>(&m_pModelCom))))
			return E_FAIL;
		m_InstnaceLayer = TEXT("Prototype_Component_Model_Plant_Berry");
		break;
	case Map_MODEL_Plant_Bush:
		if (FAILED(__super::Add_Component(LEVEL_GAMEPLAY, TEXT("Prototype_Component_Model_Plant_Bush"),
			TEXT("Com_Model"), reinterpret_cast<CComponent**>(&m_pModelCom))))
			return E_FAIL;
		m_InstnaceLayer = TEXT("Prototype_Component_Model_Plant_Bush");
		break;
	case Map_MODEL_Plant_Weed:
		if (FAILED(__super::Add_Component(LEVEL_GAMEPLAY, TEXT("Prototype_Component_Model_Plant_Weed"),
			TEXT("Com_Model"), reinterpret_cast<CComponent**>(&m_pModelCom))))
			return E_FAIL;
		m_InstnaceLayer = TEXT("Prototype_Component_Model_Plant_Weed");
		break;
	case Map_MODEL_Tree_Bamboo:
		if (FAILED(__super::Add_Component(LEVEL_GAMEPLAY, TEXT("Prototype_Component_Model_Tree_Bamboo"),
			TEXT("Com_Model"), reinterpret_cast<CComponent**>(&m_pModelCom))))
			return E_FAIL;
		m_InstnaceLayer = TEXT("Prototype_Component_Model_Tree_Bamboo");
		break;
	default:
		break;
	}

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

	Safe_Release(m_pModelCom);
	Safe_Release(m_pShaderCom);
	Safe_Release(m_pSubShaderCom);
}
