#include "AreaManager.h"
#include "GameInstance.h"

CAreaManager::CAreaManager()
    : m_pGameInstance{ CGameInstance::Get_Instance() }
{
    Safe_AddRef(m_pGameInstance);
}

void CAreaManager::Update()
{
	// 객체의 현제 위치를 계산
	m_vPlayerPos = m_pGameInstance->Get_GlobalData()->My_Pos;

	_uint PlayerX{}, PlayerY{};

	PlayerX = _uint(m_vPlayerPos.x) / m_iAreaValue;
	PlayerY = _uint(m_vPlayerPos.z) / m_iAreaValue;

	m_pPlayerCurrentArea = PlayerX + (PlayerY * m_iDivideArea);

	/* 
		플레이어블 객체의 지역이 변화되어진 순간 ( 이전 지역과 달라지는 순간 )
		지역이 변화되었음을 알리고 RenderAreaChange
		지역들을 갱신
		현제 지역으로 값 대입
	*/
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
		플레이어의 주변 지역들을 갱신시키는 함수
	*/

	_int ArrayNum = 0;

	for (_int y = -4; y < 5; y++) {
		for (_int x = -4; x < 5; x++) {
			m_pRenderArea[ArrayNum] = m_pPlayerCurrentArea + (m_iDivideArea * y) + x;
			ArrayNum++;
		}
	}
}

_uint CAreaManager::AreaIndexSet(_float3 Pos)
{
	// 객체의 자신의 지역 번호를 셋팅하는 함수

	_uint PosX{}, PosZ{};

	PosX = _uint(Pos.x) / m_iAreaValue;
	PosZ = _uint(Pos.z) / m_iAreaValue;

	return (PosX + (PosZ * m_iDivideArea));
}

_bool CAreaManager::IsInRenderArea(_int AreaIndex, AREADATA ChoiceArea)
{
	// 객체가 자신의 지역이 현제 플레이어주변 지역이 맞는지 확인하는 함수

	switch (ChoiceArea)
	{
	case AREA_3X3:

		for (_int areaZ = -1; areaZ < 2; areaZ++) {
			for (_int areaX = -1; areaX < 2; areaX++) {
				if (m_pRenderArea[m_iRenderAreaCenter + (m_iDivideRenderArea * areaZ) + areaX] == AreaIndex) {
					return true;
				}
			}
		}

		break;

	case AREA_5X5:
		
		for (_int areaZ = -2; areaZ < 3; areaZ++) {
			for (_int areaX = -2; areaX < 3; areaX++) {
				if (m_pRenderArea[m_iRenderAreaCenter + (m_iDivideRenderArea * areaZ) + areaX] == AreaIndex) {
					return true;
				}
			}
		}
		
		break;

	case AREA_7X7:
	
		for (_int areaZ = -3; areaZ < 4; areaZ++) {
			for (_int areaX = -3; areaX < 4; areaX++) {
				if (m_pRenderArea[m_iRenderAreaCenter + (m_iDivideRenderArea * areaZ) + areaX] == AreaIndex) {
					return true;
				}
			}
		}
		
		break;
	
	case AREA_9X9:
		
		for (_int i = 0; i < AREA_9X9; i++) {
			if (m_pRenderArea[i] == AreaIndex) {
				return true;
			}
		}
		
		break;
	default:
		break;
	}

	return false;
}

vector<_uint> CAreaManager::Get_RenderArea(AREADATA ChoiceArea)
{
	vector<_uint> vecRenderArea;

	switch (ChoiceArea)
	{
	case AREA_3X3:

		for (_int areaZ = -1; areaZ < 2; areaZ++) {
			for (_int areaX = -1; areaX < 2; areaX++) {
				vecRenderArea.push_back(m_pRenderArea[m_iRenderAreaCenter + (m_iDivideRenderArea * areaZ) + areaX]);
			}
		}

		break;

	case AREA_5X5:

		for (_int areaZ = -2; areaZ < 3; areaZ++) {
			for (_int areaX = -2; areaX < 3; areaX++) {
				vecRenderArea.push_back(m_pRenderArea[m_iRenderAreaCenter + (m_iDivideRenderArea * areaZ) + areaX]);
			}
		}

		break;

	case AREA_7X7:

		for (_int areaZ = -3; areaZ < 4; areaZ++) {
			for (_int areaX = -3; areaX < 4; areaX++) {
				vecRenderArea.push_back(m_pRenderArea[m_iRenderAreaCenter + (m_iDivideRenderArea * areaZ) + areaX]);
			}
		}

		break;

	case AREA_9X9:

		for (_int i = 0; i < AREA_9X9; i++) {
			vecRenderArea.push_back(m_pRenderArea[i]);
		}

		break;

	default:
		break;
	}

	return vecRenderArea;
}

void CAreaManager::Free()
{
	Safe_Release(m_pGameInstance);
}
