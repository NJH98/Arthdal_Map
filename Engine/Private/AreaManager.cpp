#include "AreaManager.h"
#include "GameInstance.h"

CAreaManager::CAreaManager()
    : m_pGameInstance{ CGameInstance::Get_Instance() }
{
    Safe_AddRef(m_pGameInstance);
}

void CAreaManager::Update()
{
	m_vPlayerPos = m_pGameInstance->Get_GlobalData()->My_Pos;

	_uint PlayerX{}, PlayerY{};

	PlayerX = _uint(m_vPlayerPos.x) / m_iAreaValue;
	PlayerY = _uint(m_vPlayerPos.z) / m_iAreaValue;

	m_pPlayerCurrentArea = PlayerX + (PlayerY * m_iDivideArea);

	if (m_pPlayerPreArea != m_pPlayerCurrentArea) {
		RenderAreaChange = true;
		RenderAreaSet();
		m_pPlayerPreArea = m_pPlayerCurrentArea;
	}
	else {
		RenderAreaChange = false;
	}
}

void CAreaManager::RenderAreaSet()
{
	/*
		3	*	3
		5	*	5 << 현제 사용중
		7	*	7
	*/

	_int ArrayNum = 0;

	for (_int y = -2; y < 3; y++) {
		for (_int x = -2; x < 3; x++) {
			m_pRenderArea[ArrayNum] = m_pPlayerCurrentArea + (m_iDivideArea * y) + x;
			ArrayNum++;
		}
	}
}

_uint CAreaManager::AreaIndexSet(_float3 Pos)
{
	_uint PosX{}, PosZ{};

	PosX = _uint(Pos.x) / m_iAreaValue;
	PosZ = _uint(Pos.z) / m_iAreaValue;

	return (PosX + (PosZ * m_iDivideArea));
}

_bool CAreaManager::IsInRenderArea(_int AreaIndex)
{
	for (_uint i = 0; i < MAX_AREA; i++) {
		if (m_pRenderArea[i] == AreaIndex) {
			return true;
		}
	}

	return false;
}

void CAreaManager::Free()
{
	Safe_Release(m_pGameInstance);
}
