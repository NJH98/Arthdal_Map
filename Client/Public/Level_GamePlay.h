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
	// imgui �Լ�

#pragma region ���ϰ��
	HRESULT Dialog_Imgui(_float fTimeDelta);
	void ShowFileDialog();
	
	string filePath{};
#pragma endregion

#pragma region �ͷ���
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
	vector<_wstring>	m_vecString_Mask;			// ����ũ ����Ʈ �ڽ� ����
	_uint				m_iSelectTile = 0;			// ���� �������� ����ũ ��ȣ

	_uint				m_iMaskingNum[13] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};
#pragma endregion

#pragma region ���ӿ�����Ʈ

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

	// ������ ��ü �� ����
	_uint m_iSelectModel = { 0 };				// ���� �������� �� ��ȣ

	// ���̾� ����
	map<const _wstring, class CLayer*>* m_pMap_Layers = { nullptr };	// ���� ���̾��
	class CLayer* m_pLayer = { nullptr };		// ���� �������� ���̾�
	_wstring m_StringLayerName;					// ���� �������� ���̾� �̸�
	vector<_wstring>	m_vecString_Map_Layer;	// ���̾� ����Ʈ�ڽ� ����
	_uint m_iSelectMap = { 0 };					// ���� �������� ����Ʈ�ڽ� �ε���
	
	// ���� ������Ʈ ����
	_uint m_iGameObjListSize = { 0 };			 // ���� ���̾��� ���ӿ�����Ʈ ����Ʈ ũ�� ( ���� )
	_uint m_iPreGameObjListSize = { 0 };		 // ���̾��� ���� ���ӿ�����Ʈ ����Ʈ ũ��
	CGameObject* m_pGameObj = { nullptr };		 // ���� �������� ������Ʈ
	CTransform* m_pTransformCom = { nullptr };	 // ���� �������� ������Ʈ�� TransformCom ��ü
	vector<string>	m_vecString_GameObj;		 // ���ӿ�����Ʈ ����Ʈ�ڽ� ����
	_uint m_iSelectGameObj = { 0 };				 // ���� �������� ����Ʈ�ڽ� �ε���
#pragma endregion

#pragma region ��/�׺�
	HRESULT Cell_Imgui(_float fTimeDelta);
	HRESULT Cell_Add(_float fTimeDelta);
		_float3 Cell_Point_Correction(_float3 Point);
	HRESULT Cell_ListBox(_float fTimeDelta);
	HRESULT Cell_Data(_float fTimeDelta);
	HRESULT Cell_vecStringSet();
	HRESULT Cell_Save_Load(_float fTimeDelta);

	class CNavigation*	m_pNavigationCom_Terrain = { nullptr };		// �ͷ������� �����ϴ� NavigationCom ��ü
	CCell*				m_pCell = { nullptr };						// ���� �������� �� ������ 
	_uint				m_CellNum = { 0 };							// ���� �� ����
	vector<string>		m_vecString_Cell;							// �� ����Ʈ�ڽ� ����
	_uint				m_iSelectCell = { 0 };						// ���� �������� ����Ʈ�ڽ� �ε���
	_float3				PickA = {}, PickB = {}, PickC = {};			// Cell_Add �� ������
	_int				WhatPick = { 0 };							

#pragma endregion

#pragma region ����

	HRESULT Light_Imgui(_float fTimeDelta);
	HRESULT Light_Create_Light(_float fTimeDelta);
	HRESULT Light_ListBox(_float fTimeDelta);
	HRESULT Light_Save_Load(_float fTimeDelta);
	HRESULT Light_DescSet(_float fTimeDelta);

	_uint	m_iLightListSize = { 0 };			// ��ü ������ ����
	CLight* m_pLight = { nullptr };				// ���� �������� ����
	vector<string>	m_vecString_Light;			// ���� ����Ʈ�ڽ� ����
	_uint m_iSelectLight = { 0 };				// ���� �������� ����Ʈ�ڽ� �ε���

#pragma endregion

	HRESULT VectorClear();
};

END
