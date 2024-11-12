#include "stdafx.h"
#include "..\Public\Player.h"

#include "GameInstance.h"
#include "Particle_Explosion.h"
#include "Body_Player.h"
#include "Weapon.h"
#include "Terrain.h"
#include "Cell.h"

CPlayer::CPlayer(ID3D11Device * pDevice, ID3D11DeviceContext * pContext)
	: CContainerObject { pDevice, pContext }
{
}

CPlayer::CPlayer(const CPlayer & Prototype)
	: CContainerObject { Prototype }
{
}

HRESULT CPlayer::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CPlayer::Initialize(void * pArg)
{
	CGameObject::GAMEOBJECT_DESC		Desc{};

	Desc.fSpeedPerSec = 10.0f;
	Desc.fRotationPerSec = XMConvertToRadians(180.0f);

	if (FAILED(__super::Initialize(&Desc)))
		return E_FAIL;

	if (FAILED(Ready_Components()))
		return E_FAIL;

	/*if (FAILED(Ready_PartObjects()))
		return E_FAIL;*/

	if (FAILED(Ready_Terrain()))
		return E_FAIL;

	m_iState = STATE_IDLE;

	m_pTransformCom->Set_State(CTransform::STATE_POSITION, XMVectorSet(0.1f, 0.f, 0.1f, 1.f));

	return S_OK;
}

void CPlayer::Priority_Update(_float fTimeDelta)
{
	for (auto& pPartObject : m_Parts)
		pPartObject->Priority_Update(fTimeDelta);
}

_int CPlayer::Update(_float fTimeDelta)
{
	if (m_bDead) {
		return OBJ_DEAD;
	}

	if (GetKeyState(VK_DOWN) & 0x8000)
		m_pTransformCom->Go_Backward(fTimeDelta);

	if (GetKeyState(VK_LEFT) & 0x8000)
		m_pTransformCom->Turn(false, true, false, fTimeDelta * -1.f);

	if (GetKeyState(VK_RIGHT) & 0x8000)
		m_pTransformCom->Turn(false, true, false, fTimeDelta);

	if (GetKeyState(VK_UP) & 0x8000)
	{
		m_pTransformCom->Go_Straight(fTimeDelta/*m_pNavigationCom*/);

		if(m_iState & STATE_IDLE)
			m_iState ^= STATE_IDLE;

		m_iState |= STATE_WALK;
	}

	else
	{
		m_iState = STATE_RESET;
		m_iState |= STATE_IDLE;
	}

	Terrain_Landing(m_pTransformCom);

	m_pColliderCom->Update(m_pTransformCom->Get_WorldMatrix_Ptr());


	m_pModelCom->Play_Animation(fTimeDelta);

	for (auto& pPartObject : m_Parts)
		pPartObject->Update(fTimeDelta);


	// 맵툴에서 업데이트된 네비게이션 셀을 적용하기위한 임시 코드
	if (m_pGameInstance->Get_DIKeyState_Once(DIK_RETURN))
	{
		m_pNavigationCom->Clear_Cell();
		CNavigation* pNavigation = static_cast<CTerrain*>(m_pGameInstance->Get_Object(LEVEL_GAMEPLAY, TEXT("Layer_Terrain")))->Get_NavigationCom();
		
		for (auto& iter : pNavigation->Get_vecCell()) 
		{
			Vector3 CellPointA = iter->Get_Point(CCell::POINT_A);
			Vector3 CellPointB = iter->Get_Point(CCell::POINT_B);
			Vector3 CellPointC = iter->Get_Point(CCell::POINT_C);

			m_pNavigationCom->Add_Cell_NoneCheck(CellPointA, CellPointB, CellPointC);
			m_pNavigationCom->SetUp_Neighbors();
		}
	}

	return OBJ_NOEVENT;
}

