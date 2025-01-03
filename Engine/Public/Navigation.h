#pragma once

#include "Component.h"

BEGIN(Engine)

class ENGINE_DLL CNavigation final : public CComponent
{
public:
	typedef struct
	{
		_int			iCurrentIndex = {-1};
	}NAVIGATION_DESC;

	typedef struct
	{
		_float3 PointA = {};
		_float3 PointB = {};
		_float3 PointC = {};

		_int NeighborIndex_AB = {};
		_int NeighborIndex_BC = {};
		_int NeighborIndex_CA = {};

		_bool IsRide = false;
		_int CellName = -1;
	}CELL_DESC;

private:
	CNavigation(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CNavigation(const CNavigation& Prototype);
	virtual ~CNavigation() = default;

public:
	virtual HRESULT Initialize_Prototype(const _wstring& strNavigationDataFile);
	virtual HRESULT Initialize(void* pArg) override; 

public:
	void Update(_fmatrix TerrainWorldMatrix);
	_bool isMove(_fvector vPosition);

public:
	void Clear_Cell();
	void Delete_Cell(_uint iter);
	void Add_Cell(_float3 PointA, _float3 PointB, _float3 PointC);
	void Add_Cell_NoneCheck(_float3 PointA, _float3 PointB, _float3 PointC);

	void Add_Bin_Cell(CELL_DESC CellDesc);

	vector<class CCell*> Get_vecCell() { return m_Cells; }
	void Set_CellIndex(_int Index) { m_iCurrentCellIndex = Index; }

	class CCell* Get_OnCell() { return m_Cells[m_iCurrentCellIndex]; }

#ifdef _DEBUG
public:
	virtual HRESULT Render() override;
#endif

private:
	vector<class CCell*>				m_Cells;
	_int								m_iCurrentCellIndex = { -1 };
	static _float4x4					m_WorldMatrix;

#ifdef _DEBUG
private:
	class CShader*						m_pShader = { nullptr };
#endif

public:
	HRESULT SetUp_Neighbors();

public:
	static CNavigation* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, const _wstring& strNavigationDataFile);
	virtual CComponent* Clone(void* pArg) override;
	virtual void Free() override;
};

END