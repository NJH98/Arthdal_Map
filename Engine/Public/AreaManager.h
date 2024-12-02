#pragma once
#include "Base.h"

BEGIN(Engine)
class CGameObject;

class CAreaManager final : public CBase
{
public:
    enum AREADATA { 
        AREA_3X3 = 9, 
        AREA_5X5 = 25,
        AREA_7X7 = 49,
        AREA_9X9 = 81,
        MAX_AREA 
    };

private:
    CAreaManager();
    virtual ~CAreaManager() = default;

public:
    void Update();
    void RenderAreaSet();

    _uint   AreaIndexSet(_float3 Pos);
    _bool   IsInRenderArea(_int AreaIndex, AREADATA ChoiceArea);
    _bool   Get_RenderAreaChange() { return RenderAreaChange; }
    _int    Get_RenderArea(_int list) { return m_pRenderArea[list]; }


private:
    class CGameInstance* m_pGameInstance = { nullptr };
    _float3 m_vPlayerPos = {};

    _int    m_pPlayerPreArea = { -1 };
    _int    m_pPlayerCurrentArea = { -1 };

    _int    m_pRenderArea[MAX_AREA -1] = { -1 };
    _bool   RenderAreaChange = false;

    _int    m_iRenderAreaCenter = 40;           // RenderArea의 중앙 Index
    _int    m_iDivideRenderArea = 9;            // RenderArea의 가로값

    _uint  m_iAreaValue = 32;                   // 지역의 가로,세로값 ( 여기에서는 정사각형으로 사용 )
    _uint  m_iDivideArea = 32;                  // 전체 지형의 가로값 X축 ( z축이 늘어날떄마다 추가되어야 하는 값 )
                                                // 전체 지형 / AreaValue 하면 나온다 여기에서는 1024 / 32 = 32 로 나옴

public:
    static CAreaManager* Create() { return new CAreaManager(); }
    virtual void Free();
};

END