void CPlayer::Late_Update(_float fTimeDelta)
{
	for (auto& pPartObject : m_Parts)
		pPartObject->Late_Update(fTimeDelta);

	/*if (GetKeyState(VK_LBUTTON) & 0x8000)
	{
		_float3		vPickPos;
		if (true == m_pGameInstance->Picking(&vPickPos))
			m_pTransformCom->Set_State(CTransform::STATE_POSITION, XMVectorSetW(XMLoadFloat3(&vPickPos), 1.f));		
	}*/

	/*static _float fTimeAcc = { 0.f };

	fTimeAcc += fTimeDelta;

	if (fTimeAcc >= 0.05f)
	{*/
	//	_vector vPosition = m_pTransformCom->Get_State(CTransform::STATE_POSITION);

	//	_vector vFixPos = m_pGameInstance->Compute_Height(vPosition, XMMatrixLookAtLH(XMVectorSet(64.f, 30.f, 64.0f, 1.f), XMVectorSet(64.f, 0.f, 64.0f, 1.f), XMVectorSet(0.f, 0.f, 1.f, 0.f)),
	//		XMMatrixOrthographicLH(200.f, 200.f, 0.f, 50.f));

	//	m_pTransformCom->Set_State(CTransform::STATE_POSITION, XMVectorSetY(vPosition, XMVectorGetY(vFixPos)));

	//	/*fTimeAcc = 0.f;
	//}*/


	
	m_pGameInstance->Add_RenderObject(CRenderer::RG_NONBLEND, this);
	m_pGameInstance->Add_RenderObject(CRenderer::RG_SHADOWOBJ, this);
#ifdef _DEBUG
	m_pGameInstance->Add_DebugObject(m_pColliderCom);
	m_pGameInstance->Add_DebugObject(m_pNavigationCom);
#endif
}

HRESULT CPlayer::Render()
{
//#ifdef _DEBUG
//	m_pColliderCom->Render();
//	m_pNavigationCom->Render();
//#endif

	if (FAILED(m_pTransformCom->Bind_ShaderResource(m_pShaderCom, "g_WorldMatrix")))
		return E_FAIL;

	if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", &m_pGameInstance->Get_Transform_Float4x4(CPipeLine::D3DTS_VIEW))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", &m_pGameInstance->Get_Transform_Float4x4(CPipeLine::D3DTS_PROJ))))
		return E_FAIL;

	_uint		iNumMeshes = m_pModelCom->Get_NumMeshes();

	for (size_t i = 0; i < iNumMeshes; i++)
	{
		m_pModelCom->Bind_MeshBoneMatrices(m_pShaderCom, "g_BoneMatrices", _uint(i));

		if (FAILED(m_pModelCom->Bind_Material(m_pShaderCom, "g_DiffuseTexture", aiTextureType_DIFFUSE, _uint(i))))
			return E_FAIL;
		/*if (FAILED(m_pModelCom->Bind_Material(m_pShaderCom, "g_NormalTexture", aiTextureType_NORMALS, i)))
			return E_FAIL;*/

		if (FAILED(m_pShaderCom->Begin(0)))
			return E_FAIL;

		if (FAILED(m_pModelCom->Render(_uint(i))))
			return E_FAIL;
	}

	return S_OK;
}

HRESULT CPlayer::Ready_Terrain()
{
	CTerrain* pTerrain = static_cast<CTerrain*>(m_pGameInstance->Get_Object(LEVEL_GAMEPLAY, TEXT("Layer_Terrain")));
	
	m_pTerrainVIBuffer = pTerrain->Get_VIBuffer();
	m_pTerrainTransform = pTerrain->Get_TranformCom();

	return S_OK;
}

HRESULT CPlayer::Terrain_Landing(CTransform* pTransform, _float fOffsetY)
{
	// 플레이어의 월드 위치
	Vector3 vWorldPos = pTransform->Get_State(CTransform::STATE_POSITION);

	// 지형의 월드행렬,역행렬
	Matrix matTerrainWorld = m_pTerrainTransform->Get_WorldMatrix();
	Matrix matTerrainWorldInverse = m_pTerrainTransform->Get_WorldMatrix_Inverse();

	// 플레이어의 월드*지형역행렬 = 로컬에서 높이 비교
	Vector3 vLocalPos{};
	vLocalPos = XMVector3TransformCoord(vWorldPos, matTerrainWorldInverse);

	// 높이값 계산
	_float fHeight = m_pTerrainVIBuffer->Compute_Height(vLocalPos);

	// offset 만큼 올려서 높이 지정
	vLocalPos.y = fHeight + fOffsetY;

	// 변경된 로컬 위치 * 지형월드행렬 = 월드의 플레이어 위치
	vWorldPos = XMVector3TransformCoord(vLocalPos, matTerrainWorld);

	// 변화된 월드 위치 적용
	pTransform->Set_State(CTransform::STATE_POSITION, vWorldPos);

	return S_OK;
}

