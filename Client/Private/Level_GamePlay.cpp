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

	// ������Ʈ ���� �׽�Ʈ �뵵
	/*if (FAILED(OBJ_TEST()))
		return E_FAIL;*/
	
	//m_pGameInstance->PlayBGM(L"SD_BGM_Sual_Boss.ogg", 1.f, true);

	// ImGui �ʱ�ȭ
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;

	// ImGui ��Ÿ�� ����
	ImGui::StyleColorsDark(); // ��ũ ���
	//ImGui::StyleColorsLight(); // �Ϲ� ���

	ImGui_ImplWin32_Init(g_hWnd);
	ImGui_ImplDX11_Init(m_pDevice, m_pContext);

	ImGuiStyle& style = ImGui::GetStyle();
	for (int i = 0; i < ImGuiCol_COUNT; i++)
	{
		ImVec4& col = style.Colors[i];
		col.w *= 0.8f;  // col.w�� ������ ����(����) ���� ��Ÿ���ϴ�.
	}

	// Gui ������ on/off ó�����۽� ������ �켱�۵��ؼ� ������ �߻��Ѵ�
	m_bGuiReady = false;

#pragma region imgui�ʿ� ������
	m_pTerrain = static_cast<CTerrain*>(m_pGameInstance->Get_Object(LEVEL_GAMEPLAY, TEXT("Layer_Terrain")));
	Safe_AddRef(m_pTerrain);
#pragma endregion

	return S_OK;
}

void CLevel_GamePlay::Update(_float fTimeDelta)
{
	// ImGui ������ ������Ʈ
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	ImGui::Begin("MainImgui");
	{
		Dialog_Imgui(fTimeDelta);
		Terrain_Imgui(fTimeDelta);
	}
	ImGui::End();

	VectorClear();

	if (m_pGameInstance->Get_DIKeyState_Once(DIK_T)) {
		
	}

}

#pragma region Imgui �Լ� ����

#pragma region ���ϰ�� ���� �Լ�
static string wstring_to_string(const wstring& wstr) {
	wstring_convert<codecvt_utf8<wchar_t>> converter;
	return converter.to_bytes(wstr);

	// wstring ���� �ۼ��� �̸��� string���� ��ȯ > ���� .ctr �� �ٿ��� 
	// char* ������ ����� �����ϴ� imgui ������ char* �� ����ϱ⿡ ���� �Լ�
}

static string WideCharToMultiByte(const wstring& wstr) {
	int size_needed = WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), NULL, 0, NULL, NULL);
	string strTo(size_needed, 0);
	WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), &strTo[0], size_needed, NULL, NULL);
	return strTo;
}

static wchar_t* stringToWchar(const string& str) {
	int size_needed = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), static_cast<int>(str.size()), NULL, 0);
	wchar_t* wcharStr = new wchar_t[size_needed + 1];
	MultiByteToWideChar(CP_UTF8, 0, str.c_str(), static_cast<int>(str.size()), wcharStr, size_needed);
	wcharStr[size_needed] = L'\0';

	return wcharStr; // ȣ���ڰ� delete[]�� ���� �ʿ�
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
	// ���� ��ȭ ���� �ʱ�ȭ
	ImGuiFileDialog::Instance()->OpenDialog(
		"OpenFileDialog",           // vKey
		"Select a File",           // vTitle
		".bmp",					   // vFilters
		IGFD::FileDialogConfig()   // vConfig (�⺻ ����)
	);

	// ��ȭ ���ڰ� ���� ��
	if (ImGuiFileDialog::Instance()->Display("OpenFileDialog"))
	{
		// ����ڰ� ������ ������ ���
		if (ImGuiFileDialog::Instance()->IsOk())
		{
			// ���õ� ���� ��� ��������
			filePath = ImGuiFileDialog::Instance()->GetFilePathName();
		}

		// ��ȭ ���� �ݱ�
		ImGuiFileDialog::Instance()->Close();
	}
}
#pragma endregion

#pragma region Terrain �۵� �ڵ�

