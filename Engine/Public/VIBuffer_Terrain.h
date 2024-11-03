#pragma once

#include "VIBuffer.h"

BEGIN(Engine)

class ENGINE_DLL CVIBuffer_Terrain final : public CVIBuffer
{
public:
	typedef struct
	{
		_float2 TerrainXZ = _float2(64.f,64.f);
		_tchar* pHeightMapFilePath = nullptr;
	}TERRAIN_BUFFER_DESC;

private:
	CVIBuffer_Terrain(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CVIBuffer_Terrain(const CVIBuffer_Terrain& Prototype);
	virtual ~CVIBuffer_Terrain() = default;

public:
	virtual HRESULT Initialize_Prototype(const _tchar* pHeightMapFilePath);
	virtual HRESULT Initialize(void* pArg) override;

public:
	void Culling(_fmatrix WorldMatrix);

	void Change_Height(_float Range, _float HowMuch);
	HRESULT Save_HeightMap();

private:
	_uint			m_iNumVerticesX = {};
	_uint			m_iNumVerticesZ = {};

private:
	class CQuadTree*		m_pQuadTree = { nullptr };

public:
	static CVIBuffer_Terrain* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, const _tchar* pHeightMapFilePath);
	virtual CComponent* Clone(void* pArg) override;
	virtual void Free() override;
};


END