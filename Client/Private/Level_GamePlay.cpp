#include "stdafx.h"
#include "..\Public\Level_GamePlay.h"

#include "FreeCamera.h"
#include "GameInstance.h"

#include "Terrain.h"

CLevel_GamePlay::CLevel_GamePlay(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CLevel{ pDevice, pContext }
{
}

HRESULT CLevel_GamePlay::Initialize()
{
	if (FAILED(Ready_Lights()))
		return E_FAIL;
	if (FAILED(Ready_Layer_Camera()))
		return E_FAIL;
	if (FAILED(Ready_Layer_BackGround()))
		return E_FAIL;
	/*if (FAILED(Ready_Layer_Effect()))
		return E_FAIL;*/
	/*if (FAILED(Ready_Layer_Monster()))
		return E_FAIL;*/
	/*if (FAILED(Ready_Layer_Player()))
		return E_FAIL;*/
	/*if (FAILED(Ready_Layer_Paticle()))
		return E_FAIL;*/

	// 오브젝트 관련 테스트 용도
	/*if (FAILED(OBJ_TEST()))
		return E_FAIL;*/
	
	//m_pGameInstance->PlayBGM(L"SD_BGM_Sual_Boss.ogg", 1.f, true);

	// ImGui 초기화
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;

	// ImGui 스타일 고르기
	ImGui::StyleColorsDark(); // 다크 모드
	//ImGui::StyleColorsLight(); // 일반 모드

	ImGui_ImplWin32_Init(g_hWnd);
	ImGui_ImplDX11_Init(m_pDevice, m_pContext);

	ImGuiStyle& style = ImGui::GetStyle();
	for (int i = 0; i < ImGuiCol_COUNT; i++)
	{
		ImVec4& col = style.Colors[i];
		col.w *= 0.8f;  // col.w는 색상의 알파(투명도) 값을 나타냅니다.
	}

	// Gui 랜더링 on/off 처음시작시 랜더가 우선작동해서 오류가 발생한다
	m_bGuiReady = false;

#pragma region imgui필요 포인터
	m_pTerrain = static_cast<CTerrain*>(m_pGameInstance->Get_Object(LEVEL_GAMEPLAY, TEXT("Layer_Terrain")));
	Safe_AddRef(m_pTerrain);
#pragma endregion

	return S_OK;
}

void CLevel_GamePlay::Update(_float fTimeDelta)
{
	// ImGui 프레임 업데이트
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	ImGui::Begin("MainImgui");
	{
		Terrain_Imgui(fTimeDelta);
	}
	ImGui::End();

	if (m_pGameInstance->Get_DIKeyState_Once(DIK_T)) {
		m_pTerrain->Get_VIBuffer()->Save_HeightMap();
	}
}

#pragma region Imgui 함수 정리

#pragma region Terrain 작동 코드

HRESULT CLevel_GamePlay::Terrain_Imgui(_float fTimeDelta)
{
	if (ImGui::CollapsingHeader("Terrain"))
	{
		// x,y 값을 입력받고 지형 생성
		if (FAILED(Create_Terrain_Input(fTimeDelta)))
			return E_FAIL;

		// 지형의 높이를 조정하는 코드
		if (FAILED(Terrain_HeightChange(fTimeDelta)))
			return E_FAIL;
	}

	return S_OK;
}

HRESULT CLevel_GamePlay::Create_Terrain_Input(_float fTimeDelta)
{
	ImGui::PushItemWidth(50); // 크기조정
	static int TerrainX = 0;
	ImGui::InputInt("TerrainX", &TerrainX, 0, 0, 1);
	static int TerrainZ = 0;
	ImGui::InputInt("TerrainZ", &TerrainZ, 0, 0, 1);

	if (ImGui::Button("  Delete Terrain  ")) {
		_bool test = CONFIRM_BOX(TEXT("삭제하시갰습니까?"));

		if (test) {
			m_pTerrain->Set_Dead(true);
			Safe_Release(m_pTerrain);
		}
	}
	ImGui::SameLine();
	if (ImGui::Button("  Create Terrain  ")) {
		CGameObject*	pTerrain = {};
		_float2			TerrainXZ{};

		TerrainXZ.x = _float(TerrainX);
		TerrainXZ.y = _float(TerrainZ);

		if (FAILED(m_pGameInstance->Add_GameObject_Out(TEXT("Prototype_GameObject_Terrain"), LEVEL_GAMEPLAY, TEXT("Layer_Terrain"), pTerrain, &TerrainXZ)))
			return E_FAIL;

		m_pTerrain = static_cast<CTerrain*>(pTerrain);
		Safe_AddRef(m_pTerrain);
	}

	ImGui::PopItemWidth();

	return S_OK;
}

HRESULT CLevel_GamePlay::Terrain_HeightChange(_float fTimeDelta)
{
	ImGui::SeparatorText("Height_Change");

	ImGui::PushItemWidth(150); // 크기조정
	static float TerrainRange = 0.f;
	ImGui::InputFloat("Range", &TerrainRange, 0.5f);
	static float TerrainHeightValue = 0.f;
	ImGui::InputFloat("Value", &TerrainHeightValue, 0.5f);

	static bool bHeight_Picking;
	ImGui::Checkbox("Height Picking", &bHeight_Picking);

	ImGui::PopItemWidth();
	
	if (bHeight_Picking) 
	{
		if (m_pGameInstance->Get_DIMouseState(DIMK_LBUTTON) && m_fTerrainTimeCheck > 0.2f) {
			// 피킹하고
			m_pGameInstance->Picking(&m_vPickPos);
			// 피킹정보 저장하고
			m_pGameInstance->Get_GlobalData()->Pick_Pos = m_vPickPos;
			
			if (m_pGameInstance->Get_DIKeyState(DIK_Q)) {
				// 값을 올린다
				m_pTerrain->Get_VIBuffer()->Change_Height(TerrainRange, TerrainHeightValue);
				m_fTerrainTimeCheck = 0.f;
			}

			if (m_pGameInstance->Get_DIKeyState(DIK_E)) {
				// 값을 내린다
				m_pTerrain->Get_VIBuffer()->Change_Height(TerrainRange, -TerrainHeightValue);
				m_fTerrainTimeCheck = 0.f;
			}
			
		}

		if (m_fTerrainTimeCheck < 0.2f)
			m_fTerrainTimeCheck += fTimeDelta;
	}
	

	return S_OK;
}

#pragma endregion

#pragma endregion


HRESULT CLevel_GamePlay::Render()
{
	//SetWindowText(g_hWnd, TEXT("게임플레이레벨입니다."));
	
	// ImGui 렌더링
	if (m_bGuiReady == true)
	{
		ImGui::Render();
		ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
	}
	else
		m_bGuiReady = true;

	return S_OK;
}


#pragma region 기본초기화
HRESULT CLevel_GamePlay::Ready_Lights()
{
	/* 게임플레이 레벨에 필요한 광원을 준비한다. */
	LIGHT_DESC			LightDesc{};

	ZeroMemory(&LightDesc, sizeof LightDesc);
	LightDesc.eType = LIGHT_DESC::TYPE_DIRECTIONAL;
	LightDesc.vDirection = _float4(1.f, -1.f, 1.f, 0.f);
	LightDesc.vDiffuse = _float4(1.f, 1.f, 1.f, 1.f);
	LightDesc.vAmbient = _float4(0.4f, 0.4f, 0.4f, 1.f);
	LightDesc.vSpecular = _float4(1.f, 1.f, 1.f, 1.f);

	if (FAILED(m_pGameInstance->Add_Light(LightDesc)))	
		return E_FAIL;

	//ZeroMemory(&LightDesc, sizeof LightDesc);
	//LightDesc.eType = LIGHT_DESC::TYPE_POINT;
	//LightDesc.vPosition = _float4(10.f, 3.f, 10.f, 1.f);
	//LightDesc.fRange = 7.f;	
	//LightDesc.vDiffuse = _float4(1.f, 0.f, 0.f, 1.f);
	//LightDesc.vAmbient = /*_float4(0.4f, 0.2f, 0.2f, 1.f);*/_float4(0.f, 0.f, 0.f, 0.f);
	//LightDesc.vSpecular = LightDesc.vDiffuse;

	//if (FAILED(m_pGameInstance->Add_Light(LightDesc)))
	//	return E_FAIL;

	//ZeroMemory(&LightDesc, sizeof LightDesc);
	//LightDesc.eType = LIGHT_DESC::TYPE_POINT;
	//LightDesc.vPosition = _float4(20.f, 3.f, 10.f, 1.f);
	//LightDesc.fRange = 7.f;
	//LightDesc.vDiffuse = _float4(0.f, 1.f, 0.f, 1.f);
	//LightDesc.vAmbient = /*_float4(0.2f, 0.4f, 0.2f, 1.f);*/_float4(0.f, 0.f, 0.f, 0.f);
	//LightDesc.vSpecular = LightDesc.vDiffuse;

	//if (FAILED(m_pGameInstance->Add_Light(LightDesc)))
	//	return E_FAIL;

	return S_OK;
}

HRESULT CLevel_GamePlay::Ready_Layer_Camera()
{
	CFreeCamera::CAMERA_FREE_DESC		Desc{};

	Desc.fSensor = 0.2f;
	Desc.vEye = _float4(0.f, 10.f, -10.f, 1.f);
	Desc.vAt = _float4(0.f, 0.f, 0.f, 1.f);
	Desc.fFovy = XMConvertToRadians(60.0f);
	Desc.fNear = 0.1f;
	Desc.fFar = 1000.f;
	Desc.fSpeedPerSec = 30.f;
	Desc.fRotationPerSec = XMConvertToRadians(90.0f);
	Desc.fAspect = (_float)g_iWinSizeX / g_iWinSizeY;

	if (FAILED(m_pGameInstance->Add_CloneObject_ToLayer(LEVEL_GAMEPLAY, TEXT("Layer_Camera"), TEXT("Prototype_GameObject_FreeCamera"), &Desc)))
		return E_FAIL;

	return S_OK;
}

HRESULT CLevel_GamePlay::Ready_Layer_BackGround()
{
	if (FAILED(m_pGameInstance->Add_CloneObject_ToLayer(LEVEL_GAMEPLAY, TEXT("Layer_Terrain"), TEXT("Prototype_GameObject_Terrain"))))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Add_CloneObject_ToLayer(LEVEL_GAMEPLAY, TEXT("Layer_BackGround"), TEXT("Prototype_GameObject_Sky"))))
		return E_FAIL;

	return S_OK;
}

