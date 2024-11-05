#include "stdafx.h"
#include "..\Public\Level_GamePlay.h"

#include <codecvt>
#include <commdlg.h>
#include <fstream>
#include <iostream>

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

	ShowFileDialog();

	ImGui::Begin("MainImgui");
	{
		Terrain_Imgui(fTimeDelta);
	}
	ImGui::End();

	VectorClear();

	if (m_pGameInstance->Get_DIKeyState_Once(DIK_T)) {
		
	}

}

#pragma region Imgui 함수 정리

#pragma region 파일경로 관련 함수
static string wstring_to_string(const wstring& wstr) {
	wstring_convert<codecvt_utf8<wchar_t>> converter;
	return converter.to_bytes(wstr);

	// wstring 으로 작성된 이름을 string으로 변환 > 이후 .ctr 을 붙여서 
	// char* 형으로 사용이 가능하다 imgui 에서는 char* 를 사용하기에 만든 함수
}

static string WideCharToMultiByte(const wstring& wstr) {
	int size_needed = WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), NULL, 0, NULL, NULL);
	string strTo(size_needed, 0);
	WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), &strTo[0], size_needed, NULL, NULL);
	return strTo;
}

static wchar_t* stringToWchar(const std::string& str) {
	int size_needed = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), static_cast<int>(str.size()), NULL, 0);
	wchar_t* wcharStr = new wchar_t[size_needed + 1];
	MultiByteToWideChar(CP_UTF8, 0, str.c_str(), static_cast<int>(str.size()), wcharStr, size_needed);
	wcharStr[size_needed] = L'\0';

	return wcharStr; // 호출자가 delete[]로 해제 필요
}

void CLevel_GamePlay::ShowFileDialog()
{
	// 파일 대화 상자 초기화
	ImGuiFileDialog::Instance()->OpenDialog(
		"OpenFileDialog",           // vKey
		"Select a File",           // vTitle
		".bmp",					   // vFilters
		IGFD::FileDialogConfig()   // vConfig (기본 설정)
	);

	// 대화 상자가 열릴 때
	if (ImGuiFileDialog::Instance()->Display("OpenFileDialog"))
	{
		// 사용자가 파일을 선택한 경우
		if (ImGuiFileDialog::Instance()->IsOk())
		{
			// 선택된 파일 경로 가져오기
			filePath = ImGuiFileDialog::Instance()->GetFilePathName();
		}

		// 대화 상자 닫기
		ImGuiFileDialog::Instance()->Close();
	}
}
#pragma endregion

#pragma region Terrain 작동 코드

