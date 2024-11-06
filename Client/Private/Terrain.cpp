#include "stdafx.h"
#include "..\Public\Terrain.h"

#include "GameInstance.h"

CTerrain::CTerrain(ID3D11Device * pDevice, ID3D11DeviceContext * pContext)
	: CGameObject{ pDevice, pContext }
{
}

CTerrain::CTerrain(const CTerrain & Prototype)
	: CGameObject{ Prototype }
{
}

HRESULT CTerrain::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CTerrain::Initialize(void * pArg)
{
	CGameObject::GAMEOBJECT_DESC		Desc{};
	
	/* 직교퉁여을 위한 데이터들을 모두 셋하낟. */
	if (FAILED(__super::Initialize(&Desc)))
		return E_FAIL;

	if (FAILED(Ready_Components(pArg)))
		return E_FAIL;

	for (_uint i = 0; i < 2; i++) {
		m_pTextureCom[TEXTURE_MASK]->Add_MaskTexture();
	}

	return S_OK;
}

void CTerrain::Priority_Update(_float fTimeDelta)
{

}

_int CTerrain::Update(_float fTimeDelta)
{
	if (m_bDead) {
		return OBJ_DEAD;
	}

	if (m_pGameInstance->Get_DIKeyState_Once(DIK_Y)) {
	}

	return OBJ_NOEVENT;
}

void CTerrain::Late_Update(_float fTimeDelta)
{
	__super::Late_Update(fTimeDelta);

	//m_pVIBufferCom->Culling(m_pTransformCom->Get_WorldMatrix());
	//m_pNavigationCom->Update(XMLoadFloat4x4(m_pTransformCom->Get_WorldMatrix_Ptr()));
	
#ifdef _DEBUG
	//m_pGameInstance->Add_DebugObject(m_pNavigationCom);
#endif

	m_pGameInstance->Add_RenderObject(CRenderer::RG_NONBLEND, this);
}

HRESULT CTerrain::Render()
{
	if (FAILED(m_pTransformCom->Bind_ShaderResource(m_pShaderCom, "g_WorldMatrix")))		
		return E_FAIL;

	if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", &m_pGameInstance->Get_Transform_Float4x4(CPipeLine::D3DTS_VIEW))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", &m_pGameInstance->Get_Transform_Float4x4(CPipeLine::D3DTS_PROJ))))
		return E_FAIL;

	if (FAILED(m_pTextureCom[TEXTURE_DIFFUSE]->Bind_ShadeResourcesMask(m_pShaderCom, "g_DiffuseTexture", 7)))
		return E_FAIL;
	if (FAILED(m_pTextureCom[TEXTURE_NORMAL]->Bind_ShadeResourcesMask(m_pShaderCom, "g_NomalTexture", 7)))
		return E_FAIL;
	if (FAILED(m_pTextureCom[TEXTURE_MASK]->Bind_ShadeResourcesMask(m_pShaderCom, "g_MaskTexture", 2)))
		return E_FAIL;

	/*_float3 MousePick = m_pGameInstance->Get_GlobalData()->Pick_Pos;
	if (FAILED(m_pShaderCom->Bind_RawValue("g_MouseWorldPos", &MousePick, sizeof(_float3))))
		return E_FAIL;*/

	if (FAILED(m_pShaderCom->Begin(0)))
		return E_FAIL;
	if (FAILED(m_pVIBufferCom->Bind_Buffers()))
		return E_FAIL;
	if (FAILED(m_pVIBufferCom->Render()))
		return E_FAIL;



	return S_OK;
}

HRESULT CTerrain::Ready_Components(void* pArg)
{
	/* FOR.Com_Shader */
	if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Shader_VtxNorTex"),
		TEXT("Com_Shader"), reinterpret_cast<CComponent**>(&m_pShaderCom))))
		return E_FAIL;

	/* FOR.Com_Texture */
	if (FAILED(__super::Add_Component(LEVEL_GAMEPLAY, TEXT("Prototype_Component_Texture_Terrain"),
		TEXT("Com_Texture"), reinterpret_cast<CComponent**>(&m_pTextureCom[TEXTURE_DIFFUSE]))))
		return E_FAIL;
	/* FOR.Com_Nomal */
	if (FAILED(__super::Add_Component(LEVEL_GAMEPLAY, TEXT("Prototype_Component_Texture_Terrain_Normal"),
		TEXT("Com_Nomal"), reinterpret_cast<CComponent**>(&m_pTextureCom[TEXTURE_NORMAL]))))
		return E_FAIL;

	/* FOR.Com_Brush */
	if (FAILED(__super::Add_Component(LEVEL_GAMEPLAY, TEXT("Prototype_Component_Texture_Brush"),
		TEXT("Com_Brush"), reinterpret_cast<CComponent**>(&m_pTextureCom[TEXTURE_BRUSH]))))
		return E_FAIL;

	/* FOR.Com_Mask */
	if (FAILED(__super::Add_Component(LEVEL_GAMEPLAY, TEXT("Prototype_Component_Texture_Mask"),
		TEXT("Com_Mask"), reinterpret_cast<CComponent**>(&m_pTextureCom[TEXTURE_MASK]))))
		return E_FAIL;

	/* FOR.Com_VIBuffer */
	if (FAILED(__super::Add_Component(LEVEL_GAMEPLAY, TEXT("Prototype_Component_VIBuffer_Terrain"),
		TEXT("Com_VIBuffer"), reinterpret_cast<CComponent**>(&m_pVIBufferCom), pArg)))
		return E_FAIL;

	/* For.Com_Navigation */
	/*if (FAILED(__super::Add_Component(LEVEL_GAMEPLAY, TEXT("Prototype_Component_Navigation"),
		TEXT("Com_Navigation"), reinterpret_cast<CComponent**>(&m_pNavigationCom))))
		return E_FAIL;*/


	return S_OK;
}

CTerrain * CTerrain::Create(ID3D11Device * pDevice, ID3D11DeviceContext * pContext)
{
	CTerrain*		pInstance = new CTerrain(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX(TEXT("Failed to Created : CTerrain"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject * CTerrain::Clone(void * pArg)
{
	CTerrain*		pInstance = new CTerrain(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed to Cloned : CTerrain"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CTerrain::Free()
{
	__super::Free();
	
	Safe_Release(m_pShaderCom);

	for (auto& pTextureCom : m_pTextureCom)
		Safe_Release(pTextureCom);

	Safe_Release(m_pVIBufferCom);
	//Safe_Release(m_pNavigationCom);
}