HRESULT CLevel_GamePlay::Ready_Layer_Effect()
{
	/*for (size_t i = 0; i < 50; i++)
	{
		if (FAILED(m_pGameInstance->Add_CloneObject_ToLayer(LEVEL_GAMEPLAY, TEXT("Layer_Effect"), TEXT("Prototype_GameObject_Effect_Explosion"))))
			return E_FAIL;
	}*/

	return S_OK;
}

HRESULT CLevel_GamePlay::Ready_Layer_Monster()
{
	for (size_t i = 0; i < 10; i++)
	{
		if (FAILED(m_pGameInstance->Add_CloneObject_ToLayer(LEVEL_GAMEPLAY, TEXT("Layer_Monster"), TEXT("Prototype_GameObject_Monster"))))
			return E_FAIL;
	}

	return S_OK;
}

HRESULT CLevel_GamePlay::Ready_Layer_Paticle()
{
	//if (FAILED(m_pGameInstance->Add_CloneObject_ToLayer(LEVEL_GAMEPLAY, TEXT("Layer_Particle"), TEXT("Prototype_GameObject_Particle_Expolosion"))))
	//	return E_FAIL;

	if (FAILED(m_pGameInstance->Add_CloneObject_ToLayer(LEVEL_GAMEPLAY, TEXT("Layer_Particle"), TEXT("Prototype_GameObject_Particle_Snow"))))
		return E_FAIL;

	return S_OK;
}

