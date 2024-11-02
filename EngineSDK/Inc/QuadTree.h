#pragma once

#include "Base.h"

BEGIN(Engine)

class CQuadTree final : public CBase
{
public:
	enum CORNER { CORNER_LT, CORNER_RT, CORNER_RB, CORNER_LB, CORNER_END };
	enum NEIGHBOR { NEIGHBOR_LEFT, NEIGHBOR_TOP, NEIGHBOR_RIGHT, NEIGHBOR_BOTTOM, NEIGHBOR_END };
private:
	CQuadTree();
	virtual ~CQuadTree() = default;

public:
	HRESULT Initialize(_uint iLT, _uint iRT, _uint iRB, _uint iLB);
	void Culling(class CGameInstance* pGameInstance, const _float3* pVertexPositions, _uint* pIndices, _uint* pNumIndices);
	void Make_Neighbors();



private:
	CQuadTree*			m_pChildren[CORNER_END] = { nullptr, nullptr, nullptr, nullptr };
	_uint				m_iCorners[CORNER_END] = {};
	_uint				m_iCenter = {};
	CQuadTree*			m_pNeighbors[NEIGHBOR_END] = { nullptr, nullptr, nullptr, nullptr };



private:
	_bool isDraw(_fvector vCamPosition, const _float3* pVertexPositions);

public:
	static CQuadTree* Create(_uint iLT, _uint iRT, _uint iRB, _uint iLB);
	virtual void Free() override;
};

END