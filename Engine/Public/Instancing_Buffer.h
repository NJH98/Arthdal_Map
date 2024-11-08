#pragma once
#include "Base.h"

BEGIN(Engine)
class VertexBuffer;
#define MAX_MESH_INSTANCE 100

class ENGINE_DLL CInstancing_Buffer : public CBase
{
private:
	CInstancing_Buffer(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CInstancing_Buffer(const CInstancing_Buffer& Prototype);
	virtual ~CInstancing_Buffer() = default;

public:
	void ClearData();
	void AddData(InstancingData& data);
	void PushData();

public:
	_uint32 Get_Count() { return static_cast<_uint32>(m_vecData.size()); }

	void	SetID(_uint64 instanceId) { m_iInstanceID = instanceId; }
	_uint64	GetID() { return m_iInstanceID; }

private:
	void CreateBuffer(_uint32 maxCount = MAX_MESH_INSTANCE);

private:
	_uint64						m_iInstanceID = { 0 };
	shared_ptr<VertexBuffer>	_instanceBuffer;
	_uint32						m_iMaxCount = 0;
	vector<InstancingData>		m_vecData;

	_uint						m_iInstanceStride = { 0 };


public:
	virtual HRESULT Initialize();

private:

public:
	static CInstancing_Buffer* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual void Free() override;
};


END