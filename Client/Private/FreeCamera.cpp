#include "stdafx.h"
#include "..\Public\FreeCamera.h"

#include "GameInstance.h"

CFreeCamera::CFreeCamera(ID3D11Device * pDevice, ID3D11DeviceContext * pContext)
	: CCamera { pDevice, pContext }
{
}

CFreeCamera::CFreeCamera(const CFreeCamera & Prototype)
	: CCamera{ Prototype }
{
}

HRESULT CFreeCamera::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CFreeCamera::Initialize(void * pArg)
{
	CAMERA_FREE_DESC*		pDesc = static_cast<CAMERA_FREE_DESC*>(pArg);

	m_fSensor = pDesc->fSensor;

	if (FAILED(__super::Initialize(pDesc)))
		return E_FAIL;

	return S_OK;
}

void CFreeCamera::Priority_Update(_float fTimeDelta)
{
	_float Speed = 1.f;
	if (m_pGameInstance->Get_DIKeyState(DIK_LSHIFT))
		Speed = 3.f;

	if (m_pGameInstance->Get_DIKeyState(DIK_W) & 0x80)
		m_pTransformCom->Go_Straight(fTimeDelta * Speed);
	if (m_pGameInstance->Get_DIKeyState(DIK_S) & 0x80)
		m_pTransformCom->Go_Backward(fTimeDelta * Speed);
	if (m_pGameInstance->Get_DIKeyState(DIK_A) & 0x80)
		m_pTransformCom->Go_Left(fTimeDelta * Speed);
	if (m_pGameInstance->Get_DIKeyState(DIK_D) & 0x80)
		m_pTransformCom->Go_Right(fTimeDelta * Speed);

	_long		MouseMove = { 0 };

	if (m_pGameInstance->Get_DIMouseState(DIMK_RBUTTON) & 0x80)
	{
		if (MouseMove = m_pGameInstance->Get_DIMouseMove(DIMM_X)){
			m_pTransformCom->Turn(XMVectorSet(0.f, 1.f, 0.f, 0.f), fTimeDelta * MouseMove * m_fSensor);
		}

		if (MouseMove = m_pGameInstance->Get_DIMouseMove(DIMM_Y)){
			m_pTransformCom->Turn(m_pTransformCom->Get_State(CTransform::STATE_RIGHT), fTimeDelta * MouseMove * m_fSensor);
		}
	}

	__super::Priority_Update(fTimeDelta);
}

_int CFreeCamera::Update(_float fTimeDelta)
{

	return OBJ_NOEVENT;
}

void CFreeCamera::Late_Update(_float fTimeDelta)
{
}

HRESULT CFreeCamera::Render()
{
	return S_OK;
}

CFreeCamera * CFreeCamera::Create(ID3D11Device * pDevice, ID3D11DeviceContext * pContext)
{
	CFreeCamera*		pInstance = new CFreeCamera(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX(TEXT("Failed to Created : CFreeCamera"));
		Safe_Release(pInstance);
	}

	return pInstance;
}



CGameObject * CFreeCamera::Clone(void * pArg)
{
	CFreeCamera*		pInstance = new CFreeCamera(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed to Cloned : CFreeCamera"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CFreeCamera::Free()
{
	__super::Free();

}
