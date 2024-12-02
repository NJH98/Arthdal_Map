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
	m_fRadiuse = MapDesc.CullRadiuse;

	// TransformCom 셋팅
	if (FAILED(__super::Initialize(nullptr)))
		return E_FAIL;
	// 이외 Components 셋팅
	if (FAILED(Ready_Components()))
		return E_FAIL;
	
	// 위치,회전,크기 조정
	m_pTransformCom->Set_WorldMatrix(MapDesc.WorldMatrix);

	_float TurnX = XMConvertToDegrees(asin(MapDesc.WorldMatrix._32));
	_float TurnY = XMConvertToDegrees(atan2(MapDesc.WorldMatrix._31, MapDesc.WorldMatrix._33));
	_float TurnZ = XMConvertToDegrees(atan2(MapDesc.WorldMatrix._12, MapDesc.WorldMatrix._22));

	m_GameObjDesc.Angle = _float3(TurnX, TurnY, TurnZ);

	// 피킹객체를 알기위한 DepthNum 지정
	list<CGameObject*>* GameObjectLayer = m_pGameInstance->Get_ObjectList(LEVEL_GAMEPLAY, MapDesc.LayerTag);
	if (GameObjectLayer != nullptr) {
		m_iDepthNum = _uint(GameObjectLayer->size());
	}
	else
		m_iDepthNum = 0;


	Vector3 Pos = m_pTransformCom->Get_State(CTransform::STATE_POSITION);
	m_iArea = m_pGameInstance->AreaIndexSet(Pos);

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

	if (m_pGameInstance->Get_RenderAreaChange()) {
		m_bIsCalling = m_pGameInstance->IsInRenderArea(m_iArea, CAreaManager::AREA_5X5);
	}

	if (m_bIsCalling == false)
		return;

	Vector3 Pos = m_pTransformCom->Get_State(CTransform::STATE_POSITION);

	if (m_pGameInstance->isIn_Frustum_WorldSpace(Pos, m_fRadiuse)) {
		if (m_bIsRenderInstance)
			m_pGameInstance->Push_Instance_Object(m_InstnaceLayer, this);
		else
			m_pGameInstance->Add_RenderObject(CRenderer::RG_NONBLEND, this);
	}

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

	if (m_iUseModel < Map_MODEL_ObjectEnd) {
		_wstring PrototypeTag = L"Prototype_Component_Model_Map_Object" + to_wstring(m_iUseModel);

		if (FAILED(__super::Add_Component(LEVEL_GAMEPLAY, PrototypeTag,
			TEXT("Com_Model"), reinterpret_cast<CComponent**>(&m_pModelCom))))
			return E_FAIL;
		m_InstnaceLayer = PrototypeTag;
	}
	else if (m_iUseModel < Map_MODEL_PlantEnd) {
		_wstring PrototypeTag = L"Prototype_Component_Model_Map_Plant" + to_wstring(m_iUseModel - Map_MODEL_ObjectEnd);

		if (FAILED(__super::Add_Component(LEVEL_GAMEPLAY, PrototypeTag,
			TEXT("Com_Model"), reinterpret_cast<CComponent**>(&m_pModelCom))))
			return E_FAIL;
		m_InstnaceLayer = PrototypeTag;
	}
	else if (m_iUseModel < Map_MODEL_TreeEnd) {
		_wstring PrototypeTag = L"Prototype_Component_Model_Map_Tree" + to_wstring(m_iUseModel - Map_MODEL_PlantEnd);

		if (FAILED(__super::Add_Component(LEVEL_GAMEPLAY, PrototypeTag,
			TEXT("Com_Model"), reinterpret_cast<CComponent**>(&m_pModelCom))))
			return E_FAIL;
		m_InstnaceLayer = PrototypeTag;
	}
	else if (m_iUseModel < Map_MODEL_HouseEnd) {
		_wstring PrototypeTag = L"Prototype_Component_Model_Map_House" + to_wstring(m_iUseModel - Map_MODEL_TreeEnd);

		if (FAILED(__super::Add_Component(LEVEL_GAMEPLAY, PrototypeTag,
			TEXT("Com_Model"), reinterpret_cast<CComponent**>(&m_pModelCom))))
			return E_FAIL;
		m_InstnaceLayer = PrototypeTag;
	}
	else if (m_iUseModel < Map_MODEL_GuildEnd) {
		_wstring PrototypeTag = L"Prototype_Component_Model_Map_Guild" + to_wstring(m_iUseModel - Map_MODEL_HouseEnd);

		if (FAILED(__super::Add_Component(LEVEL_GAMEPLAY, PrototypeTag,
			TEXT("Com_Model"), reinterpret_cast<CComponent**>(&m_pModelCom))))
			return E_FAIL;
		m_InstnaceLayer = PrototypeTag;
	}
	else if (m_iUseModel < Map_MODEL_CommonEnd) {
		_wstring PrototypeTag = L"Prototype_Component_Model_Map_Common" + to_wstring(m_iUseModel - Map_MODEL_GuildEnd);

		if (FAILED(__super::Add_Component(LEVEL_GAMEPLAY, PrototypeTag,
			TEXT("Com_Model"), reinterpret_cast<CComponent**>(&m_pModelCom))))
			return E_FAIL;
		m_InstnaceLayer = PrototypeTag;
	}
	else if (m_iUseModel < Map_MODEL_LampEnd) {
		_wstring PrototypeTag = L"Prototype_Component_Model_Map_Lamp" + to_wstring(m_iUseModel - Map_MODEL_CommonEnd);

		if (FAILED(__super::Add_Component(LEVEL_GAMEPLAY, PrototypeTag,
			TEXT("Com_Model"), reinterpret_cast<CComponent**>(&m_pModelCom))))
			return E_FAIL;
		m_InstnaceLayer = PrototypeTag;
	}

#pragma endregion

	m_fRadiuse = m_pModelCom->Get_CullRadius();

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
