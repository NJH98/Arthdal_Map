#include "stdafx.h"
#include "..\Public\Loader.h"

#include "Sky.h"
#include "Player.h"
#include "Weapon.h"
#include "Monster.h"
#include "Terrain.h"
#include "ForkLift.h"
#include "FreeCamera.h"
#include "BackGround.h"
#include "Body_Player.h"
#include "Particle_Snow.h"
#include "Effect_Explosion.h"
#include "Particle_Explosion.h"

#include "MapObject_Default.h"

#include "GameInstance.h"


CLoader::CLoader(ID3D11Device * pDevice, ID3D11DeviceContext * pContext)
	: m_pDevice { pDevice }
	, m_pContext { pContext }
	, m_pGameInstance { CGameInstance::Get_Instance()}
{
	Safe_AddRef(m_pGameInstance);
	Safe_AddRef(m_pDevice);
	Safe_AddRef(m_pContext);
}


_uint APIENTRY LoadingMain(void* pArg)
{
	CoInitializeEx(nullptr, 0);

	CLoader*		pLoader = static_cast<CLoader*>(pArg);

	if(FAILED(pLoader->Loading()))
		return 1;
		
	return 0;
}


HRESULT CLoader::Initialize(LEVELID eNextLevelID)
{
	m_eNextLevelID = eNextLevelID;

	InitializeCriticalSection(&m_CriticalSection);

	m_hThread = (HANDLE)_beginthreadex(nullptr, 0, LoadingMain, this, 0, nullptr);
	if (0 == m_hThread)
		return E_FAIL;


	return S_OK;
}

HRESULT CLoader::Loading()
{
	EnterCriticalSection(&m_CriticalSection);

	HRESULT			hr = {};

	switch (m_eNextLevelID)
	{
	case LEVEL_LOGO:
		hr = Ready_Resources_For_LogoLevel();
		break;
	case LEVEL_GAMEPLAY:
		hr = Ready_Resources_For_GamePlayLevel();
		break;
	}

	LeaveCriticalSection(&m_CriticalSection);

	return hr;
}

void CLoader::Draw_LoadingText()
{
	SetWindowText(g_hWnd, m_szLoadingText);
}

HRESULT CLoader::Ready_Resources_For_LogoLevel()
{
	lstrcpy(m_szLoadingText, TEXT("텍스쳐를 로딩중입니다."));
	/* For. Prototype_Component_Texture_BackGround */
	if (FAILED(m_pGameInstance->Add_Prototype(LEVEL_LOGO, TEXT("Prototype_Component_Texture_BackGround"),
		CTexture::Create(m_pDevice, m_pContext, TEXT("../Bin/Resources/Textures/Default%d.jpg"), 2))))
		return E_FAIL;

	lstrcpy(m_szLoadingText, TEXT("모델을(를) 로딩중입니다."));


	lstrcpy(m_szLoadingText, TEXT("사운드을(를) 로딩중입니다."));


	lstrcpy(m_szLoadingText, TEXT("객체원형을(를) 로딩중입니다."));
	/* For. Prototype_GameObject_BackGround */
	if (FAILED(m_pGameInstance->Add_Prototype(TEXT("Prototype_GameObject_BackGround"),
		CBackGround::Create(m_pDevice, m_pContext))))
		return E_FAIL;

	lstrcpy(m_szLoadingText, TEXT("로딩이 완료되었습니다."));

	m_isFinished = true;

	return S_OK;
}

