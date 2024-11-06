#pragma once

#include "Client_Defines.h"
#include "Level.h"

BEGIN(Client)

class CLevel_GamePlay final : public CLevel
{
private:
	CLevel_GamePlay(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual ~CLevel_GamePlay() = default;

public:
	virtual HRESULT Initialize() override;
	virtual void Update(_float fTimeDelta) override;
	virtual HRESULT Render() override;

public:
	HRESULT Ready_Lights();
	HRESULT Ready_Layer_Camera();	
	HRESULT Ready_Layer_BackGround();
	HRESULT Ready_Layer_Effect();
	HRESULT Ready_Layer_Monster();
	HRESULT Ready_Layer_Paticle();
	HRESULT Ready_Layer_Player();

	HRESULT OBJ_TEST();


public:
	static CLevel_GamePlay* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual void Free() override;

private:
	bool			m_bGuiReady = { false };

	_float3			m_vPickPos = {};

private:
	// imgui 함수

	// 파일경로
	HRESULT Dialog_Imgui(_float fTimeDelta);
	void ShowFileDialog();
	
	string filePath{};

	// Terrain 용
	HRESULT Terrain_Imgui(_float fTimeDelta);
	HRESULT	Create_Terrain_Input(_float fTimeDelta);
	HRESULT Terrain_HeightChange(_float fTimeDelta);
	HRESULT Terrain_HeightSaveLoad(_float fTimeDelta);
	HRESULT Terrain_Mask_ListBox(_float fTimeDelta);
	HRESULT Terrain_Masking(_float fTimeDelta);
	HRESULT Terrain_MaskSaveLoad(_float fTimeDelta);
	HRESULT Change_Mask(_float fTimeDelta);

	class CTerrain*		m_pTerrain = { nullptr };
	_float				m_fTerrainTimeCheck = 0.f;
	vector<_wstring>	m_vecString_Mask;			// 마스크 리스트 박스 벡터
	_uint				m_iSelectTile = 0;			// 현제 선택중인 마스크 번호

	HRESULT VectorClear();
};

END