HRESULT CLevel_GamePlay::Terrain_Imgui(_float fTimeDelta)
{
	if (ImGui::CollapsingHeader("Terrain"))
	{
		// x,y ���� �Է¹ް� ���� ����
		if (FAILED(Create_Terrain_Input(fTimeDelta)))
			return E_FAIL;

		if (ImGui::CollapsingHeader("Height")) 
		{
			// ������ ���̸� �����ϴ� �ڵ�
			if (FAILED(Terrain_HeightChange(fTimeDelta)))
				return E_FAIL;

			// ������ ���̸� ����, �ҷ����� �ڵ�
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
	ImGui::PushItemWidth(50); // ũ������
	static int TerrainX = 0;
	ImGui::InputInt("TerrainX", &TerrainX, 0, 0, 1);
	static int TerrainZ = 0;
	ImGui::InputInt("TerrainZ", &TerrainZ, 0, 0, 1);

	if (ImGui::Button("  Delete Terrain  ")) {
		_bool test = CONFIRM_BOX(TEXT("�����Ͻð����ϱ�?"));

		if (test) {
			m_pTerrain->Set_Dead(true);
			Safe_Release(m_pTerrain);
			m_pTerrain = nullptr;
		}
	}
	ImGui::SameLine();
	if (ImGui::Button("  Create Terrain  ")) {

		// ������ ���� ����
		m_pTerrain->Set_Dead(true);
		Safe_Release(m_pTerrain);
		m_pTerrain = nullptr;

		// �ű� ���� ���� ( ������ ������ 1���� ������´� )
		CGameObject*	pTerrain = {};
		CVIBuffer_Terrain::TERRAIN_BUFFER_DESC Desc{};
		Desc.TerrainXZ.x = _float(TerrainX);
		Desc.TerrainXZ.y = _float(TerrainZ);

		if (FAILED(m_pGameInstance->Add_GameObject_Out(TEXT("Prototype_GameObject_Terrain"), LEVEL_GAMEPLAY, TEXT("Layer_Terrain"), pTerrain, &Desc)))
			return E_FAIL;

		// ���� �������� ����
		m_pTerrain = static_cast<CTerrain*>(pTerrain);
		Safe_AddRef(m_pTerrain);
	}

	ImGui::PopItemWidth();

	return S_OK;
}

HRESULT CLevel_GamePlay::Terrain_HeightChange(_float fTimeDelta)
{
	ImGui::SeparatorText("Height_Change");

	ImGui::PushItemWidth(150); // ũ������
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
			// ��ŷ�ϰ�
			m_pGameInstance->Picking(&m_vPickPos);
			// ��ŷ���� �����ϰ�
			m_pGameInstance->Get_GlobalData()->Pick_Pos = m_vPickPos;
			
			if (m_pGameInstance->Get_DIKeyState(DIK_Q)) {
				// ���� �ø���
				m_pTerrain->Get_VIBuffer()->Change_Height(TerrainRange, TerrainHeightValue);
				m_fTerrainTimeCheck = 0.f;
			}

			if (m_pGameInstance->Get_DIKeyState(DIK_E)) {
				// ���� ������
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

	ImGui::PushItemWidth(300); // ũ������
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

		// ������ ���� ����
		m_pTerrain->Set_Dead(true);
		Safe_Release(m_pTerrain);
		m_pTerrain = nullptr;

		// ���ο� ��ü ����/�޾ƿ���
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
	Change_Mask(fTimeDelta);
	ImGui::PushItemWidth(200); // ũ������
	if (ImGui::BeginListBox("##Mask_List"))
	{
		if (m_pTerrain != nullptr) {
			for (int n = 0; n < m_vecString_Mask.size(); n++)
			{
				bool is_selected = (m_iSelectTile == n);
				string MapName = wstring_to_string(m_vecString_Mask[n]);
				if (ImGui::Selectable(MapName.c_str(), is_selected))
				{
					m_iSelectTile = n;				// ���� ������ ����Ʈ �ڽ��� �ε���
				}

				if (is_selected)
					ImGui::SetItemDefaultFocus();
				// �ݺ������� ����Ʈ�ڽ��� ���õ� ��ü ã��
			}
		}

		ImGui::EndListBox();
	}
	ImGui::PopItemWidth();

	ImGui::PushItemWidth(100); // ũ������
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

	ImGui::PushItemWidth(150); // ũ������
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

	ImGui::PushItemWidth(300); // ũ������
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
			static int SelectStageingImgage = 7;

#pragma region BaseTexture
			ImGui::SeparatorText("Using Base Texture");
			ID3D11ShaderResourceView* UseBaseTexture = m_pTerrain->Get_Texture(CTerrain::TEXTURE_DIFFUSE)->Get_ShaderResourceView(0);
			if (UseBaseTexture) {
				ImGui::Image((void*)UseBaseTexture, ImVec2(100, 100)); // ���ϴ� ũ��� ����
			}
			ImGui::SameLine(0.0f, 30.0f);
			ImVec2 buttonSize(100, 100);
			if (ImGui::Button("Base Change", buttonSize)) {
				m_pTerrain->Get_Texture(CTerrain::TEXTURE_DIFFUSE)->Swap_SRVs(0, SelectStageingImgage);
				m_pTerrain->Get_Texture(CTerrain::TEXTURE_NORMAL)->Swap_SRVs(0, SelectStageingImgage);
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

			// �̹���
			ID3D11ShaderResourceView* UseTexture1 = m_pTerrain->Get_Texture(CTerrain::TEXTURE_DIFFUSE)->Get_ShaderResourceView(1);
			if (UseTexture1) {
				ImGui::Image((void*)UseTexture1, ImVec2(100, 100)); // ���ϴ� ũ��� ����
			}
			ImGui::SameLine();
			ID3D11ShaderResourceView* UseTexture2 = m_pTerrain->Get_Texture(CTerrain::TEXTURE_DIFFUSE)->Get_ShaderResourceView(2);
			if (UseTexture2) {
				ImGui::Image((void*)UseTexture2, ImVec2(100, 100)); // ���ϴ� ũ��� ����
			}
			ImGui::SameLine();
			ID3D11ShaderResourceView* UseTexture3 = m_pTerrain->Get_Texture(CTerrain::TEXTURE_DIFFUSE)->Get_ShaderResourceView(3);
			if (UseTexture3) {
				ImGui::Image((void*)UseTexture3, ImVec2(100, 100)); // ���ϴ� ũ��� ����
			}
			ImGui::SameLine(0.0f, 30.0f);
			ID3D11ShaderResourceView* UseTexture4 = m_pTerrain->Get_Texture(CTerrain::TEXTURE_DIFFUSE)->Get_ShaderResourceView(4);
			if (UseTexture4) {
				ImGui::Image((void*)UseTexture4, ImVec2(100, 100)); // ���ϴ� ũ��� ����
			}
			ImGui::SameLine();
			ID3D11ShaderResourceView* UseTexture5 = m_pTerrain->Get_Texture(CTerrain::TEXTURE_DIFFUSE)->Get_ShaderResourceView(5);
			if (UseTexture5) {
				ImGui::Image((void*)UseTexture5, ImVec2(100, 100)); // ���ϴ� ũ��� ����
			}
			ImGui::SameLine();
			ID3D11ShaderResourceView* UseTexture6 = m_pTerrain->Get_Texture(CTerrain::TEXTURE_DIFFUSE)->Get_ShaderResourceView(6);
			if (UseTexture6) {
				ImGui::Image((void*)UseTexture6, ImVec2(100, 100)); // ���ϴ� ũ��� ����
			}

			// ���� ��ư
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
			ImGui::SameLine(0.0f, 60.0f);
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

			ImVec2 buttonSize2(200, 30);
			if (ImGui::Button("Mask Change", buttonSize2)) {
				m_pTerrain->Get_Texture(CTerrain::TEXTURE_DIFFUSE)->Swap_SRVs(SelectUseImage, SelectStageingImgage);
				m_pTerrain->Get_Texture(CTerrain::TEXTURE_NORMAL)->Swap_SRVs(SelectUseImage, SelectStageingImgage);
			}
#pragma endregion

#pragma region Stageing Texture
			ImGui::SeparatorText("Stageing Mask Texture");
			// 1���� ��
			ID3D11ShaderResourceView* UseTexture7 = m_pTerrain->Get_Texture(CTerrain::TEXTURE_DIFFUSE)->Get_ShaderResourceView(7);
			if (UseTexture7) {
				ImGui::Image((void*)UseTexture7, ImVec2(100, 100)); // ���ϴ� ũ��� ����
			}ImGui::SameLine(0.0f, 10.0f);
			ID3D11ShaderResourceView* UseTexture8 = m_pTerrain->Get_Texture(CTerrain::TEXTURE_DIFFUSE)->Get_ShaderResourceView(8);
			if (UseTexture8) {
				ImGui::Image((void*)UseTexture8, ImVec2(100, 100)); // ���ϴ� ũ��� ����
			}ImGui::SameLine(0.0f, 10.0f);
			ID3D11ShaderResourceView* UseTexture9 = m_pTerrain->Get_Texture(CTerrain::TEXTURE_DIFFUSE)->Get_ShaderResourceView(9);
			if (UseTexture9) {
				ImGui::Image((void*)UseTexture9, ImVec2(100, 100)); // ���ϴ� ũ��� ����
			}ImGui::SameLine(0.0f, 10.0f);
			ID3D11ShaderResourceView* UseTexture10 = m_pTerrain->Get_Texture(CTerrain::TEXTURE_DIFFUSE)->Get_ShaderResourceView(10);
			if (UseTexture10) {
				ImGui::Image((void*)UseTexture10, ImVec2(100, 100)); // ���ϴ� ũ��� ����
			}ImGui::SameLine(0.0f, 10.0f);
			ID3D11ShaderResourceView* UseTexture11 = m_pTerrain->Get_Texture(CTerrain::TEXTURE_DIFFUSE)->Get_ShaderResourceView(11);
			if (UseTexture11) {
				ImGui::Image((void*)UseTexture11, ImVec2(100, 100)); // ���ϴ� ũ��� ����
			}ImGui::SameLine(0.0f, 10.0f);
			ID3D11ShaderResourceView* UseTexture12 = m_pTerrain->Get_Texture(CTerrain::TEXTURE_DIFFUSE)->Get_ShaderResourceView(12);
			if (UseTexture12) {
				ImGui::Image((void*)UseTexture12, ImVec2(100, 100)); // ���ϴ� ũ��� ����
			}

			// ���� ��ư
			if (ImGui::RadioButton("Tile 7", SelectStageingImgage == 7)) {
				SelectStageingImgage = 7;
			}
			ImGui::SameLine(0.0f, 45.0f);
			if (ImGui::RadioButton("Tile 8", SelectStageingImgage == 8)) {
				SelectStageingImgage = 8;
			}
			ImGui::SameLine(0.0f, 45.0f);
			if (ImGui::RadioButton("Tile 9", SelectStageingImgage == 9)) {
				SelectStageingImgage = 9;
			}
			ImGui::SameLine(0.0f, 45.0f);
			if (ImGui::RadioButton("Tile 10", SelectStageingImgage == 10)) {
				SelectStageingImgage = 10;
			}
			ImGui::SameLine(0.0f, 40.0f);
			if (ImGui::RadioButton("Tile 11", SelectStageingImgage == 11)) {
				SelectStageingImgage = 11;
			}
			ImGui::SameLine(0.0f, 40.0f);
			if (ImGui::RadioButton("Tile 12", SelectStageingImgage == 12)) {
				SelectStageingImgage = 12;
			}

			// 2���� ��
			ID3D11ShaderResourceView* UseTexture13 = m_pTerrain->Get_Texture(CTerrain::TEXTURE_DIFFUSE)->Get_ShaderResourceView(13);
			if (UseTexture13) {
				ImGui::Image((void*)UseTexture13, ImVec2(100, 100)); // ���ϴ� ũ��� ����
			}ImGui::SameLine(0.0f, 10.0f);
			ID3D11ShaderResourceView* UseTexture14 = m_pTerrain->Get_Texture(CTerrain::TEXTURE_DIFFUSE)->Get_ShaderResourceView(14);
			if (UseTexture14) {
				ImGui::Image((void*)UseTexture14, ImVec2(100, 100)); // ���ϴ� ũ��� ����
			}ImGui::SameLine(0.0f, 10.0f);
			ID3D11ShaderResourceView* UseTexture15 = m_pTerrain->Get_Texture(CTerrain::TEXTURE_DIFFUSE)->Get_ShaderResourceView(15);
			if (UseTexture15) {
				ImGui::Image((void*)UseTexture15, ImVec2(100, 100)); // ���ϴ� ũ��� ����
			}ImGui::SameLine(0.0f, 10.0f);
			ID3D11ShaderResourceView* UseTexture16 = m_pTerrain->Get_Texture(CTerrain::TEXTURE_DIFFUSE)->Get_ShaderResourceView(16);
			if (UseTexture16) {
				ImGui::Image((void*)UseTexture16, ImVec2(100, 100)); // ���ϴ� ũ��� ����
			}ImGui::SameLine(0.0f, 10.0f);
			ID3D11ShaderResourceView* UseTexture17 = m_pTerrain->Get_Texture(CTerrain::TEXTURE_DIFFUSE)->Get_ShaderResourceView(17);
			if (UseTexture17) {
				ImGui::Image((void*)UseTexture17, ImVec2(100, 100)); // ���ϴ� ũ��� ����
			}ImGui::SameLine(0.0f, 10.0f);
			ID3D11ShaderResourceView* UseTexture18 = m_pTerrain->Get_Texture(CTerrain::TEXTURE_DIFFUSE)->Get_ShaderResourceView(18);
			if (UseTexture18) {
				ImGui::Image((void*)UseTexture18, ImVec2(100, 100)); // ���ϴ� ũ��� ����
			}

			// ���� ��ư
			if (ImGui::RadioButton("Tile 13", SelectStageingImgage == 13)) {
				SelectStageingImgage = 13;
			}
			ImGui::SameLine(0.0f, 40.0f);
			if (ImGui::RadioButton("Tile 14", SelectStageingImgage == 14)) {
				SelectStageingImgage = 14;
			}
			ImGui::SameLine(0.0f, 40.0f);
			if (ImGui::RadioButton("Tile 15", SelectStageingImgage == 15)) {
				SelectStageingImgage = 15;
			}
			ImGui::SameLine(0.0f, 40.0f);
			if (ImGui::RadioButton("Tile 16", SelectStageingImgage == 16)) {
				SelectStageingImgage = 16;
			}
			ImGui::SameLine(0.0f, 40.0f);
			if (ImGui::RadioButton("Tile 17", SelectStageingImgage == 17)) {
				SelectStageingImgage = 17;
			}
			ImGui::SameLine(0.0f, 40.0f);
			if (ImGui::RadioButton("Tile 18", SelectStageingImgage == 18)) {
				SelectStageingImgage = 18;
			}

			// 3���� ��
			ID3D11ShaderResourceView* UseTexture19 = m_pTerrain->Get_Texture(CTerrain::TEXTURE_DIFFUSE)->Get_ShaderResourceView(19);
			if (UseTexture19) {
				ImGui::Image((void*)UseTexture19, ImVec2(100, 100)); // ���ϴ� ũ��� ����
			}ImGui::SameLine(0.0f, 10.0f);
			ID3D11ShaderResourceView* UseTexture20 = m_pTerrain->Get_Texture(CTerrain::TEXTURE_DIFFUSE)->Get_ShaderResourceView(20);
			if (UseTexture20) {
				ImGui::Image((void*)UseTexture20, ImVec2(100, 100)); // ���ϴ� ũ��� ����
			}

			if (ImGui::RadioButton("Tile 19", SelectStageingImgage == 19)) {
				SelectStageingImgage = 19;
			}
			ImGui::SameLine(0.0f, 40.0f);
			if (ImGui::RadioButton("Tile 20", SelectStageingImgage == 20)) {
				SelectStageingImgage = 20;
			}
			

#pragma endregion

		}
		ImGui::End();
	}
	

	return S_OK;
}

#pragma endregion

#pragma region �ʱ�ȭ �ڵ�
HRESULT CLevel_GamePlay::VectorClear()
{
	m_vecString_Mask.clear();

	return S_OK;
}
#pragma endregion

#pragma endregion


HRESULT CLevel_GamePlay::Render()
{
	//SetWindowText(g_hWnd, TEXT("�����÷��̷����Դϴ�."));
	
	// ImGui ������
	if (m_bGuiReady == true)
	{
		ImGui::Render();
		ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
	}
	else
		m_bGuiReady = true;

	return S_OK;
}


#pragma region �⺻�ʱ�ȭ
HRESULT CLevel_GamePlay::Ready_Lights()
{
	/* �����÷��� ������ �ʿ��� ������ �غ��Ѵ�. */
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

	// ImGui ����
	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
}