HRESULT CLevel_GamePlay::Terrain_Imgui(_float fTimeDelta)
{
	if (ImGui::CollapsingHeader("Terrain"))
	{
		// x,y 값을 입력받고 지형 생성
		if (FAILED(Create_Terrain_Input(fTimeDelta)))
			return E_FAIL;

		string substring;
		if (filePath.length() > 55) {
			substring = filePath.substr(56);
		}
		ImGui::Text("%s", substring.c_str());

		if (ImGui::CollapsingHeader("Height")) 
		{
			// 지형의 높이를 조정하는 코드
			if (FAILED(Terrain_HeightChange(fTimeDelta)))
				return E_FAIL;

			// 지형의 높이를 저장, 불러오기 코드
			if (FAILED(Terrain_HeightSaveLoad(fTimeDelta)))
				return E_FAIL;
		}

		if (ImGui::CollapsingHeader("Mask"))
		{
			if (FAILED(Terrain_Mask_ListBox(fTimeDelta)))
				return E_FAIL;

			if (FAILED(Terrain_Masking(fTimeDelta)))
				return E_FAIL;

			if (FAILED(Terrain_MaskSaveLoad(fTimeDelta)))
				return E_FAIL;
		}
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
			m_pTerrain = nullptr;
		}
	}
	ImGui::SameLine();
	if (ImGui::Button("  Create Terrain  ")) {

		// 기존의 지형 삭제
		m_pTerrain->Set_Dead(true);
		Safe_Release(m_pTerrain);
		m_pTerrain = nullptr;

		// 신규 지형 생성 ( 지형은 언제나 1개로 기준잡는다 )
		CGameObject*	pTerrain = {};
		CVIBuffer_Terrain::TERRAIN_BUFFER_DESC Desc{};
		Desc.TerrainXZ.x = _float(TerrainX);
		Desc.TerrainXZ.y = _float(TerrainZ);

		if (FAILED(m_pGameInstance->Add_GameObject_Out(TEXT("Prototype_GameObject_Terrain"), LEVEL_GAMEPLAY, TEXT("Layer_Terrain"), pTerrain, &Desc)))
			return E_FAIL;

		// 현제 지형으로 변경
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
	ImGui::InputFloat("Height_Range", &TerrainRange, 0.5f);
	static float TerrainHeightValue = 0.f;
	ImGui::InputFloat("Height_Value", &TerrainHeightValue, 0.5f);

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

HRESULT CLevel_GamePlay::Terrain_HeightSaveLoad(_float fTimeDelta)
{
	ImGui::SeparatorText("Save/Load_Height.bmp");

	ImGui::PushItemWidth(300); // 크기조정
	if (ImGui::Button("  Save_Terrain_Height.bmp  ")) {
		if (filePath.length() < 55) {
			MSG_BOX(TEXT("Chocie FilePath"));
			return E_FAIL;
		}

		_tchar* wstrFilePath = stringToWchar(filePath);

		if (FAILED(m_pTerrain->Get_VIBuffer()->Save_HeightMap(wstrFilePath)))
		{
			MSG_BOX(TEXT("Failed to Save"));
		}

		delete[] wstrFilePath;
	}

	if (ImGui::Button("  Load_Terrain_Height.bmp  ")) {
		if (filePath.length() < 55) {
			MSG_BOX(TEXT("Chocie FilePath"));
			return E_FAIL;
		}

		// 기존의 지형 삭제
		m_pTerrain->Set_Dead(true);
		Safe_Release(m_pTerrain);
		m_pTerrain = nullptr;

		// 새로운 객체 생성/받아오기
		CGameObject* pTerrain = {};
		CVIBuffer_Terrain::TERRAIN_BUFFER_DESC Desc{};

		_tchar* wstrFilePath = stringToWchar(filePath);

		Desc.pHeightMapFilePath = wstrFilePath;

		if (FAILED(m_pGameInstance->Add_GameObject_Out(TEXT("Prototype_GameObject_Terrain"), LEVEL_GAMEPLAY, TEXT("Layer_Terrain"), pTerrain, &Desc))) 
		{
			MSG_BOX(TEXT("Failed to Load"));
		}

		delete[] wstrFilePath;

		m_pTerrain = static_cast<CTerrain*>(pTerrain);
		Safe_AddRef(m_pTerrain);
	}

	ImGui::PopItemWidth();

	return S_OK;
}

HRESULT CLevel_GamePlay::Terrain_Mask_ListBox(_float fTimeDelta)
{
	CTexture* pMaskTexture = nullptr;

	if (m_pTerrain != nullptr) {
		pMaskTexture = m_pTerrain->Get_Texture(CTerrain::TEXTURE_MASK);

		for (_uint i = 0; i < pMaskTexture->Get_TextureNum(); i++) {
			_wstring MaskTexturename = L"MASK_" + to_wstring(i);
			m_vecString_Mask.push_back(MaskTexturename);
		}
	}

	ImGui::SeparatorText("Mask image List");
	ImGui::PushItemWidth(200); // 크기조정
	if (ImGui::BeginListBox("##Mask_List"))
	{
		if (m_pTerrain != nullptr) {
			for (int n = 0; n < m_vecString_Mask.size(); n++)
			{
				bool is_selected = (m_iSelectTile == n);
				string MapName = wstring_to_string(m_vecString_Mask[n]);
				if (ImGui::Selectable(MapName.c_str(), is_selected))
				{
					m_iSelectTile = n;				// 현제 선택한 리스트 박스의 인덱스
				}

				if (is_selected)
					ImGui::SetItemDefaultFocus();
				// 반복문으로 리스트박스의 선택된 객체 찾기
			}
		}

		ImGui::EndListBox();
	}
	ImGui::PopItemWidth();

	ImGui::PushItemWidth(100); // 크기조정
	if (ImGui::Button("Add_Mask_Tex")) {
		if (FAILED(pMaskTexture->Add_MaskTexture()))
			return E_FAIL;
	}
	ImGui::SameLine();
	if (ImGui::Button("Del_Mask_Tex")) {
		if (FAILED(pMaskTexture->Delete_MaskTexture(m_iSelectTile)))
			return E_FAIL;
	}

	ImGui::PopItemWidth();

	return S_OK;
}

HRESULT CLevel_GamePlay::Terrain_Masking(_float fTimeDelta)
{
	ImGui::SeparatorText("Masking_Change");

	ImGui::PushItemWidth(150); // 크기조정
	static _int MaskRange = 1;
	ImGui::InputInt("Mask_Range", &MaskRange);
	static _int MaskValue = 0;
	ImGui::SliderInt("Mask_Value", &MaskValue, 0, 255);
	static _int MaskRGB = 0;
	ImGui::SliderInt("Mask_RGB", &MaskRGB, 0, 2);
	static bool bMask_Picking;
	ImGui::Checkbox("Mask Picking", &bMask_Picking);

	ImGui::PopItemWidth();

	if (bMask_Picking)
	{
		if (m_pGameInstance->Get_DIMouseState(DIMK_LBUTTON) && m_fTerrainTimeCheck > 0.1f) {
			_float2 test{};
			_float3 PickPos{};
			m_pGameInstance->Picking(&PickPos);
			
			test.x = (PickPos.x / _float(m_pTerrain->Get_VIBuffer()->Get_VerticesX())) * 256.f;
			test.y = (PickPos.z / _float(m_pTerrain->Get_VIBuffer()->Get_VerticesZ())) * 256.f;

			if (m_pGameInstance->Get_DIKeyState(DIK_Q)) {
				if (0.f <= test.x && 0.f <= test.y &&
					test.x <= 256.f && test.y <= 256.f) 
				{
					CTexture* pMaskTexture = m_pTerrain->Get_Texture(CTerrain::TEXTURE_MASK);
					pMaskTexture->Pick_ChangeMask(test, m_iSelectTile, MaskRange, MaskValue, MaskRGB);
				}
			}

			if (m_pGameInstance->Get_DIKeyState(DIK_E)) {
				if (0.f <= test.x && 0.f <= test.y &&
					test.x <= 256.f && test.y <= 256.f) 
				{
					CTexture* pMaskTexture = m_pTerrain->Get_Texture(CTerrain::TEXTURE_MASK);
					pMaskTexture->Pick_ChangeMask(test, m_iSelectTile, MaskRange, 255, 3);
				}
			}

		}

		if (m_fTerrainTimeCheck < 0.2f)
			m_fTerrainTimeCheck += fTimeDelta;
	}


	return S_OK;
}

HRESULT CLevel_GamePlay::Terrain_MaskSaveLoad(_float fTimeDelta)
{
	ImGui::SeparatorText("Save/Load_Mask.bmp");

	ImGui::PushItemWidth(300); // 크기조정
	if (ImGui::Button("  Save_Terrain_Height.bmp  ")) {
		if (filePath.length() < 55) {
			MSG_BOX(TEXT("Chocie FilePath"));
			return E_FAIL;
		}

		_tchar* wstrFilePath = stringToWchar(filePath);

		if (FAILED(m_pTerrain->Get_Texture(CTerrain::TEXTURE_MASK)->Save_MaskTexture(wstrFilePath, m_iSelectTile)))
		{
			MSG_BOX(TEXT("Failed to Save"));
		}

		delete[] wstrFilePath;
	}

	return S_OK;
}

#pragma endregion

#pragma region 초기화 코드
HRESULT CLevel_GamePlay::VectorClear()
{
	m_vecString_Mask.clear();

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
