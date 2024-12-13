#pragma once

#include "Client_Defines.h"
#include "Level.h"
#include "GameObject.h"

BEGIN(Engine)
class CLayer;
class CGameObject;
class CTransform;
class CNavigation;
class CCell;
class CLight;
END

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

#pragma region 파일경로
	HRESULT Dialog_Imgui(_float fTimeDelta);
	void ShowFileDialog();
	
	string filePath{};
#pragma endregion

#pragma region 터레인
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

	_uint				m_iMaskingNum[13] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};
#pragma endregion

#pragma region 게임오브젝트

	HRESULT GameObject_Imgui(_float fTimeDelta);
	HRESULT GameObject_Create_GameObject(_float fTimeDelta);
	HRESULT GameObject_Model_ListBox(_float fTimeDelta);
	HRESULT GameObject_Layer_ListBox(_float fTimeDelta);
	HRESULT GameObject_Save_Load(_float fTimeDelta);
	HRESULT GameObject_Object_ListBox(_float fTimeDelta);
	HRESULT GameObject_vecStringSet();
	HRESULT GameObject_Pos_Scal_Turn();

	HRESULT GameObject_Save_Load_wstring();
	HRESULT GameObject_Save_Load_Node();

	// 생성할 객체 모델 관련
	_uint m_iSelectModel = { 0 };				// 현제 선택중인 모델 번호

	// 레이어 변수
	map<const _wstring, class CLayer*>* m_pMap_Layers = { nullptr };	// 실제 레이어들
	class CLayer* m_pLayer = { nullptr };		// 현제 선택중인 레이어
	_wstring m_StringLayerName;					// 현제 선택중인 레이어 이름
	vector<_wstring>	m_vecString_Map_Layer;	// 레이어 리스트박스 벡터
	_uint m_iSelectMap = { 0 };					// 현제 선택중인 리스트박스 인덱스
	
	// 게임 오브젝트 변수
	_uint m_iGameObjListSize = { 0 };			 // 현제 레이어의 게임오브젝트 리스트 크기 ( 갯수 )
	_uint m_iPreGameObjListSize = { 0 };		 // 레이어의 이전 게임오브젝트 리스트 크기
	CGameObject* m_pGameObj = { nullptr };		 // 현제 선택중인 오브젝트
	CTransform* m_pTransformCom = { nullptr };	 // 현제 선택중인 오브젝트의 TransformCom 객체
	vector<string>	m_vecString_GameObj;		 // 게임오브젝트 리스트박스 벡터
	_uint m_iSelectGameObj = { 0 };				 // 현제 선택중인 리스트박스 인덱스
#pragma endregion

#pragma region 셀/네비
	HRESULT Cell_Imgui(_float fTimeDelta);
	HRESULT Cell_Add(_float fTimeDelta);
		_float3 Cell_Point_Correction(_float3 Point);
	HRESULT Cell_ListBox(_float fTimeDelta);
	HRESULT Cell_Data(_float fTimeDelta);
	HRESULT Cell_vecStringSet();
	HRESULT Cell_Save_Load(_float fTimeDelta);

	class CNavigation*	m_pNavigationCom_Terrain = { nullptr };		// 터레인으로 접근하는 NavigationCom 객체
	CCell*				m_pCell = { nullptr };						// 현제 선택중인 셀 포인터 
	_uint				m_CellNum = { 0 };							// 현제 셀 갯수
	vector<string>		m_vecString_Cell;							// 셀 리스트박스 벡터
	_uint				m_iSelectCell = { 0 };						// 현제 선택중인 리스트박스 인덱스
	_float3				PickA = {}, PickB = {}, PickC = {};			// Cell_Add 용 변수들
	_int				WhatPick = { 0 };							

#pragma endregion

#pragma region 광원

	HRESULT Light_Imgui(_float fTimeDelta);
	HRESULT Light_Create_Light(_float fTimeDelta);
	HRESULT Light_ListBox(_float fTimeDelta);
	HRESULT Light_Save_Load(_float fTimeDelta);
	HRESULT Light_DescSet(_float fTimeDelta);

	_uint	m_iLightListSize = { 0 };			// 전체 광원의 갯수
	CLight* m_pLight = { nullptr };				// 현제 선택중인 광원
	vector<string>	m_vecString_Light;			// 광원 리스트박스 벡터
	_uint m_iSelectLight = { 0 };				// 현제 선택중인 리스트박스 인덱스

#pragma endregion

	HRESULT VectorClear();
};

END
