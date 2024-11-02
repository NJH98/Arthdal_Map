#pragma once

#include "VIBuffer.h"
#include "Model.h"

/* 모델 = 메시 + 메시 + 메시 .. */
/* 메시로 구분하는 이유 : 파츠의 교체를 용이하게 만들어주기 위해서.*/
/* 메시 = 정점버퍼 + 인덱스 */

BEGIN(Engine)

class CMesh final : public CVIBuffer
{
private:
	CMesh(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CMesh(const CMesh& Prototype);
	virtual ~CMesh() = default;

public:
	_uint Get_MaterialIndex() const {
		return m_iMaterialIndex;
	}

public:
	virtual HRESULT Initialize_Prototype(const CModel* pModel, const DATA_BINMESH* pAIMesh, _fmatrix PreTransformMatrix);
	virtual HRESULT Initialize(void* pArg) override;

public:
	HRESULT Bind_BoneMatrices(const CModel* pModel, class CShader* pShader, const _char* pConstantName);
	HRESULT Move_Bone(const CModel* pModel);

public:
	void Set_BoneNum(_uint iNum) { m_iNumBones = iNum; }
	const _char* GetName() const { return m_szName; }
	bool IsNameEqual(const _char* targetName) const {
		return strcmp(m_szName, targetName) == 0;
	}

private:
	_char				m_szName[MAX_PATH] = {};
	_uint				m_iMaterialIndex = { 0 };
	_uint				m_iNumBones = { 0 };

	/* uint : 모델에 로드해놓은 전체 뼈 중의 인덱스를 이야기한다. */
	vector<_int>		m_BoneIndices;

	_float4x4			m_BoneMatrices[g_iMaxMeshBones] = {};
	vector<_float4x4>	m_OffsetMatrices;


private:
	HRESULT	Ready_VertexBuffer_NonAnim(const DATA_BINMESH* pAIMesh, _fmatrix PreTransformMatrix);
	HRESULT	Ready_VertexBuffer_Anim(const CModel* pModel, const DATA_BINMESH* pAIMesh);


public:
	static CMesh* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, const CModel* pModel, const DATA_BINMESH* pAIMesh, _fmatrix PreTransformMatrix);
	virtual CComponent* Clone(void* pArg);
	virtual void Free() override;
};

END