HRESULT CPlayer::Ready_Components()
{
	/* For.Com_Navigation */
	CNavigation::NAVIGATION_DESC			NaviDesc{};

	NaviDesc.iCurrentIndex = 0;

	if (FAILED(__super::Add_Component(LEVEL_GAMEPLAY, TEXT("Prototype_Component_Navigation"),
		TEXT("Com_Navigation"), reinterpret_cast<CComponent**>(&m_pNavigationCom), &NaviDesc)))
		return E_FAIL;

	/* For.Com_Collider */
	CBounding_AABB::BOUNDING_AABB_DESC			ColliderDesc{};
	ColliderDesc.vExtents = _float3(0.5f, 1.0f, 0.5f);
	ColliderDesc.vCenter = _float3(0.f, ColliderDesc.vExtents.y, 0.f);

	if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Collider_AABB"),
		TEXT("Com_Collider"), reinterpret_cast<CComponent**>(&m_pColliderCom), &ColliderDesc)))
		return E_FAIL;

	/* FOR.Com_Shader */
	if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Shader_VtxAnimModel"),
		TEXT("Com_Shader"), reinterpret_cast<CComponent**>(&m_pShaderCom))))
		return E_FAIL;

	/* FOR.Com_Model */
	if (FAILED(__super::Add_Component(LEVEL_GAMEPLAY, TEXT("Prototype_Component_Model_Fiona"),
		TEXT("Com_Model"), reinterpret_cast<CComponent**>(&m_pModelCom))))
		return E_FAIL;
	

	return S_OK;
}

HRESULT CPlayer::Ready_PartObjects()
{
	/* 실제 추가하고 싶은 파트오브젝트의 갯수만큼 밸류를 셋팅해놓자. */
	m_Parts.resize(PART_END);

	CBody_Player::BODY_DESC		BodyDesc{};
	BodyDesc.pParentState = &m_iState;
	BodyDesc.pParentWorldMatrix = m_pTransformCom->Get_WorldMatrix_Ptr();
	if (FAILED(__super::Add_PartObject(PART_BODY, TEXT("Prototype_GameObject_Body_Player"), &BodyDesc)))
		return E_FAIL;

	CWeapon::WEAPON_DESC		WeaponDesc{};
	WeaponDesc.pParentState = &m_iState;
	WeaponDesc.pParentWorldMatrix = m_pTransformCom->Get_WorldMatrix_Ptr();
	WeaponDesc.pSocketBoneMatrix = dynamic_cast<CBody_Player*>(m_Parts[PART_BODY])->Get_BoneMatrix_Ptr("SWORD");

	if (FAILED(__super::Add_PartObject(PART_WEAPON, TEXT("Prototype_GameObject_Weapon"), &WeaponDesc)))
		return E_FAIL;

	CParticle_Explosion::EFFECT_DESC		EffectDesc{};
	EffectDesc.pParentState = &m_iState;
	EffectDesc.pParentWorldMatrix = m_pTransformCom->Get_WorldMatrix_Ptr();
	EffectDesc.pSocketBoneMatrix = dynamic_cast<CBody_Player*>(m_Parts[PART_BODY])->Get_BoneMatrix_Ptr("SWORD");

	if (FAILED(__super::Add_PartObject(PART_EFFECT, TEXT("Prototype_GameObject_Particle_Explosion"), &EffectDesc)))
		return E_FAIL;

	return S_OK;
}

CPlayer * CPlayer::Create(ID3D11Device * pDevice, ID3D11DeviceContext * pContext)
{
	CPlayer*		pInstance = new CPlayer(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX(TEXT("Failed to Created : CPlayer"));
		Safe_Release(pInstance);
	}

	return pInstance;
}



CGameObject * CPlayer::Clone(void * pArg)
{
	CPlayer*		pInstance = new CPlayer(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed to Cloned : CPlayer"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CPlayer::Free()
{
	__super::Free();

	Safe_Release(m_pColliderCom);
	Safe_Release(m_pNavigationCom);
	Safe_Release(m_pShaderCom);
	Safe_Release(m_pModelCom);
	Safe_Release(m_pTerrainVIBuffer);
	Safe_Release(m_pTerrainTransform);
}