HRESULT CLevel_GamePlay::Ready_Layer_Player()
{
	if (FAILED(m_pGameInstance->Add_CloneObject_ToLayer(LEVEL_GAMEPLAY, TEXT("Layer_Player"), TEXT("Prototype_GameObject_Player"))))
		return E_FAIL;

	return S_OK;
}

#pragma endregion

HRESULT CLevel_GamePlay::OBJ_TEST()
{
	if (FAILED(m_pGameInstance->Add_CloneObject_ToLayer(LEVEL_GAMEPLAY, TEXT("Layer_Player"), TEXT("Prototype_GameObject_Player"))))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Add_CloneObject_ToLayer(LEVEL_GAMEPLAY, TEXT("Layer_BackGround"), TEXT("Prototype_GameObject_ForkLift"))))
		return E_FAIL;

	return S_OK;
}

CLevel_GamePlay * CLevel_GamePlay::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CLevel_GamePlay*		pInstance = new CLevel_GamePlay(pDevice, pContext);

	if (FAILED(pInstance->Initialize()))
	{
		MSG_BOX(TEXT("Failed to Created : CLevel_GamePlay"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CLevel_GamePlay::Free()
{
	__super::Free();

	Safe_Release(m_pTerrain);

	// ImGui 정리
	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
}
