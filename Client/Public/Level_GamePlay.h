#pragma once

#include "Client_Defines.h"
#include "Level.h"
#include "GameObject.h"

BEGIN(Engine)
class CLayer;
class CGameObject;
class CTransform;
class CNavigation;
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
#pragma endregion

#pragma region ���ӿ�����Ʈ

	HRESULT GameObject_Imgui(_float fTimeDelta);
	HRESULT GameObject_Create_GameObject(_float fTimeDelta);
	HRESULT GameObject_Layer_ListBox(_float fTimeDelta);
	HRESULT GameObject_Save_Load(_float fTimeDelta);
	HRESULT GameObject_Object_ListBox(_float fTimeDelta);
	HRESULT GameObject_vecStringSet();
	HRESULT GameObject_Pos_Scal_Turn();

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
	vector<_wstring>	m_vecString_GameObj;	 // ���ӿ�����Ʈ ����Ʈ�ڽ� ����
	_uint m_iSelectGameObj = { 0 };				 // ���� �������� ����Ʈ�ڽ� �ε���
#pragma endregion

	HRESULT VectorClear();
};

END
