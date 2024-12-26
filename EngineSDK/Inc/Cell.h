#pragma once

#include "Base.h"
#include "Transform.h"

BEGIN(Engine)

class ENGINE_DLL CCell final : public CBase
{
public:
	enum POINT { POINT_A, POINT_B, POINT_C, POINT_END };
	enum LINE { LINE_AB, LINE_BC, LINE_CA, LINE_END };
private:
	CCell(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual ~CCell() = default;
	
public:
	_vector Get_Point_vector(POINT ePoint) const {
		return XMLoadFloat3(&m_vPoints[ePoint]);
	}

	_vector Get_Point(POINT ePoint) const {
		return XMLoadFloat3(&m_vPoints[ePoint]);
	}

	_int Get_Neighbor(LINE eLine) const {
		return m_iNeighborIndices[eLine];
	}

	void Set_Neighbor(LINE eLine, CCell* pNeighbor) {
		m_iNeighborIndices[eLine] = pNeighbor->m_iIndex;
	}

	void No_Neighbor(LINE eLine) {
		m_iNeighborIndices[eLine] = -1;
	}

	void Set_Bin_Neighbor(LINE eLine, _int iNeighborIndex) {
		m_iNeighborIndices[eLine] = iNeighborIndex;
	}

public:
	HRESULT Initialize(const _float3* pPoints, _int iIndex);
	_bool Compare_Points(_fvector vSour, _fvector vDest);
	_bool isIn(_fvector vPosition, _int* pNeighborIndex);
	void Set_PickCell(_bool IsPick) { m_bIsPick = IsPick; }
	_bool Get_PickCell() { return m_bIsPick; }
	_int Get_Index() { return m_iIndex; }
	void Set_Index(_int Index) { m_iIndex = Index; }

	_bool Get_Ride() { return m_bIsRide; }
	void Set_Ride(_bool IsRide) { m_bIsRide = IsRide; }

	_int Get_Name() { return m_iName; }
	void Set_Name(_int Name) { m_iName = Name; }

	_int Get_AreaIndex() { return m_iAreaIndex; }
	_bool Get_Render() { return m_bIsRender; }
	void Set_Render(_bool Temp) { m_bIsRender = Temp; }

	void Cell_Landing(CTransform* pTransform);

#ifdef _DEBUG
public:
	HRESULT Render();
#endif

private:
	class CGameInstance*			m_pGameInstance = { nullptr };
	ID3D11Device*					m_pDevice = { nullptr };
	ID3D11DeviceContext*			m_pContext = { nullptr };
	_int							m_iIndex = {};
	_float3							m_vPoints[POINT_END] = {};
	_int							m_iNeighborIndices[LINE_END] = { -1, -1, -1 };

	_bool							m_bIsPick = false;
	_bool							m_bIsRide = false;
	_int							m_iName = -1;

	_int							m_iAreaIndex = -1;
	_bool							m_bIsRender = false;

#ifdef _DEBUG
private:
	class CVIBuffer_Cell*			m_pVIBuffer = { nullptr };
#endif

public:
	static CCell* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, const _float3* pPoints, _int iIndex);
	virtual void Free() override;
};

END