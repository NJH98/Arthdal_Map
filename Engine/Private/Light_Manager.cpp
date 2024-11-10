#include "..\Public\Light_Manager.h"
#include "Light.h"

CLight_Manager::CLight_Manager()
{
}

LIGHT_DESC * CLight_Manager::Get_LightDesc(_uint iIndex)
{
	if (m_Lights.size() < iIndex)
		return nullptr;

	auto	iter = m_Lights.begin();

	for (size_t i = 0; i < iIndex; i++)
		++iter;	

	return (*iter)->Get_LightDesc();
}

CLight* CLight_Manager::Get_Light(_uint iIndex)
{
	if (m_Lights.size() < iIndex)
		return nullptr;

	auto	iter = m_Lights.begin();

	for (size_t i = 0; i < iIndex; i++)
		++iter;

	return *iter;
}

void CLight_Manager::Delete_Light(_uint iIndex)
{
	if (m_Lights.size() < iIndex)
		return;

	auto	iter = m_Lights.begin();

	for (size_t i = 0; i < iIndex; i++)
		++iter;

	Safe_Release(*iter);
	m_Lights.erase(iter);
}

void CLight_Manager::Clear_Light()
{
	for (auto& pLight : m_Lights)
		Safe_Release(pLight);

	m_Lights.clear();
}

HRESULT CLight_Manager::Initialize()
{

	return S_OK;
}

HRESULT CLight_Manager::Add_Light(const LIGHT_DESC & LightDesc)
{
	CLight*		pLight = CLight::Create(LightDesc);
	if (nullptr == pLight)
		return E_FAIL;

	m_Lights.push_back(pLight);

	return S_OK;
}

HRESULT CLight_Manager::Render(CShader * pShader, CVIBuffer_Rect * pVIBuffer)
{
	for (auto& pLight : m_Lights)
		pLight->Render(pShader, pVIBuffer);

	return S_OK;
}

CLight_Manager * CLight_Manager::Create()
{
	CLight_Manager*		pInstance = new CLight_Manager();

	if (FAILED(pInstance->Initialize()))
	{
		MSG_BOX(TEXT("Failed to Cloned : CLight_Manager"));
		Safe_Release(pInstance);
	}

	return pInstance;
}


void CLight_Manager::Free()
{
	__super::Free();

	for (auto& pLight : m_Lights)
		Safe_Release(pLight);

	m_Lights.clear();	
}