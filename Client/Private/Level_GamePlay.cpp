#include "stdafx.h"
#include "..\Public\Level_GamePlay.h"

#include <codecvt>
#include <commdlg.h>
#include <fstream>
#include <iostream>

#include "FreeCamera.h"
#include "GameInstance.h"
#include "MapObject_Default.h"

#include "Terrain.h"
#include "Navigation.h"
#include "Cell.h"
#include "Light.h"

#include "..\imgui\ImGuizmo.h"
static ImGuizmo::OPERATION eGizmoType = { ImGuizmo::TRANSLATE };

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
	if (FAILED(OBJ_TEST()))
		return E_FAIL;
	
	//m_pGameInstance->PlayBGM(L"SD_BGM_Sual_Boss.ogg", 1.f, true);

	// ImGui 초기화
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
	io.Fonts->AddFontFromFileTTF("../Bin/Font/MaruBuri-Bold.ttf", 16.0f, NULL, io.Fonts->GetGlyphRangesKorean());

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

	ImGuizmo::Enable(true);

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

	ImGuizmo::SetOrthographic(false);
	ImGuizmo::SetRect(0, 0, g_iWinSizeX, g_iWinSizeY); // 뷰포트 크기 설정
	ImGuizmo::BeginFrame();


	ImGui::Begin("MainImgui");
	{
		Dialog_Imgui(fTimeDelta);
		ImGui::Spacing();
		Terrain_Imgui(fTimeDelta);
		ImGui::Spacing();
		GameObject_Imgui(fTimeDelta);
		ImGui::Spacing();
		Cell_Imgui(fTimeDelta);
		ImGui::Spacing();
		Light_Imgui(fTimeDelta);
	}
	ImGui::End();


	//   =============== ImGuizmo =============== 
	if (nullptr != m_pGameObj)  // 카메라가 내가 픽한 오브젝트 위치에 도달했을때 nullptr을 반환
	{
		static CGameObject* pPrePickedObj = nullptr;
		static _bool     bFristSetting = false;
		static _float4x4 WorldMatrix = {};
		static _float4x4 ViewMatrix = {};
		static _float4x4 ProjMatrix = {};


		if (!bFristSetting || pPrePickedObj != m_pGameObj)
		{
			XMStoreFloat4x4(&WorldMatrix, m_pGameObj->Get_TranformCom()->Get_WorldMatrix());

			pPrePickedObj = m_pGameObj;
			bFristSetting = true;
		}

		XMStoreFloat4x4(&ViewMatrix, m_pGameInstance->Get_Transform_Matrix(CPipeLine::D3DTS_VIEW));
		XMStoreFloat4x4(&ProjMatrix, m_pGameInstance->Get_Transform_Matrix(CPipeLine::D3DTS_PROJ));

		// eGizmoType = ImGuizmo::ROTATE
		// eGizmoType = ImGuizmo::ROTATE

		if (ImGuizmo::Manipulate((_float*)(&ViewMatrix), (_float*)(&ProjMatrix), eGizmoType, ImGuizmo::LOCAL, (_float*)(&WorldMatrix)))
		{		
			m_pGameObj->Get_TranformCom()->Set_WorldMatrix(WorldMatrix);
		}
	}

	VectorClear();

	if (m_pGameInstance->Get_DIKeyState_Once(DIK_T)) {
		//m_pGameInstance->Add_CloneObject_ToLayer(LEVEL_GAMEPLAY, TEXT("Layer_Player"), TEXT("Prototype_GameObject_Player"));
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

static _wstring char_to_wstring(const char* _char)
{
	string str(_char);
	wstring_convert<codecvt_utf8_utf16<wchar_t>> converter;
	return converter.from_bytes(str);
}

static wchar_t* stringToWchar(const string& str) {
	int size_needed = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), static_cast<int>(str.size()), NULL, 0);
	wchar_t* wcharStr = new wchar_t[size_needed + 1];
	MultiByteToWideChar(CP_UTF8, 0, str.c_str(), static_cast<int>(str.size()), wcharStr, size_needed);
	wcharStr[size_needed] = L'\0';

	return wcharStr; // 호출자가 delete[]로 해제 필요
}

HRESULT CLevel_GamePlay::Dialog_Imgui(_float fTimeDelta)
{
	string substring;
	if (filePath.length() > 55) {
		substring = filePath.substr(56);
	}
	else {
		substring = "None FilePath";
	}
	ImGui::Text("%s", substring.c_str());

	static bool bDialog_Start;
	ImGui::Checkbox("Open Dialog", &bDialog_Start);

	if (bDialog_Start) {
		ShowFileDialog();
	}

	return S_OK;
}

