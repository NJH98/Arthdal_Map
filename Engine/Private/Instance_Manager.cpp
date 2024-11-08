#include "..\Public\Instance_Manager.h"
#include "GameObject.h"
#include "Model.h"
#include "Mesh.h"
#include "Shader.h"

CInstance_Manager::CInstance_Manager(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: m_pDevice{ pDevice }
	, m_pContext{ pContext }
{
	Safe_AddRef(m_pDevice);
	Safe_AddRef(m_pContext);
}

HRESULT CInstance_Manager::Initialize()
{
	return S_OK;
}

void CInstance_Manager::Push_Instance_Object(const _wstring& strTag, CGameObject* pGameObject)
{
	m_GameObjects[strTag].push_back(pGameObject);
	//Safe_AddRef(pGameObject);
}

void CInstance_Manager::Render()
{
	Render_InstanceModel();
	ClearData();
}

void CInstance_Manager::ClearData()
{
	for (auto& pair : m_Buffers)
	{
		pair.second->ClearData();
	}

	for (auto& pair : m_GameObjects)
	{
		pair.second.clear();
	}

}

void CInstance_Manager::Render_InstanceModel()
{
	if (m_GameObjects.empty())
		return;

	for (auto& pair : m_GameObjects)
	{
		
		const vector<CGameObject*>& vec = pair.second;
		if (vec.empty())
			continue;
		{
			const _wstring& instanceId = pair.first;

			for (_int32 i = 0; i < vec.size(); i++)
			{
				CGameObject* gameObject = vec[i];
				InstancingData data;
				data.world = gameObject->Get_WorldMatrix();
				AddData(instanceId, data);
			}
			CInstancing_Buffer* buffer = m_Buffers[instanceId];
			static_cast<CModel*>(vec[0]->Find_Component(TEXT("Com_Model")))->RenderInstancing(static_cast<CShader*>(vec[0]->Find_Component(TEXT("Com_Shader"))), buffer);
		}
	}
}

void CInstance_Manager::AddData(const _wstring strTag, InstancingData& data)
{
	if (m_Buffers.find(strTag) == m_Buffers.end())
		m_Buffers[strTag] = CInstancing_Buffer::Create(m_pDevice, m_pContext);

	m_Buffers[strTag]->AddData(data);
}


CInstance_Manager* CInstance_Manager::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CInstance_Manager* pInstance = new CInstance_Manager(pDevice, pContext);

	if (FAILED(pInstance->Initialize()))
	{
		MSG_BOX(TEXT("Failed to Created : CInstance_Manager"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CInstance_Manager::Free()
{
	__super::Free();
	for (auto& pair : m_Buffers)
	{
		Safe_Release(pair.second);
	}

	Safe_Release(m_pDevice);
	Safe_Release(m_pContext);
}
