#pragma once

#include "Client_Defines.h"
#include "MapObject.h"

BEGIN(Client)

class CMapObject_Default final : public CMapObject
{
public:
    enum MAP_MODEL_LIST {
        MAP_MODEL_ForkLift,
        Map_MODEL_AgoVillage_Boss,
        Map_MODEL_Plant_Berry,
        Map_MODEL_Plant_Bush,
        Map_MODEL_Plant_Weed,
        Map_MODEL_Tree_Bamboo,
        MAP_MODEL_END
    };
private:
    CMapObject_Default(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    CMapObject_Default(const CMapObject_Default& Prototype);
    virtual ~CMapObject_Default() = default;

public:
    virtual HRESULT Initialize_Prototype() override;
    virtual HRESULT Initialize(void* pArg) override;
    virtual void Priority_Update(_float fTimeDelta) override;
    virtual _int Update(_float fTimeDelta) override;
    virtual void Late_Update(_float fTimeDelta) override;
    virtual HRESULT Render() override;
    virtual HRESULT Render_LightDepth() override;

    _uint Get_UseShader() { return m_iUseShader; };
    void Set_UseShader(_uint ShaderNum) { m_iUseShader = ShaderNum; }

    void Set_DepthNum(_float DepthNum) { m_DepthNum = DepthNum; }

private:
    HRESULT Ready_Components();
    _uint           m_iUseModel = 0;
    _uint           m_iUseShader = 1;
    _float          m_DepthNum = 0.f;

    CModel*         m_pModelCom[MAP_MODEL_END] = { nullptr };
    CShader*        m_pShaderCom = { nullptr };

public:
    static CMapObject_Default* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    virtual CGameObject* Clone(void* pArg);
    virtual void Free() override;
};

END