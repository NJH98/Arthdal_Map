#pragma once

#include "Client_Defines.h"
#include "MapObject.h"

BEGIN(Client)

class CMapObject_Default final : public CMapObject
{
public:
    typedef struct
    {
        Matrix WorldMatrix = Matrix::Identity;
        _uint  ModelNum = 0;
        _float CullRadiuse = 0.f;
        _wstring	LayerTag = {};
    } MAPOBJECT_DESC;

public:
    enum MAP_MODEL_LIST {
        Map_MODEL_ObjectEnd = 185,
        Map_MODEL_PlantEnd = 207,
        Map_MODEL_TreeEnd = 232,
        Map_MODEL_HouseEnd = 275,
        Map_MODEL_GuildEnd = 292,
        Map_MODEL_CommonEnd = 336,
        Map_MODEL_LampEnd = 346,
        MAP_MODEL_END,
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

    _uint Get_UseShader() { return m_iUseShader; }
    void Set_UseShader(_uint ShaderNum) { m_iUseShader = ShaderNum; }
    _uint Get_UseModel() { return m_iUseModel; }
    void Set_InstanceRender(_bool isInstance) { m_bIsRenderInstance = isInstance; }
    _float Get_Radiuse() { return m_fRadiuse; }
    void Set_Radiuse(_float Radiuse) { m_fRadiuse = Radiuse; }

private:
    HRESULT Ready_Components();
    _uint           m_iUseModel = 0;
    _float          m_fRadiuse = 0.f;
    _uint           m_iUseShader = 1;
    _wstring        m_InstnaceLayer = {};
    _bool           m_bIsRenderInstance = true;
    _bool           m_bIsCalling = true;

    _uint           m_iArea = 0;

    CModel*         m_pModelCom = { nullptr };
    CShader*        m_pShaderCom = { nullptr };
    CShader*        m_pSubShaderCom = { nullptr };

public:
    static CMapObject_Default* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    virtual CGameObject* Clone(void* pArg);
    virtual void Free() override;
};

END