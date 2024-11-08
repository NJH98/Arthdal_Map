#pragma once

#include "Base.h"
#include "Instancing_Buffer.h"
BEGIN(Engine)

class CGameObject;

class CInstance_Manager final : public CBase
{
private:
	CInstance_Manager(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual ~CInstance_Manager() = default;

public:
	HRESULT Initialize();
	void Push_Instance_Object(const _wstring& strTag, CGameObject* pGameObject);
	void Render();
	void Clear() { m_Buffers.clear(); }
	void ClearData();

private:
	void Render_InstanceModel();

private:
	void AddData(const _wstring strTag, InstancingData& data);

private:
	map<const _wstring, vector<class CGameObject*>> m_GameObjects;
	//list<class CGameObject*>			m_GameObjects;
	list<_float4x4>						m_WorldMatrixs;
	map<const _wstring, CInstancing_Buffer*> m_Buffers;
private:
	ID3D11Device* m_pDevice = { nullptr };
	ID3D11DeviceContext* m_pContext = { nullptr };

public:
	static CInstance_Manager* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual void Free() override;

};

END