#pragma once

#include "Base.h"

BEGIN(Engine)

class CBone final : public CBase
{
private:
	CBone();
	virtual ~CBone() = default;

public:
	const _char* Get_Name() const {
		return m_szName;
	}

	_matrix Get_CombinedTransformationMatrix() const {
		return XMLoadFloat4x4(&m_CombinedTransformationMatrix);
	}

	const _float4x4* Get_CombinedTransformationMatrix_Ptr() const {
		return &m_CombinedTransformationMatrix;
	}

	void Set_TransformationMatrix(_fmatrix TransformationMatrix) {
		XMStoreFloat4x4(&m_TransformationMatrix, TransformationMatrix);
	}

	_float4x4 Get_TransformationMatrix() const {
		return m_TransformationMatrix;
	}


public:
	HRESULT Initialize(DATA_BINNODE* pNode);
	void Update_CombinedTransformationMatrix(const vector<CBone*>& Bones, _fmatrix PreTransformMatrix);

private:
	_char				m_szName[MAX_PATH] = {};

	/* 부모기준으로 표현된 이 뼈만의 상태행렬. */
	_float4x4			m_TransformationMatrix = {};

	/* 이 뼈의 최종적인 상태행렬 ( 내 상태행렬 * 부모의 최종상태행렬 ) */
	/* m_TransformationMatrix * Parent`s m_CombinedTransformationMatrix */
	_float4x4			m_CombinedTransformationMatrix = {};
	_int				m_iParentBoneIndex = { -1 };

public:
	static CBone* Create(DATA_BINNODE* pNode);
	CBone* Clone();
	virtual void Free() override;
};

END