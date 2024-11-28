#pragma once
#include "Base.h"

BEGIN(Engine)
class CGameObject;

class CAreaManager final : public CBase
{
public:
    enum AREADATA { MAX_AREA = 25 };

private:
    CAreaManager();
    virtual ~CAreaManager() = default;

public:
    void Update();
    void RenderAreaSet();

    _uint   AreaIndexSet(_float3 Pos);
    _bool   IsInRenderArea(_int AreaIndex);
    _bool   Get_RenderAreaChange() { return RenderAreaChange; }
    _int    Get_RenderArea(_int list) { return m_pRenderArea[list]; }


private:
    class CGameInstance* m_pGameInstance = { nullptr };
    _float3 m_vPlayerPos = {};

    _int    m_pPlayerPreArea = { -1 };
    _int    m_pPlayerCurrentArea = { -1 };

    _int    m_pRenderArea[MAX_AREA] = { -1 };
    _bool   RenderAreaChange = false;

    _uint  m_iAreaValue = 32;
    _uint  m_iDivideArea = 32;

public:
    static CAreaManager* Create() { return new CAreaManager(); }
    virtual void Free();
};

END