void CLevel_GamePlay::ShowFileDialog()
{
	// 파일 대화 상자 초기화
	ImGuiFileDialog::Instance()->OpenDialog(
		"OpenFileDialog",           // vKey
		"Select a File",           // vTitle
		".bmp,.dat",			   // vFilters
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
			// 마스크의 리스트 박스 && 마스크 이미지 변경
			if (FAILED(Terrain_Mask_ListBox(fTimeDelta)))
				return E_FAIL;

			// 마스킹 피킹 함수
			if (FAILED(Terrain_Masking(fTimeDelta)))
				return E_FAIL;

			// 마스크 저장/불러오기
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

	ImGui::SeparatorText("Wire_Change");
	if (ImGui::Button("  Terrain Wire  ")) {
		m_pTerrain->Change_Wire();
	}

	return S_OK;
}

HRESULT CLevel_GamePlay::Terrain_HeightChange(_float fTimeDelta)
{
	ImGui::SeparatorText("Height_Change");

	ImGui::PushItemWidth(150); // 크기조정
	static float TerrainRange = 0.f;
	ImGui::InputFloat("Height_Range", &TerrainRange, 0.5f);
	static float TerrainHeightUpMax = 0.f;
	ImGui::InputFloat("Height_UpMax", &TerrainHeightUpMax, 0.5f);
	static float TerrainHeightDownMax = 0.f;
	ImGui::InputFloat("Height_DownMax", &TerrainHeightDownMax, 0.5f);

	static bool bHeight_Picking;
	ImGui::Checkbox("Height Picking", &bHeight_Picking);

	ImGui::PopItemWidth();
	
	if (bHeight_Picking) 
	{
		if (m_pGameInstance->Get_DIMouseState(DIMK_LBUTTON) /*&& m_fTerrainTimeCheck > 0.2f*/) {
			// 피킹하고
			m_pGameInstance->Picking(&m_vPickPos);
			// 피킹정보 저장하고
			m_pGameInstance->Get_GlobalData()->Pick_Pos = m_vPickPos;
			
			if (m_pGameInstance->Get_DIKeyState(DIK_Q)) {
				// 값을 올린다
				m_pTerrain->Get_VIBuffer()->Change_Height(TerrainRange, TerrainHeightUpMax, true);
				m_fTerrainTimeCheck = 0.f;
			}

			if (m_pGameInstance->Get_DIKeyState(DIK_E)) {
				// 값을 내린다
				m_pTerrain->Get_VIBuffer()->Change_Height(TerrainRange, TerrainHeightDownMax, false);
				m_fTerrainTimeCheck = 0.f;
			}
			
		}

		/*if (m_fTerrainTimeCheck < 0.2f)
			m_fTerrainTimeCheck += fTimeDelta;*/
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

		filePath = "";
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

		filePath = "";
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
	Change_Mask(fTimeDelta);
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
			
			test.x = (PickPos.x / _float(m_pTerrain->Get_VIBuffer()->Get_VerticesX())) * 2048.f;
			test.y = (PickPos.z / _float(m_pTerrain->Get_VIBuffer()->Get_VerticesZ())) * 2048.f;

			if (m_pGameInstance->Get_DIKeyState(DIK_Q)) {
				if (0.f <= test.x && 0.f <= test.y &&
					test.x <= 2048.f && test.y <= 2048.f)
				{
					CTexture* pMaskTexture = m_pTerrain->Get_Texture(CTerrain::TEXTURE_MASK);
					pMaskTexture->Pick_ChangeMask(test, m_iSelectTile, MaskRange, MaskValue, MaskRGB);
				}
			}

			if (m_pGameInstance->Get_DIKeyState(DIK_E)) {
				if (0.f <= test.x && 0.f <= test.y &&
					test.x <= 2048.f && test.y <= 2048.f)
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
	if (ImGui::Button("  Save_Mask_bmp  ")) {
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

		filePath = "";
	}

	if (ImGui::Button("  Load_Mask_bmp  ")) {
		if (filePath.length() < 55) {
			MSG_BOX(TEXT("Chocie FilePath"));
			return E_FAIL;
		}

		_tchar* wstrFilePath = stringToWchar(filePath);

		if (FAILED(m_pTerrain->Get_Texture(CTerrain::TEXTURE_MASK)->Load_MaskTexture(wstrFilePath, m_iSelectTile)))
		{
			MSG_BOX(TEXT("Failed to Save"));
		}

		delete[] wstrFilePath;

		filePath = "";
	}

	return S_OK;
}

HRESULT CLevel_GamePlay::Change_Mask(_float fTimeDelta)
{
	static bool bMask_Change;
	ImGui::Checkbox("Open Mask Change Imgui", &bMask_Change);

	if (bMask_Change) {
		ImGui::Begin("Mask_Change");
		{
			static int SelectUseImage = 1;
			static int SelectStageingImgage = 13;

#pragma region BaseTexture
			ImGui::SeparatorText("Using Base Texture");
			ID3D11ShaderResourceView* UseBaseTexture = m_pTerrain->Get_Texture(CTerrain::TEXTURE_DIFFUSE)->Get_ShaderResourceView(0);
			if (UseBaseTexture) {
				ImGui::Image((void*)UseBaseTexture, ImVec2(100, 100)); // 원하는 크기로 설정
			}
			ImGui::SameLine(0.0f, 30.0f);
			ImVec2 buttonSize(100, 100);
			if (ImGui::Button("Base Change", buttonSize)) {
				m_pTerrain->Get_Texture(CTerrain::TEXTURE_DIFFUSE)->Swap_SRVs(0, SelectStageingImgage);
				m_pTerrain->Get_Texture(CTerrain::TEXTURE_NORMAL)->Swap_SRVs(0, SelectStageingImgage);

				if (m_iMaskingNum[0] == SelectStageingImgage)
					m_iMaskingNum[0] = 0;
				else
					m_iMaskingNum[0] = SelectStageingImgage;
			}

			ImGui::SameLine(0.0f, 30.0f);
			ImVec2 SavebuttonSize(100, 30);
			if (ImGui::Button("Save_Use_Mask", SavebuttonSize)) {

				if (filePath.length() < 55) {
					MSG_BOX(TEXT("Chocie FilePath"));
					return E_FAIL;
				}

				ofstream outFile(filePath, ios::binary);

				if (!outFile.is_open()) {
					MSG_BOX(TEXT("파일 저장 실패"));
					return E_FAIL;
				}

				for (_uint i = 0; i < 14; i++) {
					outFile.write(reinterpret_cast<const char*>(&m_iMaskingNum[i]), sizeof(_uint));
				}

				outFile.close();

				filePath = "";
			}

			ImGui::SameLine();

			if (ImGui::Button("Load_Use_Mask", SavebuttonSize)) {

				if (filePath.length() < 55) {
					MSG_BOX(TEXT("Chocie FilePath"));
					return E_FAIL;
				}

				ifstream inFile(filePath, ios::binary);

				if (!inFile.is_open()) {
					MSG_BOX(TEXT("파일 불러오기 실패"));
					return E_FAIL;
				}

				_uint i = 0;
				while (inFile.peek() != EOF) {
					inFile.read(reinterpret_cast<char*>(&m_iMaskingNum[i]), sizeof(_uint));
					i++;
				}

				inFile.close();

				for (_uint i = 0; i < 13; i++) {
					m_pTerrain->Get_Texture(CTerrain::TEXTURE_DIFFUSE)->Swap_SRVs(i, m_iMaskingNum[i]);
					m_pTerrain->Get_Texture(CTerrain::TEXTURE_NORMAL)->Swap_SRVs(i, m_iMaskingNum[i]);
				}

				filePath = "";
			}

#pragma endregion

#pragma region MaskTexture
			ImGui::SeparatorText("Using Mask Texture");

			string substring1, substring2;
			substring1 = "Mask_1_RGB";
			substring2 = "Mask_2_RGB";
			ImGui::Text("%s", substring1.c_str());
			ImGui::SameLine(0.0f, 280.0f);
			ImGui::Text("%s", substring2.c_str());

			// 이미지
			ID3D11ShaderResourceView* UseTexture1 = m_pTerrain->Get_Texture(CTerrain::TEXTURE_DIFFUSE)->Get_ShaderResourceView(1);
			if (UseTexture1) {
				ImGui::Image((void*)UseTexture1, ImVec2(100, 100)); // 원하는 크기로 설정
			}
			ImGui::SameLine();
			ID3D11ShaderResourceView* UseTexture2 = m_pTerrain->Get_Texture(CTerrain::TEXTURE_DIFFUSE)->Get_ShaderResourceView(2);
			if (UseTexture2) {
				ImGui::Image((void*)UseTexture2, ImVec2(100, 100)); // 원하는 크기로 설정
			}
			ImGui::SameLine();
			ID3D11ShaderResourceView* UseTexture3 = m_pTerrain->Get_Texture(CTerrain::TEXTURE_DIFFUSE)->Get_ShaderResourceView(3);
			if (UseTexture3) {
				ImGui::Image((void*)UseTexture3, ImVec2(100, 100)); // 원하는 크기로 설정
			}
			ImGui::SameLine(0.0f, 30.0f);
			ID3D11ShaderResourceView* UseTexture4 = m_pTerrain->Get_Texture(CTerrain::TEXTURE_DIFFUSE)->Get_ShaderResourceView(4);
			if (UseTexture4) {
				ImGui::Image((void*)UseTexture4, ImVec2(100, 100)); // 원하는 크기로 설정
			}
			ImGui::SameLine();
			ID3D11ShaderResourceView* UseTexture5 = m_pTerrain->Get_Texture(CTerrain::TEXTURE_DIFFUSE)->Get_ShaderResourceView(5);
			if (UseTexture5) {
				ImGui::Image((void*)UseTexture5, ImVec2(100, 100)); // 원하는 크기로 설정
			}
			ImGui::SameLine();
			ID3D11ShaderResourceView* UseTexture6 = m_pTerrain->Get_Texture(CTerrain::TEXTURE_DIFFUSE)->Get_ShaderResourceView(6);
			if (UseTexture6) {
				ImGui::Image((void*)UseTexture6, ImVec2(100, 100)); // 원하는 크기로 설정
			}

			// 라디오 버튼
			if (ImGui::RadioButton("Tile 1", SelectUseImage == 1)) {
				SelectUseImage = 1;
			}
			ImGui::SameLine(0.0f, 45.0f);
			if (ImGui::RadioButton("Tile 2", SelectUseImage == 2)) {
				SelectUseImage = 2;
			}
			ImGui::SameLine(0.0f, 45.0f);
			if (ImGui::RadioButton("Tile 3", SelectUseImage == 3)) {
				SelectUseImage = 3;
			}
			ImGui::SameLine(0.0f, 80.0f);
			if (ImGui::RadioButton("Tile 4", SelectUseImage == 4)) {
				SelectUseImage = 4;
			}
			ImGui::SameLine(0.0f, 45.0f);
			if (ImGui::RadioButton("Tile 5", SelectUseImage == 5)) {
				SelectUseImage = 5;
			}
			ImGui::SameLine(0.0f, 45.0f);
			if (ImGui::RadioButton("Tile 6", SelectUseImage == 6)) {
				SelectUseImage = 6;
			}

			string substring3, substring4;
			substring3 = "Mask_3_RGB";
			substring4 = "Mask_4_RGB";
			ImGui::Text("%s", substring3.c_str());
			ImGui::SameLine(0.0f, 280.0f);
			ImGui::Text("%s", substring4.c_str());

			// 이미지
			ID3D11ShaderResourceView* UseTexture7 = m_pTerrain->Get_Texture(CTerrain::TEXTURE_DIFFUSE)->Get_ShaderResourceView(7);
			if (UseTexture7) {
				ImGui::Image((void*)UseTexture7, ImVec2(100, 100)); // 원하는 크기로 설정
			}
			ImGui::SameLine();
			ID3D11ShaderResourceView* UseTexture8 = m_pTerrain->Get_Texture(CTerrain::TEXTURE_DIFFUSE)->Get_ShaderResourceView(8);
			if (UseTexture8) {
				ImGui::Image((void*)UseTexture8, ImVec2(100, 100)); // 원하는 크기로 설정
			}
			ImGui::SameLine();
			ID3D11ShaderResourceView* UseTexture9 = m_pTerrain->Get_Texture(CTerrain::TEXTURE_DIFFUSE)->Get_ShaderResourceView(9);
			if (UseTexture9) {
				ImGui::Image((void*)UseTexture9, ImVec2(100, 100)); // 원하는 크기로 설정
			}
			ImGui::SameLine(0.0f, 30.0f);
			ID3D11ShaderResourceView* UseTexture10 = m_pTerrain->Get_Texture(CTerrain::TEXTURE_DIFFUSE)->Get_ShaderResourceView(10);
			if (UseTexture10) {
				ImGui::Image((void*)UseTexture10, ImVec2(100, 100)); // 원하는 크기로 설정
			}
			ImGui::SameLine();
			ID3D11ShaderResourceView* UseTexture11 = m_pTerrain->Get_Texture(CTerrain::TEXTURE_DIFFUSE)->Get_ShaderResourceView(11);
			if (UseTexture11) {
				ImGui::Image((void*)UseTexture11, ImVec2(100, 100)); // 원하는 크기로 설정
			}
			ImGui::SameLine();
			ID3D11ShaderResourceView* UseTexture12 = m_pTerrain->Get_Texture(CTerrain::TEXTURE_DIFFUSE)->Get_ShaderResourceView(12);
			if (UseTexture12) {
				ImGui::Image((void*)UseTexture12, ImVec2(100, 100)); // 원하는 크기로 설정
			}

			// 라디오 버튼
			if (ImGui::RadioButton("Tile 7", SelectUseImage == 7)) {
				SelectUseImage = 7;
			}
			ImGui::SameLine(0.0f, 45.0f);
			if (ImGui::RadioButton("Tile 8", SelectUseImage == 8)) {
				SelectUseImage = 8;
			}
			ImGui::SameLine(0.0f, 45.0f);
			if (ImGui::RadioButton("Tile 9", SelectUseImage == 9)) {
				SelectUseImage = 9;
			}
			ImGui::SameLine(0.0f, 80.0f);
			if (ImGui::RadioButton("Tile 10", SelectUseImage == 10)) {
				SelectUseImage = 10;
			}
			ImGui::SameLine(0.0f, 45.0f);
			if (ImGui::RadioButton("Tile 11", SelectUseImage == 11)) {
				SelectUseImage = 11;
			}
			ImGui::SameLine(0.0f, 45.0f);
			if (ImGui::RadioButton("Tile 12", SelectUseImage == 12)) {
				SelectUseImage = 12;
			}

			ImVec2 buttonSize2(200, 30);
			if (ImGui::Button("Mask Change", buttonSize2)) {
				m_pTerrain->Get_Texture(CTerrain::TEXTURE_DIFFUSE)->Swap_SRVs(SelectUseImage, SelectStageingImgage);
				m_pTerrain->Get_Texture(CTerrain::TEXTURE_NORMAL)->Swap_SRVs(SelectUseImage, SelectStageingImgage);

				if (m_iMaskingNum[SelectUseImage] == SelectStageingImgage)
					m_iMaskingNum[SelectUseImage] = SelectUseImage;
				else
					m_iMaskingNum[SelectUseImage] = SelectStageingImgage;
			}
#pragma endregion

#pragma region Stageing Texture
			ImGui::SeparatorText("Stageing Mask Texture");

			ImGui::BeginChild("ScrollableRegion", ImVec2(0, 300), true, ImGuiWindowFlags_HorizontalScrollbar);
			ID3D11ShaderResourceView* UseTextureStageing{};

			for (_uint i = 13; i < 81; i++) {

				UseTextureStageing = m_pTerrain->Get_Texture(CTerrain::TEXTURE_DIFFUSE)->Get_ShaderResourceView(i);
				if (UseTextureStageing) {
					ImGui::Image((void*)UseTextureStageing, ImVec2(100, 100)); // 원하는 크기로 설정
				}
				if (i % 6 != 0) {
					ImGui::SameLine(0.0f, 10.0f);
				}
				else {
					for (_uint j = 0; j < 6; j++) {
						string TileNum = "Tile_" + to_string(i + j);
						if (ImGui::RadioButton(TileNum.c_str(), SelectStageingImgage == i + j - 5)) {
							SelectStageingImgage = i + j - 5;
						}
						if(j != 5)
							ImGui::SameLine(0.0f, 40.0f);
					}
				}
			}
			ImGui::EndChild();
#pragma endregion

		}
		ImGui::End();
	}
	

	return S_OK;
}

#pragma endregion

#pragma region GameObject 작동 코드

HRESULT CLevel_GamePlay::GameObject_Imgui(_float fTimeDelta)
{
	if (ImGui::CollapsingHeader("GameObject"))
	{
		// 오브젝트 생성
		ImGui::Spacing();
		GameObject_Create_GameObject(fTimeDelta);

		if (FAILED(GameObject_Model_ListBox(fTimeDelta)))
			return E_FAIL;

		if (ImGui::CollapsingHeader("Layer List"))
		{
			// 레이어 리스트 박스
			if (FAILED(GameObject_Layer_ListBox(fTimeDelta)))
				return E_FAIL;

			if (FAILED(GameObject_Save_Load(fTimeDelta)))
				return E_FAIL;
		}

		if (m_pMap_Layers != nullptr) 
		{
			ImGui::Begin("Information Imgui"); 
			{
				if (ImGui::CollapsingHeader("GameObjct List"))
				{
					// 게임오브젝트 리스트 박스
					if (FAILED(GameObject_Object_ListBox(fTimeDelta)))
						return E_FAIL;

					// 게임오브젝트 정보 컨트롤
					if (FAILED(GameObject_Pos_Scal_Turn()))
						return E_FAIL;

					// 정보에 따른 데이터 저장방식들
					GameObject_Save_Load_wstring();
					GameObject_Save_Load_Node();
				}
			}
			ImGui::End();
		}
	}

	return S_OK;
}

HRESULT CLevel_GamePlay::GameObject_Create_GameObject(_float fTimeDelta)
{
	ImGui::SeparatorText("GameObject_Add");
	ImGui::PushItemWidth(150); // 크기조정

	static char Layertag[256] = {};
	ImGui::InputText("Layertag##Monster", Layertag, IM_ARRAYSIZE(Layertag));
	
	if (ImGui::Button("  Create GameObject  ")) {
		CMapObject_Default::MAPOBJECT_DESC Desc{};
		_wstring wLayertag{};
		if (Layertag[0] == '\0')						// 입력받은 레이어 테그가 없을경우
			wLayertag = TEXT("Layer_Defalt");		// Layer_Defalt 레이어로 넣어준다
		else
			wLayertag = char_to_wstring(Layertag);	// 입력받은 값이 있다면 해당값으로 레이어를 선언한다

		Desc.LayerTag = wLayertag;
		Desc.ModelNum = m_iSelectModel;
		if (FAILED(m_pGameInstance->Add_CloneObject_ToLayer(LEVEL_GAMEPLAY, wLayertag, TEXT("Prototype_GameObject_MapObject_Default"), &Desc)))
			return E_FAIL;
	}

	ImGui::PopItemWidth();

	static bool bObjCreate_Picking;
	ImGui::Spacing();
	ImGui::Checkbox("Obj Picking", &bObjCreate_Picking);
	static bool bObjRandom;
	ImGui::Spacing();
	ImGui::Checkbox("Obj Random", &bObjRandom);


	if (bObjCreate_Picking) 
	{
		if (m_pGameInstance->Get_DIMouseState_Once(DIMK_LBUTTON)) {
			_float3 PickPos{};
			m_pGameInstance->Picking(&PickPos);

			if (m_pGameInstance->Get_DIKeyState(DIK_Q)) {
				CMapObject_Default::MAPOBJECT_DESC Desc{};
				_wstring wLayertag{};
				if (Layertag[0] == '\0')					// 입력받은 레이어 테그가 없을경우
					wLayertag = TEXT("Layer_Defalt");		// Layer_Defalt 레이어로 넣어준다
				else
					wLayertag = char_to_wstring(Layertag);	// 입력받은 값이 있다면 해당값으로 레이어를 선언한다

				if (m_StringLayerName.size() > 0)
					wLayertag = m_StringLayerName;

				Matrix WorldMatrix = Matrix::Identity;
				if (bObjRandom) {
					Matrix scaleMatrix = Matrix::CreateScale(m_pGameInstance->Get_Random(0.7f, 1.2f));
					_float rotationAngle = XMConvertToRadians(m_pGameInstance->Get_Random(0.f, 360.f));
					Matrix rotationMatrix = Matrix::CreateRotationY(rotationAngle);

					WorldMatrix *= scaleMatrix;       // 스케일 적용
					WorldMatrix *= rotationMatrix;    // Y축 회전 적용
				}

				WorldMatrix.Translation(Vector3(PickPos));

				Desc.LayerTag = wLayertag;
				Desc.ModelNum = m_iSelectModel;
				Desc.WorldMatrix = WorldMatrix;

				if (FAILED(m_pGameInstance->Add_CloneObject_ToLayer(LEVEL_GAMEPLAY, wLayertag, TEXT("Prototype_GameObject_MapObject_Default"), &Desc)))
					return E_FAIL;
			}
		}
	}

	return S_OK;
}

HRESULT CLevel_GamePlay::GameObject_Model_ListBox(_float fTimeDelta)
{
	GLOBAL_DATA GDesc = *m_pGameInstance->Get_GlobalData();

	ImGui::SeparatorText("Model List");
	ImGui::PushItemWidth(300); // 크기조정
	if (ImGui::BeginListBox("##Model_List"))
	{
		for (int n = 0; n < GDesc.ModelName.size(); n++)
		{
			bool is_selected = (m_iSelectModel == n);
			string ModelName = GDesc.ModelName[n];
			if (ImGui::Selectable(ModelName.c_str(), is_selected))
			{
				m_iSelectModel = n;		// 현제 레이어 리스트 박스의 인덱스
			}

			if (is_selected)
				ImGui::SetItemDefaultFocus();
		}

		ImGui::EndListBox();
	}
	ImGui::PopItemWidth();

	return S_OK;
}

HRESULT CLevel_GamePlay::GameObject_Layer_ListBox(_float fTimeDelta)
{
	// 레이어 가져오기
	m_pMap_Layers = m_pGameInstance->Get_Map_Layer();

	// 리스트 박스용 String 값 넣기
	for (auto& iter : m_pMap_Layers[LEVEL_GAMEPLAY]) {
		m_vecString_Map_Layer.push_back(iter.first.c_str());
	}

	ImGui::SeparatorText("Layer List");
	ImGui::PushItemWidth(200); // 크기조정
	if (ImGui::BeginListBox("##Map_Layer_List")) 
	{
		for (int n = 0; n < m_vecString_Map_Layer.size(); n++)
		{
			bool is_selected = (m_iSelectMap == n);
			string MapName = wstring_to_string(m_vecString_Map_Layer[n]);
			if (ImGui::Selectable(MapName.c_str(), is_selected))
			{
				// 기존 선택중인 객체 셋팅 초기화
				if (m_pGameObj != nullptr) {
					static_cast<CMapObject_Default*>(m_pGameObj)->Set_UseShader(1);
					static_cast<CMapObject_Default*>(m_pGameObj)->Set_InstanceRender(true);
				}

				m_iSelectMap = n;		// 현제 레이어 리스트 박스의 인덱스

				if (MapName != "Layer_Terrain" &&
					MapName != "Layer_BackGround" &&
					MapName != "Layer_Camera") // 선택되어지지 말아야하는 객체는 여기서 레이어를 예외처리 
				{
					auto iter = m_pMap_Layers[LEVEL_GAMEPLAY].begin();
					advance(iter, n);						// 이터레이터를 이동
					m_pLayer = (*iter).second;				// 현제 선택중인 Layer 에 대입
					m_StringLayerName = (*iter).first;		// 현제 선택중인 Layer 의 map 이름

					// 현제 선택중인 Layer 의 0번쨰 오브젝트를 현제 선택한 오브젝트로 지정
					m_pGameObj = m_pGameInstance->Get_Object(LEVEL_GAMEPLAY, m_StringLayerName);

					// GameObject 리스트 박스 정보 갱신
					GameObject_vecStringSet();
				}
			}

			if (is_selected)
				ImGui::SetItemDefaultFocus();
		}

		ImGui::EndListBox();
	}
	ImGui::PopItemWidth();

	return S_OK;
}

HRESULT CLevel_GamePlay::GameObject_Save_Load(_float fTimeDelta)
{
	ImVec2 buttonSize(100, 30);
	if (ImGui::Button("Save_Obj", buttonSize)) {

		if (m_StringLayerName.length() < 1) {
			MSG_BOX(TEXT("Chocie Layer"));
			return E_FAIL;
		}

		if (filePath.length() < 55) {
			MSG_BOX(TEXT("Chocie FilePath"));
			return E_FAIL;
		}

		ofstream outFile(filePath, ios::binary);

		if (!outFile.is_open()) {
			MSG_BOX(TEXT("파일 저장 실패"));
			return E_FAIL;
		}

		for (const auto& LayerList : *(m_pGameInstance->Get_ObjectList(LEVEL_GAMEPLAY, m_StringLayerName))) {

			CMapObject_Default::MAPOBJECT_DESC Desc{};

			Vector3 scale = static_cast<CMapObject_Default*>(LayerList)->Get_TranformCom()->Get_Scaled();
			_float Maxscale = max(max(scale.x, scale.y), scale.z);

			Desc.WorldMatrix = LayerList->Get_TranformCom()->Get_WorldMatrix();
			Desc.ModelNum = static_cast<CMapObject_Default*>(LayerList)->Get_UseModel();
			Desc.CullRadiuse = static_cast<CMapObject_Default*>(LayerList)->Get_Radiuse() * Maxscale;

			outFile.write(reinterpret_cast<const char*>(&Desc.WorldMatrix), sizeof(_matrix));
			outFile.write(reinterpret_cast<const char*>(&Desc.ModelNum), sizeof(_uint));
			outFile.write(reinterpret_cast<const char*>(&Desc.CullRadiuse), sizeof(_float));
		}

		outFile.close();

		filePath = "";
		MSG_BOX(TEXT("파일 저장 완료"));
	}
	ImGui::SameLine();
	if (ImGui::Button("Load_Obj", buttonSize)) {

		if (m_StringLayerName.length() < 1) {
			MSG_BOX(TEXT("Chocie Layer"));
			return E_FAIL;
		}

		if (filePath.length() < 55) {
			MSG_BOX(TEXT("Chocie FilePath"));
			return E_FAIL;
		}

		ifstream inFile(filePath, ios::binary);

		if (!inFile.is_open()) {
			MSG_BOX(TEXT("파일 불러오기 실패"));
			return E_FAIL;
		}

		while (inFile.peek() != EOF) {
			CMapObject_Default::MAPOBJECT_DESC Desc{};

			inFile.read(reinterpret_cast<char*>(&Desc.WorldMatrix), sizeof(_matrix));
			inFile.read(reinterpret_cast<char*>(&Desc.ModelNum), sizeof(_uint));
			inFile.read(reinterpret_cast<char*>(&Desc.CullRadiuse), sizeof(_float));
			Desc.LayerTag = m_StringLayerName;

			if (FAILED(m_pGameInstance->Add_CloneObject_ToLayer(LEVEL_GAMEPLAY, m_StringLayerName, TEXT("Prototype_GameObject_MapObject_Default"), &Desc)))
				return E_FAIL;
		}

		inFile.close();

		filePath = "";
	}
	
	return S_OK;
}

static int  FindModelNum = -1;

HRESULT CLevel_GamePlay::GameObject_Object_ListBox(_float fTimeDelta)
{
	ImGui::SeparatorText("GameObj List");
	ImGui::PushItemWidth(200); // 크기조정

	_int ModelNum = -1;
	if (m_pGameObj != nullptr) {
		ModelNum = _int(static_cast<CMapObject_Default*>(m_pGameObj)->Get_UseModel());
	}
	ImGui::Text("ModelNum = %d", ModelNum);

	static bool bFind_GameObject;
	ImGui::Checkbox("Find Picking", &bFind_GameObject);
	
	_float3 FindPos{};
	_uint	FindNum{};

	if (bFind_GameObject) {
		if (m_pGameInstance->Get_DIMouseState_Once(DIMK_LBUTTON) && m_pGameInstance->Get_DIKeyState(DIK_E)) {
			if (m_pGameInstance->Picking(&FindPos, &FindNum))
			{
				// 기존 객체 변경 점
				// 사용중인 쉐이더 , 인스턴싱 유무
				if (m_pGameObj != nullptr) {
					static_cast<CMapObject_Default*>(m_pGameObj)->Set_UseShader(1);
					static_cast<CMapObject_Default*>(m_pGameObj)->Set_InstanceRender(true);
				}
				

				// 현제 선택한 리스트 박스의 인덱스
				m_iSelectGameObj = FindNum;
				// 현제 선택한 게임 오브젝트를 대입한다
				m_pGameObj = m_pGameInstance->Get_Object(LEVEL_GAMEPLAY, m_StringLayerName, FindNum);

				if (m_pGameObj != nullptr) {
					// 현제 선택중인 오브젝트의 TransformCom 객체를 셋팅한다.
					m_pTransformCom = m_pGameObj->Get_TranformCom();
					// 피킹된 객체 쉐이더 변경
					static_cast<CMapObject_Default*>(m_pGameObj)->Set_UseShader(3);
					// 피킹된 객체 인스턴싱 랜더에서 기본 랜더로 교체
					static_cast<CMapObject_Default*>(m_pGameObj)->Set_InstanceRender(false);

					FindModelNum = _int(static_cast<CMapObject_Default*>(m_pGameObj)->Get_UseModel());
				}
			}
		}
	}
	
	
	if (ImGui::BeginListBox("##GameObj_List"))
	{
		for (_uint n = 0; n < m_iGameObjListSize; n++)
		{
			bool is_selected = (m_iSelectGameObj == n);
			if (m_vecString_GameObj.size() > 0) {

				string MapName = m_vecString_GameObj[n];
				
				if (ImGui::Selectable(MapName.c_str(), is_selected))
				{
					// 현제 선택한 리스트 박스의 인덱스
					m_iSelectGameObj = n;
					// 현제 선택한 게임 오브젝트를 대입한다
					m_pGameObj = m_pGameInstance->Get_Object(LEVEL_GAMEPLAY, m_StringLayerName, n);
				}

				if (is_selected)
					ImGui::SetItemDefaultFocus();
				// 반복문으로 리스트박스의 선택된 객체 찾기
			}
		}

		if (m_pGameObj != nullptr)
		{
			// 현제 선택중인 오브젝트의 TransformCom 객체를 셋팅한다.
			m_pTransformCom = m_pGameObj->Get_TranformCom();
		}

		ImGui::EndListBox();
	}
	ImGui::PopItemWidth();
	ImGui::SameLine();
	if (ImGui::Button("DeleteObj")) {
		if (m_pGameObj != nullptr) {
			m_pGameObj->Set_Dead(true);

			m_iSelectGameObj--;
			m_pGameObj = m_pGameInstance->Get_Object(LEVEL_GAMEPLAY, m_StringLayerName, m_iSelectGameObj);
			
			if (m_pGameObj != nullptr)
			{
				// 현제 선택중인 오브젝트의 TransformCom 객체를 셋팅한다.
				m_pTransformCom = m_pGameObj->Get_TranformCom();
			}
			else
			{
				m_pTransformCom = nullptr;
			}

			_uint SetDepthNum = 0;
			for (auto& iter : *(m_pGameInstance->Get_ObjectList(LEVEL_GAMEPLAY, m_StringLayerName))) 
			{
				if (iter->Get_Dead())
					continue;

				iter->Set_DepthNum(SetDepthNum);
				SetDepthNum++;
			}
			
		}
	}

	GameObject_Swap_Layer_Object();
	GameObject_Swap_Layer_Model();

	if (ImGui::Button("ListUpdate")) {
		if (m_StringLayerName != L"\0") {
			GameObject_vecStringSet();
		}
	}
	ImGui::SameLine();
	if (ImGui::Button("DepthNumUpdate")) {
		if (m_StringLayerName != L"\0") {
			_uint SetDepthNum = 0;
			for (auto& iter : *(m_pGameInstance->Get_ObjectList(LEVEL_GAMEPLAY, m_StringLayerName)))
			{
				iter->Set_DepthNum(SetDepthNum);
				SetDepthNum++;
			}
		}
	}

	GameObject_Clear();

	return S_OK;
}

HRESULT CLevel_GamePlay::GameObject_vecStringSet()
{
	// 현제 선택중인 Layer 의 GameObj 리스트 크기
	m_iGameObjListSize = _uint(m_pGameInstance->Get_ObjectList(LEVEL_GAMEPLAY, m_StringLayerName)->size());
	// 기존에 사용중이던 정보 클리어
	m_vecString_GameObj.clear();
	// 갱신된 새로운 정보 대입
	for (_uint i = 0; i < m_iGameObjListSize; i++) 
	{
		_wstring GameObjlistboxname = m_StringLayerName + L"_" + to_wstring(i);
		m_vecString_GameObj.push_back(wstring_to_string(GameObjlistboxname));
	}

	return S_OK;
}

HRESULT CLevel_GamePlay::GameObject_Pos_Scal_Turn()
{
	if (m_pGameObj != nullptr && m_pTransformCom != nullptr)
	{
		// 랜더링 방식
		_uint GameObjectRenderChange = static_cast<CMapObject_Default*>(m_pGameObj)->Get_UseShader();
		// 라디오 버튼
		if (ImGui::RadioButton("Defalt", GameObjectRenderChange == 1)) {
			GameObjectRenderChange = 1;
		}
		ImGui::SameLine(0.0f, 10.0f);
		if (ImGui::RadioButton("Wire", GameObjectRenderChange == 2)) {
			GameObjectRenderChange = 2;
		}
		ImGui::SameLine(0.0f, 10.0f);
		if (ImGui::RadioButton("Pick", GameObjectRenderChange == 3)) {
			GameObjectRenderChange = 3;
		}
		static_cast<CMapObject_Default*>(m_pGameObj)->Set_UseShader(GameObjectRenderChange);

		// 회전,크기,위치
		Vector3 Pos = m_pTransformCom->Get_State(CTransform::STATE_POSITION);
		Vector3 Scal = m_pTransformCom->Get_Scaled();
		CGameObject::GAMEOBJECT_DESC Objdesc = m_pGameObj->Get_GameObjDesc();
		Vector3 Turn = Objdesc.Angle;

		static ImGuiSliderFlags flags = ImGuiSliderFlags_None;
#pragma region 위치
		ImGui::SeparatorText("Position");
		ImGui::SameLine();
		ImGui::Text(" ", Pos.x, Pos.y, Pos.z);

		ImGui::PushItemWidth(200);
		ImGui::DragFloat("PosX ", &Pos.x, 0.05f, -FLT_MAX, +FLT_MAX, "%.3f", flags);
		ImGui::PopItemWidth();
		ImGui::SameLine();
		ImGui::PushItemWidth(100);
		ImGui::InputFloat("##input PosX", &Pos.x, 0.01f, 1.0f, "%.3f");
		ImGui::PopItemWidth();

		ImGui::PushItemWidth(200);
		ImGui::DragFloat("PosY ", &Pos.y, 0.05f, -FLT_MAX, +FLT_MAX, "%.3f", flags);
		ImGui::PopItemWidth();
		ImGui::SameLine();
		ImGui::PushItemWidth(100);
		ImGui::InputFloat("##input PosY", &Pos.y, 0.01f, 1.0f, "%.3f");
		ImGui::PopItemWidth();

		ImGui::PushItemWidth(200);
		ImGui::DragFloat("PosZ ", &Pos.z, 0.05f, -FLT_MAX, +FLT_MAX, "%.3f", flags);
		ImGui::PopItemWidth();
		ImGui::SameLine();
		ImGui::PushItemWidth(100);
		ImGui::InputFloat("##input PosZ", &Pos.z, 0.01f, 1.0f, "%.3f");
		ImGui::PopItemWidth();
#pragma endregion

#pragma region 크기
		ImGui::SeparatorText("Scale");
		ImGui::SameLine();
		ImGui::Text(" ", Scal.x, Scal.y, Scal.z);

		ImGui::PushItemWidth(200);
		ImGui::DragFloat("ScalX", &Scal.x, 0.05f, 0.1f, +FLT_MAX, "%.3f", flags);
		ImGui::PopItemWidth();
		ImGui::SameLine();
		ImGui::PushItemWidth(100);
		ImGui::InputFloat("##input ScalX", &Scal.x, 0.01f, 1.0f, "%.3f");
		ImGui::PopItemWidth();

		ImGui::PushItemWidth(200);
		ImGui::DragFloat("ScalY", &Scal.y, 0.05f, 0.1f, +FLT_MAX, "%.3f", flags);
		ImGui::PopItemWidth();
		ImGui::SameLine();
		ImGui::PushItemWidth(100);
		ImGui::InputFloat("##input ScalY", &Scal.y, 0.01f, 1.0f, "%.3f");
		ImGui::PopItemWidth();

		ImGui::PushItemWidth(200);
		ImGui::DragFloat("ScalZ", &Scal.z, 0.05f, 0.1f, +FLT_MAX, "%.3f", flags);
		ImGui::PopItemWidth();
		ImGui::SameLine();
		ImGui::PushItemWidth(100);
		ImGui::InputFloat("##input ScalZ", &Scal.z, 0.01f, 1.0f, "%.3f");
		ImGui::PopItemWidth();

		static _float All_Scale = 0.f;
		ImGui::PushItemWidth(50);
		ImGui::InputFloat("##All_Scale", &All_Scale);
		ImGui::PopItemWidth();
		ImGui::SameLine();

		if (ImGui::Button("  Scale_Up  ")) {
			Scal.x += All_Scale;
			Scal.y += All_Scale;
			Scal.z += All_Scale;
		}
		ImGui::SameLine();
		if (ImGui::Button(" Scale_Down ")) {
			Scal.x -= All_Scale;
			Scal.y -= All_Scale;
			Scal.z -= All_Scale;
		}
#pragma endregion

#pragma region 회전/방향
		ImGui::SeparatorText("Spin");
		ImGui::Text(" ", Turn.x, Turn.y, Turn.z);
		
		ImGui::PushItemWidth(200);
		ImGui::DragFloat("SpinX", &Turn.x, 0.2f, 0.f, 360.f, "%.3f", flags);
		ImGui::PopItemWidth();
		ImGui::SameLine();
		ImGui::PushItemWidth(100);
		ImGui::InputFloat("##input Angle X", &Turn.x, 0.01f, 1.0f, "%.3f");
		ImGui::PopItemWidth();

		ImGui::PushItemWidth(200);
		ImGui::DragFloat("SpinY", &Turn.y, 0.2f, 0.f, 360.f, "%.3f", flags);
		ImGui::PopItemWidth();
		ImGui::SameLine();
		ImGui::PushItemWidth(100);
		ImGui::InputFloat("##input Angle Y", &Turn.y, 0.01f, 1.0f, "%.3f");
		ImGui::PopItemWidth();

		ImGui::PushItemWidth(200);
		ImGui::DragFloat("SpinZ", &Turn.z, 0.2f, 0.f, 360.f, "%.3f", flags);
		ImGui::PopItemWidth();
		ImGui::SameLine();
		ImGui::PushItemWidth(100);
		ImGui::InputFloat("##input Angle Z", &Turn.z, 0.01f, 1.0f, "%.3f");
		ImGui::PopItemWidth();

		Objdesc.Angle = Turn;
#pragma endregion

		m_pTransformCom->Set_State(CTransform::STATE_POSITION, Pos);
		m_pTransformCom->Set_Scaled(Scal.x, Scal.y, Scal.z);
		m_pTransformCom->All_Rotation(Turn);
		m_pGameObj->Set_GameObjDesc(Objdesc);
	}

	return S_OK;
}

HRESULT CLevel_GamePlay::GameObject_Save_Load_wstring()
{
	static bool bSave_Load_wstring;
	ImGui::Checkbox("Save_Load_wstring", &bSave_Load_wstring);

	if (bSave_Load_wstring)
	{
		ImGui::Begin("Save_Load_wstring");

		if (m_pGameObj != nullptr) {
			static char Name[256] = {};
			string stdName = wstring_to_string(static_cast<CMapObject_Default*>(m_pGameObj)->Get_SubDesc()->Name);
			strncpy_s(Name, stdName.c_str(), sizeof(Name));
			Name[sizeof(Name) - 1] = '\0';

			ImGui::InputText(u8"SubData_Name", Name, sizeof(Name));

			static_cast<CMapObject_Default*>(m_pGameObj)->Get_SubDesc()->Name = char_to_wstring(Name);
		}

		ImVec2 buttonSize(100, 30);
		if (ImGui::Button("Save_wstring", buttonSize)) {

			if (filePath.length() < 55) {
				MSG_BOX(TEXT("Chocie FilePath"));
				goto EndButton_Save_wstring;
			}

			ofstream outFile(filePath, ios::binary);

			if (!outFile.is_open()) {
				MSG_BOX(TEXT("파일 저장 실패"));
				goto EndButton_Save_wstring;
			}

			for (const auto& LayerList : *(m_pGameInstance->Get_ObjectList(LEVEL_GAMEPLAY, m_StringLayerName))) {

				CMapObject_Default::MAPOBJECT_DESC Desc{};
				CMapObject_Default::SUB_DESC SubDesc{};

				Desc.WorldMatrix = LayerList->Get_TranformCom()->Get_WorldMatrix();
				Desc.ModelNum = static_cast<CMapObject_Default*>(LayerList)->Get_UseModel();
				Desc.CullRadiuse = static_cast<CMapObject_Default*>(LayerList)->Get_Radiuse();
				SubDesc.Name = static_cast<CMapObject_Default*>(LayerList)->Get_SubDesc()->Name;

				outFile.write(reinterpret_cast<const char*>(&Desc.WorldMatrix), sizeof(_matrix));
				outFile.write(reinterpret_cast<const char*>(&Desc.ModelNum), sizeof(_uint));
				outFile.write(reinterpret_cast<const char*>(&Desc.CullRadiuse), sizeof(_float));

				size_t length = SubDesc.Name.size();
				outFile.write(reinterpret_cast<const char*>(&length), sizeof(length));
				outFile.write(reinterpret_cast<const char*>(SubDesc.Name.data()), length * sizeof(_tchar));
			}

			outFile.close();

			filePath = "";
		}
	EndButton_Save_wstring:

		ImGui::SameLine();
		if (ImGui::Button("Load_wstring", buttonSize)) {

			if (filePath.length() < 55) {
				MSG_BOX(TEXT("Chocie FilePath"));
				goto EndButton_Load_wstring;
			}

			ifstream inFile(filePath, ios::binary);

			if (!inFile.is_open()) {
				MSG_BOX(TEXT("파일 불러오기 실패"));
				goto EndButton_Load_wstring;
			}

			while (inFile.peek() != EOF) {
				CMapObject_Default::MAPOBJECT_DESC Desc{};
				CMapObject_Default::SUB_DESC SubDesc{};

				inFile.read(reinterpret_cast<char*>(&Desc.WorldMatrix), sizeof(_matrix));
				inFile.read(reinterpret_cast<char*>(&Desc.ModelNum), sizeof(_uint));
				inFile.read(reinterpret_cast<char*>(&Desc.CullRadiuse), sizeof(_float));
				Desc.LayerTag = m_StringLayerName;

				size_t length{};
				inFile.read(reinterpret_cast<char*>(&length), sizeof(length));
				SubDesc.Name.resize(length, L'\0');
				inFile.read(reinterpret_cast<char*>(&SubDesc.Name[0]), length * sizeof(_tchar));

				CGameObject* GameObj{};
				if (FAILED(m_pGameInstance->Add_GameObject_Out(TEXT("Prototype_GameObject_MapObject_Default"), LEVEL_GAMEPLAY, m_StringLayerName.c_str(), GameObj, &Desc)))
					return E_FAIL;

				static_cast<CMapObject_Default*>(GameObj)->Get_SubDesc()->Name = SubDesc.Name;
			}

			inFile.close();

			filePath = "";
		}
	EndButton_Load_wstring:

		ImGui::End();
	}

	return S_OK;
}

HRESULT CLevel_GamePlay::GameObject_Save_Load_Node()
{
	static bool bSave_Load_Node;
	ImGui::Checkbox("Save_Load_Node", &bSave_Load_Node);

	if (bSave_Load_Node) 
	{
		ImGui::Begin("Save_Load_Node");

		if (m_pGameObj != nullptr) {
			vector<Vector3> Node = static_cast<CMapObject_Default*>(m_pGameObj)->Get_SubDesc()->Node;

			static bool bPicking_Node;
			ImGui::Checkbox("Picking_Node", &bPicking_Node);

			if (bPicking_Node && m_pGameInstance->Get_DIKeyState(DIK_Q))
			{
				if (m_pGameInstance->Get_DIMouseState_Once(DIMK_LBUTTON))
				{
					_float3 PickPos{};
					m_pGameInstance->Picking(&PickPos);
					static_cast<CMapObject_Default*>(m_pGameObj)->Get_SubDesc()->Node.push_back(PickPos);
				}
			}

			static int DeleteNodeNum = 0;

			ImGui::PushItemWidth(200); 
			ImGui::InputInt("Delete_Node_Num", &DeleteNodeNum, 0, 0, 1); 
			ImGui::PopItemWidth(); ImGui::SameLine();
			if (ImGui::Button("Delete_Node")) {
				if (DeleteNodeNum < Node.size()) {
					auto iter = static_cast<CMapObject_Default*>(m_pGameObj)->Get_SubDesc()->Node.begin();

					for (_uint i = 0; i < _uint(DeleteNodeNum); i++) {
						iter++;
					}

					static_cast<CMapObject_Default*>(m_pGameObj)->Get_SubDesc()->Node.erase(iter);
				}
			}

			_uint NodeNum = 0;
			for (auto& pNode : Node) {
				ImGui::Text(" %d : ", NodeNum); ImGui::SameLine();
				ImGui::Text(" %.2f ", pNode.x); ImGui::SameLine();
				ImGui::Text(" %.2f ", pNode.y); ImGui::SameLine();
				ImGui::Text(" %.2f ", pNode.z);
				NodeNum++;
				ImGui::Spacing();
			}
		}

		ImVec2 buttonSize(100, 30);
		if (ImGui::Button("Save_Node", buttonSize)) {

			if (filePath.length() < 55) {
				MSG_BOX(TEXT("Chocie FilePath"));
				goto EndButton_Save_Node;
			}

			ofstream outFile(filePath, ios::binary);

			if (!outFile.is_open()) {
				MSG_BOX(TEXT("파일 저장 실패"));
				goto EndButton_Save_Node;
			}

			for (const auto& LayerList : *(m_pGameInstance->Get_ObjectList(LEVEL_GAMEPLAY, m_StringLayerName))) {

				CMapObject_Default::MAPOBJECT_DESC Desc{};
				CMapObject_Default::SUB_DESC* SubDesc{};

				Desc.WorldMatrix = LayerList->Get_TranformCom()->Get_WorldMatrix();
				Desc.ModelNum = static_cast<CMapObject_Default*>(LayerList)->Get_UseModel();
				Desc.CullRadiuse = static_cast<CMapObject_Default*>(LayerList)->Get_Radiuse();
				SubDesc = static_cast<CMapObject_Default*>(LayerList)->Get_SubDesc();
				_uint NodeSize = _uint(SubDesc->Node.size());

				outFile.write(reinterpret_cast<const char*>(&Desc.WorldMatrix), sizeof(_matrix));
				outFile.write(reinterpret_cast<const char*>(&Desc.ModelNum), sizeof(_uint));
				outFile.write(reinterpret_cast<const char*>(&Desc.CullRadiuse), sizeof(_float));

				outFile.write(reinterpret_cast<const char*>(&NodeSize), sizeof(_uint));
				for (_uint i=0; i< NodeSize; i++)
				{
					outFile.write(reinterpret_cast<const char*>(&SubDesc->Node[i]), sizeof(Vector3));
				}
			}

			outFile.close();

			filePath = "";
		}
	EndButton_Save_Node:

		ImGui::SameLine();
		if (ImGui::Button("Load_Node", buttonSize)) {

			if (filePath.length() < 55) {
				MSG_BOX(TEXT("Chocie FilePath"));
				goto EndButton_Load_Node;
			}

			ifstream inFile(filePath, ios::binary);

			if (!inFile.is_open()) {
				MSG_BOX(TEXT("파일 불러오기 실패"));
				goto EndButton_Load_Node;
			}

			while (inFile.peek() != EOF) {
				CMapObject_Default::MAPOBJECT_DESC Desc{};
				CMapObject_Default::SUB_DESC SubDesc{};

				inFile.read(reinterpret_cast<char*>(&Desc.WorldMatrix), sizeof(_matrix));
				inFile.read(reinterpret_cast<char*>(&Desc.ModelNum), sizeof(_uint));
				inFile.read(reinterpret_cast<char*>(&Desc.CullRadiuse), sizeof(_float));
				Desc.LayerTag = m_StringLayerName;

				CGameObject* GameObj{};
				if (FAILED(m_pGameInstance->Add_GameObject_Out(TEXT("Prototype_GameObject_MapObject_Default"), LEVEL_GAMEPLAY, m_StringLayerName.c_str(), GameObj, &Desc)))
					return E_FAIL;

				_uint VecSize{};
				inFile.read(reinterpret_cast<char*>(&VecSize), sizeof(_uint));

				Vector3 VecData{};
				for (_uint i = 0; i < VecSize; i++) 
				{
					inFile.read(reinterpret_cast<char*>(&VecData), sizeof(Vector3));
					static_cast<CMapObject_Default*>(GameObj)->Get_SubDesc()->Node.push_back(VecData);
				}
			}

			inFile.close();

			filePath = "";
		}
	EndButton_Load_Node:

		ImGui::End();
	}

	return S_OK;
}

HRESULT CLevel_GamePlay::GameObject_Swap_Layer_Object()
{
	static bool bSwap_Layer_Object;
	ImGui::Checkbox("Is_Swap_Layer_Object", &bSwap_Layer_Object);

	if (bSwap_Layer_Object)
	{
		ImGui::Begin("Swap_Layer_Object");

		if (m_pGameObj != nullptr) {

			static char FindLayertag[256] = {};
			ImGui::InputText("FindLayertag##find", FindLayertag, IM_ARRAYSIZE(FindLayertag));

			ImVec2 buttonSize(100, 30);
			if (ImGui::Button("Swap##1", buttonSize)) {
				m_pGameInstance->Swap_Layer(LEVEL_GAMEPLAY, m_StringLayerName, char_to_wstring(FindLayertag), m_iSelectGameObj);
			}

			if (m_StringLayerName != L"\0") {
				_uint SetDepthNum = 0;
				for (auto& iter : *(m_pGameInstance->Get_ObjectList(LEVEL_GAMEPLAY, m_StringLayerName)))
				{
					iter->Set_DepthNum(SetDepthNum);
					SetDepthNum++;
				}
			}

		}
	
		ImGui::End();
	}

	return S_OK;
}

HRESULT CLevel_GamePlay::GameObject_Swap_Layer_Model()
{
	static bool bSwap_Layer_Model;
	ImGui::Checkbox("Is_Swap_Layer_Model", &bSwap_Layer_Model);

	if (bSwap_Layer_Model)
	{
		ImGui::Begin("Swap_Layer_Model");

		if (m_pLayer != nullptr) {

			static char FindLayertag[256] = {};
			ImGui::InputText("FindLayertag##find2", FindLayertag, IM_ARRAYSIZE(FindLayertag));
			ImGui::InputInt("FindModelNum", &FindModelNum);

			ImVec2 buttonSize(100, 30);
			if (ImGui::Button("Swap##2", buttonSize)) {
				list<CGameObject*>* SwapGameObjlist =  m_pGameInstance->Get_ObjectList(LEVEL_GAMEPLAY, m_StringLayerName);
				list<CGameObject*> SelectGameObjlist{};

				for (auto iter : *SwapGameObjlist) 
				{
					if (static_cast<CMapObject_Default*>(iter)->Get_UseModel() == FindModelNum) 
					{
						SelectGameObjlist.push_back(iter);
					}
				}

				m_pGameInstance->Swap_Layer_list(LEVEL_GAMEPLAY, m_StringLayerName, char_to_wstring(FindLayertag), SelectGameObjlist);

				if (m_StringLayerName != L"\0") {
					_uint SetDepthNum = 0;
					for (auto& iter : *(m_pGameInstance->Get_ObjectList(LEVEL_GAMEPLAY, m_StringLayerName)))
					{
						iter->Set_DepthNum(SetDepthNum);
						SetDepthNum++;
					}
				}
			}

		}

		ImGui::End();
	}

	return S_OK;
}

HRESULT CLevel_GamePlay::GameObject_Clear()
{
	ImGui::SameLine();

	ImVec2 buttonSize(100, 30);
	if (ImGui::Button("Clear##5", buttonSize)) {
		if (m_StringLayerName != L"\0") {
			
			_bool ClearObjectlist = CONFIRM_BOX(TEXT("Real Clear?"));

			if (ClearObjectlist) 
			{
				for (auto& iter : *(m_pGameInstance->Get_ObjectList(LEVEL_GAMEPLAY, m_StringLayerName)))
				{
					iter->Set_Dead(true);
				}

				m_pGameObj = nullptr;
				m_pTransformCom = nullptr;
			}
		}
	}
	
	return S_OK;
}

#pragma endregion

#pragma region Cell 작동 코드

HRESULT CLevel_GamePlay::Cell_Imgui(_float fTimeDelta)
{
	if (ImGui::CollapsingHeader("Cell , Navi"))
	{
		if (m_pTerrain != nullptr)
		{
			m_pNavigationCom_Terrain = m_pTerrain->Get_NavigationCom();
		
			if (m_pNavigationCom_Terrain == nullptr)
				return E_FAIL;

			// 값을 입력받고 셀 추가
			if (FAILED(Cell_Add(fTimeDelta)))
				return E_FAIL;

			ImGui::Begin("Cell Information");
			{
				if (ImGui::CollapsingHeader("Cell List"))
				{
					// 셀 리스트 박스
					Cell_ListBox(fTimeDelta);

					Cell_Save_Load(fTimeDelta);

					Cell_Data(fTimeDelta);
				}
			}
			ImGui::End();
		}
	}

	return S_OK;
}

HRESULT CLevel_GamePlay::Cell_Add(_float fTimeDelta)
{
	static bool bCell_Render{true};
	ImGui::Checkbox("Cell Render", &bCell_Render);
	if (m_pTerrain != nullptr) {
		m_pTerrain->Set_NaviRender(bCell_Render);
	}

	ImGui::SeparatorText("Cell_Add");

	static bool bCell_Picking{};
	ImGui::Checkbox("Cell Picking", &bCell_Picking);

	static ImGuiSliderFlags flags = ImGuiSliderFlags_None;

	ImGui::SameLine();
	ImGui::Text(" ", PickA, PickB, PickC, WhatPick);

#pragma region 점들 정리
	ImGui::PushItemWidth(45);
	ImGui::InputFloat("##PickA_x", &PickA.x);
	ImGui::PopItemWidth();
	ImGui::SameLine();

	ImGui::PushItemWidth(45);
	ImGui::InputFloat("##PickA_y", &PickA.y);
	ImGui::PopItemWidth();
	ImGui::SameLine();

	ImGui::PushItemWidth(45);
	ImGui::InputFloat("##PickA_z", &PickA.z);
	ImGui::PopItemWidth();
	ImGui::SameLine();
	ImGui::Text("PointA");
	//-----------------------------------------------
	ImGui::PushItemWidth(45);
	ImGui::InputFloat("##PickB_x", &PickB.x);
	ImGui::PopItemWidth();
	ImGui::SameLine();

	ImGui::PushItemWidth(45);
	ImGui::InputFloat("##PickB_y", &PickB.y);
	ImGui::PopItemWidth();
	ImGui::SameLine();

	ImGui::PushItemWidth(45);
	ImGui::InputFloat("##PickB_z", &PickB.z);
	ImGui::PopItemWidth();
	ImGui::SameLine();
	ImGui::Text("PointB");
	//-----------------------------------------------
	ImGui::PushItemWidth(45);
	ImGui::InputFloat("##PickC_x", &PickC.x);
	ImGui::PopItemWidth();
	ImGui::SameLine();

	ImGui::PushItemWidth(45);
	ImGui::InputFloat("##PickC_y", &PickC.y);
	ImGui::PopItemWidth();
	ImGui::SameLine();

	ImGui::PushItemWidth(45);
	ImGui::InputFloat("##PickC_z", &PickC.z);
	ImGui::PopItemWidth();
	ImGui::SameLine();
	ImGui::Text("PointC");
#pragma endregion

	ImGui::InputInt("Where_Pick", &WhatPick);

	if ((ImGui::Button("   Cell Add   ") || m_pGameInstance->Get_DIKeyState_Once(DIK_RETURN)) && bCell_Picking) {
		m_pNavigationCom_Terrain->Add_Cell(PickA, PickB, PickC);
	}

	if (bCell_Picking && m_pGameInstance->Get_DIKeyState(DIK_Q) &&
		m_pGameInstance->Get_DIMouseState_Once_Up(DIMK_LBUTTON))
	{
		_float3 PickPos{};
		if (m_pGameInstance->Picking(&PickPos)) 
		{
			switch (WhatPick)
			{
			case 0:
				PickA = Cell_Point_Correction(PickPos);
				break;
			case 1:
				PickB = Cell_Point_Correction(PickPos);
				break;
			case 2:
				PickC = Cell_Point_Correction(PickPos);
				break;
			default:
				break;
			}

			if (WhatPick >= 2)
				WhatPick = 0;
			else
				WhatPick++;
		}
	}

	/*if (bCell_Picking && m_pGameInstance->Get_DIKeyState(DIK_E) &&
		m_pGameInstance->Get_DIMouseState_Once_Up(DIMK_LBUTTON)) 
	{
		_float3 PickPos{};
		if (m_pGameInstance->Picking(&PickPos)) 
		{
			_uint iternum = 0;
			for (auto& iterCell : m_pNavigationCom_Terrain->Get_vecCell())
			{
				_int Dumy{};
				if (iterCell->isIn(XMLoadFloat3(&PickPos), &Dumy))
				{
					m_pNavigationCom_Terrain->Delete_Cell(iternum);
				}

				iternum++;
			}
		}
	}*/


	ImGui::Spacing();
	ImGui::SeparatorText("Terrain Cell");
	ImGui::Spacing();
	ImVec2 buttonSize(200, 50);
	if (ImGui::Button("Setting Terrain Cell Don't Use", buttonSize)) {
		// 터레인이 크면 컴퓨터의 램이 감당을 못한다
		//return S_OK;
		// 기존의 셀을 지우고
		m_pNavigationCom_Terrain->Clear_Cell();

		// 터레인의 정보를 받아와서 셀을 생성한다
		_uint NumVerticesX = m_pTerrain->Get_VIBuffer()->Get_VerticesX() + 1;
		_uint NumVerticesZ = m_pTerrain->Get_VIBuffer()->Get_VerticesZ() + 1;

		_uint		iNumIndices = { 0 };
		_uint		iIndexMax = NumVerticesZ * NumVerticesX;
		_uint		iValue = 10;

		for (_uint i = 0; i < NumVerticesZ - 1; i += iValue) {
			for (_uint j = 0; j < NumVerticesX - 1; j += iValue) {
				_uint			iIndex = i * NumVerticesX + j;

				_uint			iIndices[] = {
				iIndex + (NumVerticesX * iValue),
				iIndex + (NumVerticesX * iValue) + iValue,
				iIndex + iValue,
				iIndex
				};


				// 범위를 벗어나게 되는경우 예외처리
				_bool	bOverIndex = false;

				for (auto temp : iIndices) {
					if (temp > iIndexMax) {
						bOverIndex = true;
					}
				}
				// x축으로 예상한 범위를 벗어나도 예외처리 ( 의도하지 않는 값이 들어가게 된다 )
				if (iIndices[1] > (NumVerticesX * (i + iValue + 1) - 1) ||
					iIndices[2] > (NumVerticesX * (i + iValue) - 1)) {
					bOverIndex = true;
				}

				if(bOverIndex)
					continue;

				_float3 TerrainPointA = m_pTerrain->Get_VIBuffer()->Get_VertexPosition()[iIndices[0]];
				_float3 TerrainPointB = m_pTerrain->Get_VIBuffer()->Get_VertexPosition()[iIndices[1]];
				_float3 TerrainPointC = m_pTerrain->Get_VIBuffer()->Get_VertexPosition()[iIndices[2]];
				m_pNavigationCom_Terrain->Add_Cell_NoneCheck(TerrainPointA, TerrainPointB, TerrainPointC);

				TerrainPointA = m_pTerrain->Get_VIBuffer()->Get_VertexPosition()[iIndices[0]];
				TerrainPointB = m_pTerrain->Get_VIBuffer()->Get_VertexPosition()[iIndices[2]];
				TerrainPointC = m_pTerrain->Get_VIBuffer()->Get_VertexPosition()[iIndices[3]];
				m_pNavigationCom_Terrain->Add_Cell_NoneCheck(TerrainPointA, TerrainPointB, TerrainPointC);
			}
		}
	}

	ImGui::Spacing();
	ImGui::SeparatorText("Neighbor Setting");
	ImGui::Spacing();
	ImVec2 buttonSize2(200, 50);
	if (ImGui::Button("Setting Neighbor", buttonSize2)) {
		m_pNavigationCom_Terrain->SetUp_Neighbors();
	}

	return S_OK;
}

_float3 CLevel_GamePlay::Cell_Point_Correction(_float3 Point)
{
	Vector3 PointPos = Point;

	// 보정 코드
	_float Homing = 0.5f; // 보정 범위
	for (auto& iterCell : m_pNavigationCom_Terrain->Get_vecCell()) 
	{
		// 셀 내부 판별 코드 ( 피킹지점에 이미 셀이 있는지 )
		/*_int Dumy{};
		if (iterCell->isIn(PointPos, &Dumy))
			return _float3(-10.f, -10.f, -10.f);*/

		Vector3 CellPointA = iterCell->Get_Point(CCell::POINT_A);
		Vector3 CellPointB = iterCell->Get_Point(CCell::POINT_B);
		Vector3 CellPointC = iterCell->Get_Point(CCell::POINT_C);

		if (Vector3::Distance(PointPos, CellPointA) < Homing)
			return CellPointA;
		else if (Vector3::Distance(PointPos, CellPointB) < Homing)
			return CellPointB;
		else if (Vector3::Distance(PointPos, CellPointC) < Homing)
			return CellPointC;
	}

	return Point;
}

HRESULT CLevel_GamePlay::Cell_ListBox(_float fTimeDelta)
{
	ImGui::SeparatorText("Cell List");
	ImGui::PushItemWidth(200); // 크기조정

	// 셀 받아오기 
	vector<class CCell*> vecCells = m_pNavigationCom_Terrain->Get_vecCell();

	static bool bFind_Cell;
	ImGui::Checkbox("Cell Find Picking", &bFind_Cell);

	_float3 FindPos{};

	if (bFind_Cell) {
		if (m_pGameInstance->Get_DIMouseState_Once(DIMK_LBUTTON) && m_pGameInstance->Get_DIKeyState(DIK_E)) {
			if (m_pGameInstance->Picking(&FindPos))
			{
				if (m_pCell != nullptr) {
					m_pCell->Set_PickCell(false);
				}

				_int Dumy{};
				for (auto& Cell : vecCells) {
					if (Cell->isIn(XMLoadFloat3(&FindPos), &Dumy)) {
						m_pCell = Cell;
						m_pCell->Set_PickCell(true);
						m_iSelectCell = m_pCell->Get_Index();
						break;
					}
				}
			}
		}
	}

	if (ImGui::BeginListBox("##Cell_List"))
	{
		/*	렉걸려서 주석
		for (_uint n = 0; n < m_CellNum; n++)
		{
			bool is_selected = (m_iSelectCell == n);
			if (m_vecString_Cell.size() > 0) {

				string CellName = m_vecString_Cell[n];

				if (ImGui::Selectable(CellName.c_str(), is_selected))
				{
					if (m_pCell != nullptr) {
						m_pCell->Set_PickCell(false);
					}

					// 현제 선택한 리스트 박스의 인덱스
					m_iSelectCell = n;
					// 현제 선택한 셀을 대입한다
					m_pCell = vecCells[n];
					m_pCell->Set_PickCell(true);
				}

				if (is_selected)
					ImGui::SetItemDefaultFocus();
				// 반복문으로 리스트박스의 선택된 객체 찾기
			}
		}*/

		ImGui::EndListBox();
	}
	ImGui::PopItemWidth();
	ImGui::SameLine();
	if (ImGui::Button("DeleteCell")) {
		if (m_pCell != nullptr) {
			_int CellIndex = m_pCell->Get_Index();

			if (m_pNavigationCom_Terrain != nullptr) 
			{
				m_pNavigationCom_Terrain->Delete_Cell(CellIndex);
				Cell_vecStringSet();

				vector<class CCell*> vecCellsNow = m_pNavigationCom_Terrain->Get_vecCell();
				if (vecCellsNow.size() > 0) {
					m_pCell = vecCellsNow[vecCellsNow.size() - 1];
					m_pCell->Set_PickCell(true);
					m_iSelectCell = m_pCell->Get_Index();
				}
				else {
					m_pCell = nullptr;
				}
			}
		}
	}

	if (ImGui::Button("ListUpdate")) {
		Cell_vecStringSet();
	}
	ImGui::Spacing();

	if (ImGui::Button("  Up  ")) {
		if (m_pCell != nullptr) {
			m_pCell->Set_PickCell(false);
		}

		_uint Index = m_pCell->Get_Index();

		if (Index < vecCells.size()) 
		{
			m_pCell = vecCells[Index + 1];
			m_pCell->Set_PickCell(true);
			m_iSelectCell = m_pCell->Get_Index();
		}
	}
	ImGui::SameLine();
	if (ImGui::Button(" Down ")) {
		if (m_pCell != nullptr) {
			m_pCell->Set_PickCell(false);
		}

		_uint Index = m_pCell->Get_Index();

		if (Index > 0) 
		{
			m_pCell = vecCells[Index - 1];
			m_pCell->Set_PickCell(true);
			m_iSelectCell = m_pCell->Get_Index();
		}
	}

	return S_OK;
}

HRESULT CLevel_GamePlay::Cell_Data(_float fTimeDelta)
{
	ImGui::SeparatorText("Cell Data");

	if (m_pCell == nullptr)
		return S_OK;

	ImGui::Text("IndexNum = %d", m_pCell->Get_Index());
	ImGui::Spacing();

	Vector3 Pos = m_pCell->Get_Point(CCell::POINT_A);
	ImGui::Text("PointA.x = %.2f", Pos.x);
	ImGui::Text("PointA.y = %.2f", Pos.y);
	ImGui::Text("PointA.z = %.2f", Pos.z);
	ImGui::Spacing();

	Pos = m_pCell->Get_Point(CCell::POINT_B);
	ImGui::Text("PointB.x = %.2f", Pos.x);
	ImGui::Text("PointB.y = %.2f", Pos.y);
	ImGui::Text("PointB.z = %.2f", Pos.z);
	ImGui::Spacing();

	Pos = m_pCell->Get_Point(CCell::POINT_C);
	ImGui::Text("PointC.x = %.2f", Pos.x);
	ImGui::Text("PointC.y = %.2f", Pos.y);
	ImGui::Text("PointC.z = %.2f", Pos.z);
	ImGui::Spacing();

	ImGui::Text("NeighborAB = %d", m_pCell->Get_Neighbor(CCell::LINE_AB));
	ImGui::Text("NeighborBC = %d", m_pCell->Get_Neighbor(CCell::LINE_BC));
	ImGui::Text("NeighborCA = %d", m_pCell->Get_Neighbor(CCell::LINE_CA));
	ImGui::Spacing();

	_bool IsRide = m_pCell->Get_Ride();
	ImGui::Checkbox("Is Ride", &IsRide);
	m_pCell->Set_Ride(IsRide);

	return S_OK;
}

HRESULT CLevel_GamePlay::Cell_vecStringSet()
{
	// 셀 받아오기 
	vector<class CCell*> vecCells = m_pNavigationCom_Terrain->Get_vecCell();

	m_CellNum = _uint(vecCells.size());
	// 기존에 사용중이던 정보 클리어
	m_vecString_Cell.clear();
	// 갱신된 새로운 정보 대입
	for (_uint i = 0; i < m_CellNum; i++)
	{
		_wstring Celllistboxname = L"Cell_" + to_wstring(i);
		m_vecString_Cell.push_back(wstring_to_string(Celllistboxname));
	}

	return S_OK;
}

HRESULT CLevel_GamePlay::Cell_Save_Load(_float fTimeDelta)
{
	ImGui::SeparatorText("Cell Save Load");
	ImVec2 buttonSize(100, 30);
	if (ImGui::Button("Save_Cell", buttonSize)) {

		if (m_pNavigationCom_Terrain == nullptr) {
			MSG_BOX(TEXT("Chocie Cell"));
			return E_FAIL;
		}

		if (filePath.length() < 55) {
			MSG_BOX(TEXT("Chocie FilePath"));
			return E_FAIL;
		}

		ofstream outFile(filePath, ios::binary);

		if (!outFile.is_open()) {
			MSG_BOX(TEXT("파일 저장 실패"));
			return E_FAIL;
		}

		vector<class CCell*> vecCells = m_pNavigationCom_Terrain->Get_vecCell();

		m_pNavigationCom_Terrain->SetUp_Neighbors();

		for (const auto& Cells : vecCells) {

			CNavigation::CELL_DESC Desc{};

			XMStoreFloat3(&Desc.PointA, Cells->Get_Point(CCell::POINT_A));
			XMStoreFloat3(&Desc.PointB, Cells->Get_Point(CCell::POINT_B));
			XMStoreFloat3(&Desc.PointC, Cells->Get_Point(CCell::POINT_C));

			Desc.NeighborIndex_AB = Cells->Get_Neighbor(CCell::LINE_AB);
			Desc.NeighborIndex_BC = Cells->Get_Neighbor(CCell::LINE_BC);
			Desc.NeighborIndex_CA = Cells->Get_Neighbor(CCell::LINE_CA);

			Desc.IsRide = Cells->Get_Ride();

			outFile.write(reinterpret_cast<const char*>(&Desc.PointA), sizeof(_float3));
			outFile.write(reinterpret_cast<const char*>(&Desc.PointB), sizeof(_float3));
			outFile.write(reinterpret_cast<const char*>(&Desc.PointC), sizeof(_float3));

			outFile.write(reinterpret_cast<const char*>(&Desc.NeighborIndex_AB), sizeof(_int));
			outFile.write(reinterpret_cast<const char*>(&Desc.NeighborIndex_BC), sizeof(_int));
			outFile.write(reinterpret_cast<const char*>(&Desc.NeighborIndex_CA), sizeof(_int));

			outFile.write(reinterpret_cast<const char*>(&Desc.IsRide), sizeof(_bool));
		}

		outFile.close();

		filePath = "";
	}
	ImGui::SameLine();
	if (ImGui::Button("Load_Cell", buttonSize)) {

		if (m_pNavigationCom_Terrain == nullptr) {
			MSG_BOX(TEXT("Chocie Cell"));
			return E_FAIL;
		}

		if (filePath.length() < 55) {
			MSG_BOX(TEXT("Chocie FilePath"));
			return E_FAIL;
		}

		ifstream inFile(filePath, ios::binary);

		if (!inFile.is_open()) {
			MSG_BOX(TEXT("파일 불러오기 실패"));
			return E_FAIL;
		}

		m_pNavigationCom_Terrain->Clear_Cell();

		while (inFile.peek() != EOF) {
			CNavigation::CELL_DESC Desc{};

			inFile.read(reinterpret_cast<char*>(&Desc.PointA), sizeof(_float3));
			inFile.read(reinterpret_cast<char*>(&Desc.PointB), sizeof(_float3));
			inFile.read(reinterpret_cast<char*>(&Desc.PointC), sizeof(_float3));

			inFile.read(reinterpret_cast<char*>(&Desc.NeighborIndex_AB), sizeof(_int));
			inFile.read(reinterpret_cast<char*>(&Desc.NeighborIndex_BC), sizeof(_int));
			inFile.read(reinterpret_cast<char*>(&Desc.NeighborIndex_CA), sizeof(_int));

			inFile.read(reinterpret_cast<char*>(&Desc.IsRide), sizeof(_bool));

			m_pNavigationCom_Terrain->Add_Bin_Cell(Desc);
		}

		inFile.close();

		filePath = "";
		Cell_vecStringSet();
	}

	return S_OK;
}


#pragma endregion

#pragma region 광원 코드

HRESULT CLevel_GamePlay::Light_Imgui(_float fTimeDelta)
{
	if (ImGui::CollapsingHeader("Light"))
	{
		// 광원 생성
		ImGui::Spacing();
		Light_Create_Light(fTimeDelta);

		if (ImGui::CollapsingHeader("Light List")) 
		{
			if (FAILED(Light_ListBox(fTimeDelta)))
				return E_FAIL;

			if (FAILED(Light_Save_Load(fTimeDelta)))
				return E_FAIL;
		}
		
		if (m_pLight != nullptr)
		{
			ImGui::Begin("Information Light");
			{
				if (FAILED(Light_DescSet(fTimeDelta)))
					return E_FAIL;
			}
			ImGui::End();
		}
	}

	return S_OK;
}

HRESULT CLevel_GamePlay::Light_Create_Light(_float fTimeDelta)
{
	ImGui::SeparatorText("Light_Add");

	ImGui::PushItemWidth(150); // 크기조정

	if (ImGui::Button("  Create Light  ")) {
		LIGHT_DESC			LightDesc{};

		ZeroMemory(&LightDesc, sizeof LightDesc);
		LightDesc.eType = LIGHT_DESC::TYPE_POINT;
		LightDesc.vPosition = _float4(0.f, 1.f, 0.f, 1.f);
		LightDesc.fRange = 10.f;
		LightDesc.vDiffuse = _float4(1.f, 1.f, 1.f, 1.f);
		LightDesc.vAmbient = _float4(1.f, 1.f, 1.f, 0.f);
		LightDesc.vSpecular = _float4(1.f, 1.f, 1.f, 1.f);

		if (FAILED(m_pGameInstance->Add_Light(LightDesc)))
			return E_FAIL;
	}

	ImGui::PopItemWidth();

	static bool bLight_Picking;
	ImGui::Spacing();
	ImGui::Checkbox("Light Picking", &bLight_Picking);

	if (bLight_Picking)
	{
		if (m_pGameInstance->Get_DIMouseState_Once(DIMK_LBUTTON) && m_pGameInstance->Get_DIKeyState(DIK_Q)) {
			_float3 PickPos{};
			if (m_pGameInstance->Picking(&PickPos)) {
				LIGHT_DESC			LightDesc{};

				ZeroMemory(&LightDesc, sizeof LightDesc);
				LightDesc.eType = LIGHT_DESC::TYPE_POINT;
				LightDesc.vPosition = _float4(PickPos.x, PickPos.y + 1.f, PickPos.z, 1.f);
				LightDesc.fRange = 10.f;
				LightDesc.vDiffuse = _float4(1.f, 1.f, 1.f, 1.f);
				LightDesc.vAmbient = _float4(1.f, 1.f, 1.f, 0.f);
				LightDesc.vSpecular = _float4(1.f, 1.f, 1.f, 1.f);

				if (FAILED(m_pGameInstance->Add_Light(LightDesc)))
					return E_FAIL;
			}
		}
	}

	return S_OK;
}

HRESULT CLevel_GamePlay::Light_ListBox(_float fTimeDelta)
{
	// light list Box 에 있는 vecString 정리
	m_iLightListSize = _uint(m_pGameInstance->Get_LightList().size());
	m_vecString_Light.clear();
	for (_uint i = 0; i < m_iLightListSize; i++)
	{
		_wstring Lightlistboxname = L"Light_" + to_wstring(i);
		m_vecString_Light.push_back(wstring_to_string(Lightlistboxname));
	}

	ImGui::PushItemWidth(200); // 크기조정

	if (ImGui::BeginListBox("##Light_List"))
	{
		for (_uint n = 0; n < m_iLightListSize; n++)
		{
			bool is_selected = (m_iSelectLight == n);
			string MapName = m_vecString_Light[n];
			if (ImGui::Selectable(MapName.c_str(), is_selected))
			{
				// 현제 선택한 리스트 박스의 인덱스
				m_iSelectLight = n;
				// 현제 선택한 광원를 대입한다
				m_pLight = m_pGameInstance->Get_Light(m_iSelectLight);
			}

			if (is_selected)
				ImGui::SetItemDefaultFocus();
			// 반복문으로 리스트박스의 선택된 객체 찾기
		}

		ImGui::EndListBox();
	}
	ImGui::PopItemWidth();

	ImGui::SameLine();
	if (ImGui::Button("DeleteLight")) {
		if (m_pLight != nullptr) {
			m_pGameInstance->Delete_Light(m_iSelectLight);

			m_iSelectLight--;
			m_pLight = m_pGameInstance->Get_Light(m_iSelectLight);
		}
	}
	return S_OK;
}

HRESULT CLevel_GamePlay::Light_Save_Load(_float fTimeDelta)
{
	ImVec2 buttonSize(100, 30);
	if (ImGui::Button("Save_Light", buttonSize)) {

		if (filePath.length() < 55) {
			MSG_BOX(TEXT("Chocie FilePath"));
			return E_FAIL;
		}
		
		ofstream outFile(filePath, ios::binary);

		if (!outFile.is_open()) {
			MSG_BOX(TEXT("파일 저장 실패"));
			return E_FAIL;
		}

		for (const auto& LightList : m_pGameInstance->Get_LightList()) {

			LIGHT_DESC Desc = *LightList->Get_LightDesc();

			outFile.write(reinterpret_cast<const char*>(&Desc.eType), sizeof(_uint));
			outFile.write(reinterpret_cast<const char*>(&Desc.vDirection), sizeof(_float4));
			outFile.write(reinterpret_cast<const char*>(&Desc.vPosition), sizeof(_float4));
			outFile.write(reinterpret_cast<const char*>(&Desc.fRange), sizeof(_float));
			outFile.write(reinterpret_cast<const char*>(&Desc.vDiffuse), sizeof(_float4));
			outFile.write(reinterpret_cast<const char*>(&Desc.vAmbient), sizeof(_float4));
			outFile.write(reinterpret_cast<const char*>(&Desc.vSpecular), sizeof(_float4));
		}

		outFile.close();

		filePath = "";
	}

	ImGui::SameLine();

	if (ImGui::Button("Load_Light", buttonSize)) {

		if (filePath.length() < 55) {
			MSG_BOX(TEXT("Chocie FilePath"));
			return E_FAIL;
		}

		ifstream inFile(filePath, ios::binary);

		if (!inFile.is_open()) {
			MSG_BOX(TEXT("파일 불러오기 실패"));
			return E_FAIL;
		}

		m_pGameInstance->Clear_Light();

		while (inFile.peek() != EOF) {
			LIGHT_DESC Desc{};

			inFile.read(reinterpret_cast<char*>(&Desc.eType), sizeof(_uint));
			inFile.read(reinterpret_cast<char*>(&Desc.vDirection), sizeof(_float4));
			inFile.read(reinterpret_cast<char*>(&Desc.vPosition), sizeof(_float4));
			inFile.read(reinterpret_cast<char*>(&Desc.fRange), sizeof(_float));
			inFile.read(reinterpret_cast<char*>(&Desc.vDiffuse), sizeof(_float4));
			inFile.read(reinterpret_cast<char*>(&Desc.vAmbient), sizeof(_float4));
			inFile.read(reinterpret_cast<char*>(&Desc.vSpecular), sizeof(_float4));

			if (FAILED(m_pGameInstance->Add_Light(Desc)))
				return E_FAIL;
		}

		inFile.close();

		filePath = "";
	}

	return S_OK;
}

HRESULT CLevel_GamePlay::Light_DescSet(_float fTimeDelta)
{	
	// 빛 정보 받아오기
	LIGHT_DESC* LightDesc = m_pLight->Get_LightDesc();

	// 라디오 버튼
	if (ImGui::RadioButton("Direction", LightDesc->eType == 0)) {
		LightDesc->eType = LIGHT_DESC::TYPE_DIRECTIONAL;
	}
	ImGui::SameLine(0.0f, 10.0f);
	if (ImGui::RadioButton("Point", LightDesc->eType == 1)) {
		LightDesc->eType = LIGHT_DESC::TYPE_POINT;
	}

	static ImGuiSliderFlags flags = ImGuiSliderFlags_None;

#pragma region 빛 방향
	ImGui::SeparatorText("Direction");
	ImGui::SameLine();
	ImGui::Text(" ", LightDesc->vDirection.x, LightDesc->vDirection.y, LightDesc->vDirection.z, LightDesc->vDirection.w);
	
	ImGui::PushItemWidth(200);
	ImGui::DragFloat("DirX ", &LightDesc->vDirection.x, 0.05f, -FLT_MAX, +FLT_MAX, "%.3f", flags);
	ImGui::PopItemWidth();
	ImGui::SameLine();
	ImGui::PushItemWidth(100);
	ImGui::InputFloat("##input DirX", &LightDesc->vDirection.x, 0.01f, 1.0f, "%.3f");
	ImGui::PopItemWidth();
	
	ImGui::PushItemWidth(200);
	ImGui::DragFloat("DirY ", &LightDesc->vDirection.y, 0.05f, -FLT_MAX, +FLT_MAX, "%.3f", flags);
	ImGui::PopItemWidth();
	ImGui::SameLine();
	ImGui::PushItemWidth(100);
	ImGui::InputFloat("##input DirY", &LightDesc->vDirection.y, 0.01f, 1.0f, "%.3f");
	ImGui::PopItemWidth();
	
	ImGui::PushItemWidth(200);
	ImGui::DragFloat("DirZ ", &LightDesc->vDirection.z, 0.05f, -FLT_MAX, +FLT_MAX, "%.3f", flags);
	ImGui::PopItemWidth();
	ImGui::SameLine();
	ImGui::PushItemWidth(100);
	ImGui::InputFloat("##input DirZ", &LightDesc->vDirection.z, 0.01f, 1.0f, "%.3f");
	ImGui::PopItemWidth();

	ImGui::PushItemWidth(200);
	ImGui::DragFloat("DirW ", &LightDesc->vDirection.w, 0.05f, -FLT_MAX, +FLT_MAX, "%.3f", flags);
	ImGui::PopItemWidth();
	ImGui::SameLine();
	ImGui::PushItemWidth(100);
	ImGui::InputFloat("##input DirW", &LightDesc->vDirection.w, 0.01f, 1.0f, "%.3f");
	ImGui::PopItemWidth();

#pragma endregion

#pragma region 빛 위치
	ImGui::SeparatorText("Position");
	ImGui::SameLine();
	ImGui::Text(" ", LightDesc->vPosition.x, LightDesc->vPosition.y, LightDesc->vPosition.z);

	ImGui::PushItemWidth(200);
	ImGui::DragFloat("PosX ", &LightDesc->vPosition.x, 0.05f, -FLT_MAX, +FLT_MAX, "%.3f", flags);
	ImGui::PopItemWidth();
	ImGui::SameLine();
	ImGui::PushItemWidth(100);
	ImGui::InputFloat("##input PosX", &LightDesc->vPosition.x, 0.01f, 1.0f, "%.3f");
	ImGui::PopItemWidth();

	ImGui::PushItemWidth(200);
	ImGui::DragFloat("PosY ", &LightDesc->vPosition.y, 0.05f, -FLT_MAX, +FLT_MAX, "%.3f", flags);
	ImGui::PopItemWidth();
	ImGui::SameLine();
	ImGui::PushItemWidth(100);
	ImGui::InputFloat("##input PosY", &LightDesc->vPosition.y, 0.01f, 1.0f, "%.3f");
	ImGui::PopItemWidth();

	ImGui::PushItemWidth(200);
	ImGui::DragFloat("PosZ ", &LightDesc->vPosition.z, 0.05f, -FLT_MAX, +FLT_MAX, "%.3f", flags);
	ImGui::PopItemWidth();
	ImGui::SameLine();
	ImGui::PushItemWidth(100);
	ImGui::InputFloat("##input PosZ", &LightDesc->vPosition.z, 0.01f, 1.0f, "%.3f");
	ImGui::PopItemWidth();

#pragma endregion

#pragma region 빛 범위
	ImGui::SeparatorText("Range");
	ImGui::SameLine();
	ImGui::Text(" ", LightDesc->fRange);

	ImGui::PushItemWidth(200);
	ImGui::DragFloat("Range ", &LightDesc->fRange, 0.05f, 1.f, +FLT_MAX, "%.3f", flags);
	ImGui::PopItemWidth();
	ImGui::SameLine();
	ImGui::PushItemWidth(100);
	ImGui::InputFloat("##input Range", &LightDesc->fRange, 0.01f, 1.0f, "%.3f");
	ImGui::PopItemWidth();

#pragma endregion

#pragma region 빛 색상
	ImGui::SeparatorText("Diffuse");
	ImGui::SameLine();
	ImGui::Text(" ", LightDesc->vDiffuse.x, LightDesc->vDiffuse.y, LightDesc->vDiffuse.z);

	ImGui::PushItemWidth(200);
	ImGui::SliderFloat("Lignt_R ", &LightDesc->vDiffuse.x, 0.f, 1.f);
	ImGui::SameLine();
	ImGui::PopItemWidth();
	ImGui::PushItemWidth(100);
	ImGui::InputFloat("##input Lignt_R", &LightDesc->vDiffuse.x, 0.01f, 1.0f, "%.3f");
	ImGui::PopItemWidth();

	ImGui::PushItemWidth(200);
	ImGui::SliderFloat("Lignt_G ", &LightDesc->vDiffuse.y, 0.f, 1.f);
	ImGui::SameLine();
	ImGui::PopItemWidth();
	ImGui::PushItemWidth(100);
	ImGui::InputFloat("##input Lignt_G", &LightDesc->vDiffuse.y, 0.01f, 1.0f, "%.3f");
	ImGui::PopItemWidth();

	ImGui::PushItemWidth(200);
	ImGui::SliderFloat("Lignt_B ", &LightDesc->vDiffuse.z, 0.f, 1.f);
	ImGui::SameLine();
	ImGui::PopItemWidth();
	ImGui::PushItemWidth(100);
	ImGui::InputFloat("##input Lignt_B", &LightDesc->vDiffuse.z, 0.01f, 1.0f, "%.3f");
	ImGui::PopItemWidth();

#pragma endregion

#pragma region 엠비언트
	ImGui::SeparatorText("Ambient");
	ImGui::SameLine();
	ImGui::Text(" ", LightDesc->vAmbient.x, LightDesc->vAmbient.y, LightDesc->vAmbient.z);

	ImGui::PushItemWidth(200);
	ImGui::SliderFloat("Ambient_R ", &LightDesc->vAmbient.x, 0.f, 1.f);
	ImGui::SameLine();
	ImGui::PopItemWidth();
	ImGui::PushItemWidth(100);
	ImGui::InputFloat("##input Ambient_R", &LightDesc->vAmbient.x, 0.01f, 1.0f, "%.3f");
	ImGui::PopItemWidth();

	ImGui::PushItemWidth(200);
	ImGui::SliderFloat("Ambient_G ", &LightDesc->vAmbient.y, 0.f, 1.f);
	ImGui::SameLine();
	ImGui::PopItemWidth();
	ImGui::PushItemWidth(100);
	ImGui::InputFloat("##input Ambient_G", &LightDesc->vAmbient.y, 0.01f, 1.0f, "%.3f");
	ImGui::PopItemWidth();

	ImGui::PushItemWidth(200);
	ImGui::SliderFloat("Ambient_B ", &LightDesc->vAmbient.z, 0.f, 1.f);
	ImGui::SameLine();
	ImGui::PopItemWidth();
	ImGui::PushItemWidth(100);
	ImGui::InputFloat("##input Ambient_B", &LightDesc->vAmbient.z, 0.01f, 1.0f, "%.3f");
	ImGui::PopItemWidth();

#pragma endregion

#pragma region 스펙큘러
	ImGui::SeparatorText("Specular");
	ImGui::SameLine();
	ImGui::Text(" ", LightDesc->vSpecular.x, LightDesc->vSpecular.y, LightDesc->vSpecular.z);

	ImGui::PushItemWidth(200);
	ImGui::DragFloat("Specular_R ", &LightDesc->vSpecular.x, 0.05f, -FLT_MAX, +FLT_MAX, "%.3f", flags);
	ImGui::PopItemWidth();
	ImGui::SameLine();
	ImGui::PushItemWidth(100);
	ImGui::InputFloat("##input Specular_R", &LightDesc->vSpecular.x, 0.01f, 1.0f, "%.3f");
	ImGui::PopItemWidth();

	ImGui::PushItemWidth(200);
	ImGui::DragFloat("Specular_G ", &LightDesc->vSpecular.y, 0.05f, -FLT_MAX, +FLT_MAX, "%.3f", flags);
	ImGui::PopItemWidth();
	ImGui::SameLine();
	ImGui::PushItemWidth(100);
	ImGui::InputFloat("##input Specular_G", &LightDesc->vSpecular.y, 0.01f, 1.0f, "%.3f");
	ImGui::PopItemWidth();

	ImGui::PushItemWidth(200);
	ImGui::DragFloat("Specular_B ", &LightDesc->vSpecular.z, 0.05f, -FLT_MAX, +FLT_MAX, "%.3f", flags);
	ImGui::PopItemWidth();
	ImGui::SameLine();
	ImGui::PushItemWidth(100);
	ImGui::InputFloat("##input Specular_B", &LightDesc->vSpecular.z, 0.01f, 1.0f, "%.3f");
	ImGui::PopItemWidth();

#pragma endregion

	m_pLight->Set_LightDesc(*LightDesc);

	return S_OK;
}

#pragma endregion

#pragma region 컨테이너 초기화 코드

HRESULT CLevel_GamePlay::VectorClear()
{
	m_vecString_Mask.clear();
	m_vecString_Map_Layer.clear();

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
	/*if (FAILED(m_pGameInstance->Add_CloneObject_ToLayer(LEVEL_GAMEPLAY, TEXT("Layer_Player"), TEXT("Prototype_GameObject_Player"))))
		return E_FAIL;*/

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