HRESULT CLoader::Ready_Resources_For_GamePlayLevel()
{
	lstrcpy(m_szLoadingText, TEXT("텍스쳐를 로딩중입니다."));
	/* 지형 색상 텍스쳐 */
	if (FAILED(m_pGameInstance->Add_Prototype(LEVEL_GAMEPLAY, TEXT("Prototype_Component_Texture_Terrain"),
		CTexture::Create(m_pDevice, m_pContext, TEXT("../Bin/Resources/Textures/Terrain/Tile/Tile_%d.dds"), 81))))
		return E_FAIL;
	/* 지형 노말 텍스쳐 */
	if (FAILED(m_pGameInstance->Add_Prototype(LEVEL_GAMEPLAY, TEXT("Prototype_Component_Texture_Terrain_Normal"),
		CTexture::Create(m_pDevice, m_pContext, TEXT("../Bin/Resources/Textures/Terrain/Tile/Tile_Normal_%d.dds"), 81))))
		return E_FAIL;

#pragma region 기타
	/* For. Prototype_Component_Texture_Brush*/
	if (FAILED(m_pGameInstance->Add_Prototype(LEVEL_GAMEPLAY, TEXT("Prototype_Component_Texture_Brush"),
		CTexture::Create(m_pDevice, m_pContext, TEXT("../Bin/Resources/Textures/Terrain/Brush.png"), 1))))
		return E_FAIL;

	/* For. Prototype_Component_Texture_Mask */
	if (FAILED(m_pGameInstance->Add_Prototype(LEVEL_GAMEPLAY, TEXT("Prototype_Component_Texture_Mask"),
		CTexture::Create(m_pDevice, m_pContext, TEXT("../Bin/Resources/Textures/Terrain/Mask.bmp"), 0))))
		return E_FAIL;

	/* For. Prototype_Component_Texture_Sky */
	if (FAILED(m_pGameInstance->Add_Prototype(LEVEL_GAMEPLAY, TEXT("Prototype_Component_Texture_Sky"),
		CTexture::Create(m_pDevice, m_pContext, TEXT("../Bin/Resources/Textures/SkyBox/Sky_%d.dds"), 4))))
		return E_FAIL;


	/* For. Prototype_Component_Texture_Particle */
	if (FAILED(m_pGameInstance->Add_Prototype(LEVEL_GAMEPLAY, TEXT("Prototype_Component_Texture_Particle"),
		CTexture::Create(m_pDevice, m_pContext, TEXT("../Bin/Resources/Textures/Snow/Snow.png"), 1))))
		return E_FAIL;

	/* For. Prototype_Component_Texture_Explosion */
	/*if (FAILED(m_pGameInstance->Add_Prototype(LEVEL_GAMEPLAY, TEXT("Prototype_Component_Texture_Explosion"),
		CTexture::Create(m_pDevice, m_pContext, TEXT("../Bin/Resources/Textures/Explosion/Explosion%d.png"), 90))))
		return E_FAIL;*/
#pragma endregion	

	lstrcpy(m_szLoadingText, TEXT("모델을(를) 로딩중입니다."));

	/* For. Prototype_Component_VIBuffer_Terrain*/
	if (FAILED(m_pGameInstance->Add_Prototype(LEVEL_GAMEPLAY, TEXT("Prototype_Component_VIBuffer_Terrain"),
		CVIBuffer_Terrain::Create(m_pDevice, m_pContext, TEXT("../Bin/Resources/Textures/Terrain/Height1.bmp")))))
		return E_FAIL;
#pragma region 기타
	/* For. Prototype_Component_VIBuffer_Cube */
	if (FAILED(m_pGameInstance->Add_Prototype(LEVEL_GAMEPLAY, TEXT("Prototype_Component_VIBuffer_Cube"),
		CVIBuffer_Cube::Create(m_pDevice, m_pContext))))
		return E_FAIL;

	CVIBuffer_Instancing::INSTANCE_DESC			ParticleDesc{};
	/* For. Prototype_Component_VIBuffer_Particle_Explosion */
	ZeroMemory(&ParticleDesc, sizeof ParticleDesc);

	ParticleDesc.iNumInstance = 200;
	ParticleDesc.vCenter = _float3(0.f, 0.f, 0.f);
	ParticleDesc.vRange = _float3(0.3f, 0.3f, 0.3f);
	ParticleDesc.vSize = _float2(0.05f, 0.1f);
	ParticleDesc.vPivot = _float3(0.f, 0.f, 0.f); 
	ParticleDesc.vSpeed = _float2(1.f, 3.f);
	ParticleDesc.vLifeTime = _float2(0.4f, 0.6f);
	ParticleDesc.isLoop = true;

	if (FAILED(m_pGameInstance->Add_Prototype(LEVEL_GAMEPLAY, TEXT("Prototype_Component_VIBuffer_Particle_Explosion"),
		CVIBuffer_Rect_Instance::Create(m_pDevice, m_pContext, ParticleDesc))))
		return E_FAIL;

	/* For. Prototype_Component_VIBuffer_Particle_Explosion */
	ZeroMemory(&ParticleDesc, sizeof ParticleDesc);

	ParticleDesc.iNumInstance = 3000;
	ParticleDesc.vCenter = _float3(64.f, 20.f, 64.f);
	ParticleDesc.vRange = _float3(128.f, 1.f, 128.f);
	ParticleDesc.vSize = _float2(0.1f, 0.3f);
	ParticleDesc.vPivot = _float3(0.f, 0.f, 0.f);
	ParticleDesc.vSpeed = _float2(1.f, 3.f);
	ParticleDesc.vLifeTime = _float2(4.f, 8.f);
	ParticleDesc.isLoop = true;

	if (FAILED(m_pGameInstance->Add_Prototype(LEVEL_GAMEPLAY, TEXT("Prototype_Component_VIBuffer_Particle_Snow"),
		CVIBuffer_Point_Instance::Create(m_pDevice, m_pContext, ParticleDesc))))
		return E_FAIL;

	_matrix		PreTransformMatrix = XMMatrixIdentity();

	/* For. Prototype_Component_Model_Fiona*/
	PreTransformMatrix = XMMatrixScaling(0.01f, 0.01f, 0.01f) * XMMatrixRotationY(XMConvertToRadians(180.0f));

	if (FAILED(m_pGameInstance->Add_Prototype(LEVEL_GAMEPLAY, TEXT("Prototype_Component_Model_Fiona"),
		CModel::Create(m_pDevice, m_pContext, CModel::TYPE_ANIM, TEXT("../Bin/Resources/Models/Fiona/TestModel"), PreTransformMatrix))))
		return E_FAIL;

#pragma endregion

#pragma region 맵 배치 오브젝트 모델
	PreTransformMatrix = XMMatrixScaling(0.01f, 0.01f, 0.01f) * XMMatrixRotationY(XMConvertToRadians(180.0f));

	for (_uint i = 0; i < 185; i++) {
		_wstring PrototypeTag = L"Prototype_Component_Model_Map_Object" + to_wstring(i);
		_wstring FilePath = L"../Bin/Resources/Models/MapModel/Object/Object_" + to_wstring(i);
		string ModelName = "Object_" + to_string(i);

		if (FAILED(m_pGameInstance->Add_Prototype(LEVEL_GAMEPLAY, PrototypeTag.c_str(),
			CModel::Create(m_pDevice, m_pContext, CModel::TYPE_NONANIM, FilePath.c_str(), PreTransformMatrix))))
			return E_FAIL;
		m_pGameInstance->Get_GlobalData()->ModelName.push_back(ModelName);
	}

	for (_uint i = 0; i < 22; i++) {
		_wstring PrototypeTag = L"Prototype_Component_Model_Map_Plant" + to_wstring(i);
		_wstring FilePath = L"../Bin/Resources/Models/MapModel/Plant/Plant_" + to_wstring(i);
		string ModelName = "Plant_" + to_string(i + 185);

		if (FAILED(m_pGameInstance->Add_Prototype(LEVEL_GAMEPLAY, PrototypeTag.c_str(),
			CModel::Create(m_pDevice, m_pContext, CModel::TYPE_NONANIM, FilePath.c_str(), PreTransformMatrix))))
			return E_FAIL;
		m_pGameInstance->Get_GlobalData()->ModelName.push_back(ModelName);
	}

	for (_uint i = 0; i < 25; i++) {
		_wstring PrototypeTag = L"Prototype_Component_Model_Map_Tree" + to_wstring(i);
		_wstring FilePath = L"../Bin/Resources/Models/MapModel/Tree/Tree_" + to_wstring(i);
		string ModelName = "Tree_" + to_string(i + 185 + 22);

		if (FAILED(m_pGameInstance->Add_Prototype(LEVEL_GAMEPLAY, PrototypeTag.c_str(),
			CModel::Create(m_pDevice, m_pContext, CModel::TYPE_NONANIM, FilePath.c_str(), PreTransformMatrix))))
			return E_FAIL;
		m_pGameInstance->Get_GlobalData()->ModelName.push_back(ModelName);
	}
	
	for (_uint i = 0; i < 43; i++) {
		_wstring PrototypeTag = L"Prototype_Component_Model_Map_House" + to_wstring(i);
		_wstring FilePath = L"../Bin/Resources/Models/MapModel/House/House_" + to_wstring(i);
		string ModelName = "House_" + to_string(i + 185 + 22 + 25);

		if (FAILED(m_pGameInstance->Add_Prototype(LEVEL_GAMEPLAY, PrototypeTag.c_str(),
			CModel::Create(m_pDevice, m_pContext, CModel::TYPE_NONANIM, FilePath.c_str(), PreTransformMatrix))))
			return E_FAIL;
		m_pGameInstance->Get_GlobalData()->ModelName.push_back(ModelName);
	}

	for (_uint i = 0; i < 17; i++) {
		_wstring PrototypeTag = L"Prototype_Component_Model_Map_Guild" + to_wstring(i);
		_wstring FilePath = L"../Bin/Resources/Models/MapModel/House/Guild_" + to_wstring(i);
		string ModelName = "Guild_" + to_string(i + 185 + 22 + 25 + 43);

		if (FAILED(m_pGameInstance->Add_Prototype(LEVEL_GAMEPLAY, PrototypeTag.c_str(),
			CModel::Create(m_pDevice, m_pContext, CModel::TYPE_NONANIM, FilePath.c_str(), PreTransformMatrix))))
			return E_FAIL;
		m_pGameInstance->Get_GlobalData()->ModelName.push_back(ModelName);
	}

	for (_uint i = 0; i < 44; i++) {
		_wstring PrototypeTag = L"Prototype_Component_Model_Map_Common" + to_wstring(i);
		_wstring FilePath = L"../Bin/Resources/Models/MapModel/Common/Common_" + to_wstring(i);
		string ModelName = "Common_" + to_string(i + 185 + 22 + 25 + 43 + 17);

		if (FAILED(m_pGameInstance->Add_Prototype(LEVEL_GAMEPLAY, PrototypeTag.c_str(),
			CModel::Create(m_pDevice, m_pContext, CModel::TYPE_NONANIM, FilePath.c_str(), PreTransformMatrix))))
			return E_FAIL;
		m_pGameInstance->Get_GlobalData()->ModelName.push_back(ModelName);
	}
	
	for (_uint i = 0; i < 10; i++) {
		_wstring PrototypeTag = L"Prototype_Component_Model_Map_Lamp" + to_wstring(i);
		_wstring FilePath = L"../Bin/Resources/Models/MapModel/Lamp/Lamp_" + to_wstring(i);
		string ModelName = "Lamp_" + to_string(i + 185 + 22 + 25 + 43 + 17 + 44);

		if (FAILED(m_pGameInstance->Add_Prototype(LEVEL_GAMEPLAY, PrototypeTag.c_str(),
			CModel::Create(m_pDevice, m_pContext, CModel::TYPE_NONANIM, FilePath.c_str(), PreTransformMatrix))))
			return E_FAIL;
		m_pGameInstance->Get_GlobalData()->ModelName.push_back(ModelName);
	}

	for (_uint i = 0; i < 4; i++) {
		_wstring PrototypeTag = L"Prototype_Component_Model_Map_Anything" + to_wstring(i);
		_wstring FilePath = L"../Bin/Resources/Models/MapModel/Anything/Anything_" + to_wstring(i);
		string ModelName = "Anything_" + to_string(i + 185 + 22 + 25 + 43 + 17 + 44 + 10);

		if (FAILED(m_pGameInstance->Add_Prototype(LEVEL_GAMEPLAY, PrototypeTag.c_str(),
			CModel::Create(m_pDevice, m_pContext, CModel::TYPE_NONANIM, FilePath.c_str(), PreTransformMatrix))))
			return E_FAIL;
		m_pGameInstance->Get_GlobalData()->ModelName.push_back(ModelName);
	}

	for (_uint i = 0; i < 50; i++) {
		_wstring PrototypeTag = L"Prototype_Component_Model_Map_Asd_Alliance" + to_wstring(i);
		_wstring FilePath = L"../Bin/Resources/Models/MapModel/Asd_Map/Asd_Alliance_" + to_wstring(i);
		string ModelName = "Asd_Alliance_" + to_string(i + 185 + 22 + 25 + 43 + 17 + 44 + 10 + 4);

		if (FAILED(m_pGameInstance->Add_Prototype(LEVEL_GAMEPLAY, PrototypeTag.c_str(),
			CModel::Create(m_pDevice, m_pContext, CModel::TYPE_NONANIM, FilePath.c_str(), PreTransformMatrix))))
			return E_FAIL;
		m_pGameInstance->Get_GlobalData()->ModelName.push_back(ModelName);
	}

	for (_uint i = 0; i < 250; i++) {
		_wstring PrototypeTag = L"Prototype_Component_Model_Map_Asd_Building" + to_wstring(i);
		_wstring FilePath = L"../Bin/Resources/Models/MapModel/Asd_Map/Asd_Building_" + to_wstring(i);
		string ModelName = "Asd_Building_" + to_string(i + 185 + 22 + 25 + 43 + 17 + 44 + 10 + 4 + 50);

		if (FAILED(m_pGameInstance->Add_Prototype(LEVEL_GAMEPLAY, PrototypeTag.c_str(),
			CModel::Create(m_pDevice, m_pContext, CModel::TYPE_NONANIM, FilePath.c_str(), PreTransformMatrix))))
			return E_FAIL;
		m_pGameInstance->Get_GlobalData()->ModelName.push_back(ModelName);
	}

	for (_uint i = 0; i < 3; i++) {
		_wstring PrototypeTag = L"Prototype_Component_Model_Map_Asd_Guild" + to_wstring(i);
		_wstring FilePath = L"../Bin/Resources/Models/MapModel/Asd_Map/Asd_Guild_" + to_wstring(i);
		string ModelName = "Asd_Guild_" + to_string(i + 185 + 22 + 25 + 43 + 17 + 44 + 10 + 4 + 50 + 250);

		if (FAILED(m_pGameInstance->Add_Prototype(LEVEL_GAMEPLAY, PrototypeTag.c_str(),
			CModel::Create(m_pDevice, m_pContext, CModel::TYPE_NONANIM, FilePath.c_str(), PreTransformMatrix))))
			return E_FAIL;
		m_pGameInstance->Get_GlobalData()->ModelName.push_back(ModelName);
	}

	for (_uint i = 0; i < 8; i++) {
		_wstring PrototypeTag = L"Prototype_Component_Model_Map_Asd_House" + to_wstring(i);
		_wstring FilePath = L"../Bin/Resources/Models/MapModel/Asd_Map/Asd_House_" + to_wstring(i);
		string ModelName = "Asd_House_" + to_string(i + 185 + 22 + 25 + 43 + 17 + 44 + 10 + 4 + 50 + 250 + 3);

		if (FAILED(m_pGameInstance->Add_Prototype(LEVEL_GAMEPLAY, PrototypeTag.c_str(),
			CModel::Create(m_pDevice, m_pContext, CModel::TYPE_NONANIM, FilePath.c_str(), PreTransformMatrix))))
			return E_FAIL;
		m_pGameInstance->Get_GlobalData()->ModelName.push_back(ModelName);
	}

	for (_uint i = 0; i < 65; i++) {
		_wstring PrototypeTag = L"Prototype_Component_Model_Map_Asd_Object" + to_wstring(i);
		_wstring FilePath = L"../Bin/Resources/Models/MapModel/Asd_Map/Asd_Object_" + to_wstring(i);
		string ModelName = "Asd_Object_" + to_string(i + 185 + 22 + 25 + 43 + 17 + 44 + 10 + 4 + 50 + 250 + 3 + 8);

		if (FAILED(m_pGameInstance->Add_Prototype(LEVEL_GAMEPLAY, PrototypeTag.c_str(),
			CModel::Create(m_pDevice, m_pContext, CModel::TYPE_NONANIM, FilePath.c_str(), PreTransformMatrix))))
			return E_FAIL;
		m_pGameInstance->Get_GlobalData()->ModelName.push_back(ModelName);
	}

	for (_uint i = 0; i < 2; i++) {
		_wstring PrototypeTag = L"Prototype_Component_Model_Map_Asd_Lamp" + to_wstring(i);
		_wstring FilePath = L"../Bin/Resources/Models/MapModel/Asd_Lamp/Asd_Lamp_" + to_wstring(i);
		string ModelName = "Asd_Lamp_" + to_string(i + 185 + 22 + 25 + 43 + 17 + 44 + 10 + 4 + 50 + 250 + 3 + 8 + 65);

		if (FAILED(m_pGameInstance->Add_Prototype(LEVEL_GAMEPLAY, PrototypeTag.c_str(),
			CModel::Create(m_pDevice, m_pContext, CModel::TYPE_NONANIM, FilePath.c_str(), PreTransformMatrix))))
			return E_FAIL;
		m_pGameInstance->Get_GlobalData()->ModelName.push_back(ModelName);
	}

	for (_uint i = 0; i < 2; i++) {
		_wstring PrototypeTag = L"Prototype_Component_Model_Map_Asd_Anything" + to_wstring(i);
		_wstring FilePath = L"../Bin/Resources/Models/MapModel/Asd_Anything/Asd_Anything_" + to_wstring(i);
		string ModelName = "Asd_Anything_" + to_string(i + 185 + 22 + 25 + 43 + 17 + 44 + 10 + 4 + 50 + 250 + 3 + 8 + 65 + 2);

		if (FAILED(m_pGameInstance->Add_Prototype(LEVEL_GAMEPLAY, PrototypeTag.c_str(),
			CModel::Create(m_pDevice, m_pContext, CModel::TYPE_NONANIM, FilePath.c_str(), PreTransformMatrix))))
			return E_FAIL;
		m_pGameInstance->Get_GlobalData()->ModelName.push_back(ModelName);
	}

#pragma endregion

	lstrcpy(m_szLoadingText, TEXT("네비게이션을(를) 로딩중입니다."));
	/* For.Prototype_Component_Navigation */
	if (FAILED(m_pGameInstance->Add_Prototype(LEVEL_GAMEPLAY, TEXT("Prototype_Component_Navigation"),
		CNavigation::Create(m_pDevice, m_pContext, TEXT("../Bin/DataFiles/Navigation.dat")))))
		return E_FAIL;


	lstrcpy(m_szLoadingText, TEXT("셰이더을(를) 로딩중입니다."));


	

	lstrcpy(m_szLoadingText, TEXT("콜라이더을(를) 로딩중입니다."));


	lstrcpy(m_szLoadingText, TEXT("사운드을(를) 로딩중입니다."));
  	m_pGameInstance->Load_Sound("BGM");

	lstrcpy(m_szLoadingText, TEXT("객체원형을(를) 로딩중입니다."));

#pragma region 기타
	/* For. Prototype_GameObject_Terrain */
	if (FAILED(m_pGameInstance->Add_Prototype(TEXT("Prototype_GameObject_Terrain"),
		CTerrain::Create(m_pDevice, m_pContext))))
		return E_FAIL;

	/* For. Prototype_GameObject_Monster */
	if (FAILED(m_pGameInstance->Add_Prototype(TEXT("Prototype_GameObject_Monster"),
		CMonster::Create(m_pDevice, m_pContext))))
		return E_FAIL;

	/* For. Prototype_GameObject_Player */
	if (FAILED(m_pGameInstance->Add_Prototype(TEXT("Prototype_GameObject_Player"),
		CPlayer::Create(m_pDevice, m_pContext))))
		return E_FAIL;

	/* For. Prototype_GameObject_Body_Player */
	if (FAILED(m_pGameInstance->Add_Prototype(TEXT("Prototype_GameObject_Body_Player"),
		CBody_Player::Create(m_pDevice, m_pContext))))
		return E_FAIL;

	/* For. Prototype_GameObject_Weapon */
	if (FAILED(m_pGameInstance->Add_Prototype(TEXT("Prototype_GameObject_Weapon"),
		CWeapon::Create(m_pDevice, m_pContext))))
		return E_FAIL;

	/* For. Prototype_GameObject_FreeCamera */
	if (FAILED(m_pGameInstance->Add_Prototype(TEXT("Prototype_GameObject_FreeCamera"),
		CFreeCamera::Create(m_pDevice, m_pContext))))
		return E_FAIL;


	/* For. Prototype_GameObject_Sky */
	if (FAILED(m_pGameInstance->Add_Prototype(TEXT("Prototype_GameObject_Sky"),
		CSky::Create(m_pDevice, m_pContext))))
		return E_FAIL;

	/* For. Prototype_GameObject_ForkLift */
	if (FAILED(m_pGameInstance->Add_Prototype(TEXT("Prototype_GameObject_ForkLift"),
		CForkLift::Create(m_pDevice, m_pContext))))
		return E_FAIL;

	/* For. Prototype_GameObject_Particle_Explosion */
	if (FAILED(m_pGameInstance->Add_Prototype(TEXT("Prototype_GameObject_Particle_Explosion"),
		CParticle_Explosion::Create(m_pDevice, m_pContext))))
		return E_FAIL;

	/* For. Prototype_GameObject_Particle_Snow */
	if (FAILED(m_pGameInstance->Add_Prototype(TEXT("Prototype_GameObject_Particle_Snow"),
		CParticle_Snow::Create(m_pDevice, m_pContext))))
		return E_FAIL;

	/* For. Prototype_GameObject_Effect_Explosion */
	if (FAILED(m_pGameInstance->Add_Prototype(TEXT("Prototype_GameObject_Effect_Explosion"),
		CEffect_Explosion::Create(m_pDevice, m_pContext))))
		return E_FAIL;

#pragma endregion

	/* 맵 배치 오브젝트 객체 원형 */
	if (FAILED(m_pGameInstance->Add_Prototype(TEXT("Prototype_GameObject_MapObject_Default"),
		CMapObject_Default::Create(m_pDevice, m_pContext))))
		return E_FAIL;

	lstrcpy(m_szLoadingText, TEXT("로딩이 완료되었습니다."));

	m_isFinished = true;

	return S_OK;
}

CLoader * CLoader::Create(ID3D11Device * pDevice, ID3D11DeviceContext * pContext, LEVELID eNextLevelID)
{
	CLoader*		pInstance = new CLoader(pDevice, pContext);

	if (FAILED(pInstance->Initialize(eNextLevelID)))
	{
		MSG_BOX(TEXT("Failed to Created : CLoader"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CLoader::Free()
{
	__super::Free();

	WaitForSingleObject(m_hThread, INFINITE);

	DeleteObject(m_hThread);

	CloseHandle(m_hThread);

	DeleteCriticalSection(&m_CriticalSection);

	Safe_Release(m_pGameInstance);
	Safe_Release(m_pDevice);
	Safe_Release(m_pContext);
}
