#pragma once

#include "Component.h"
#include "Bone.h"
#include "Animation.h"

BEGIN(Engine)

class ENGINE_DLL CModel final : public CComponent
{
public:
	enum TYPE { TYPE_NONANIM, TYPE_ANIM, TYPE_END };
private:
	CModel(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CModel(const CModel& Prototype);
	virtual ~CModel() = default;

public:
	_uint Get_NumMeshes() const {
		return m_iNumMeshes;
	}

	TYPE Get_ModelType() const {
		return m_eType;
	}

	_int Get_BoneIndex(const _char* pBoneName) const;
	_int Get_AnimIndex(const _char* pAnimName) const;
	_matrix Get_BoneCombindTransformationMatrix(_uint iBoneIndex) const {

		return m_Bones[iBoneIndex]->Get_CombinedTransformationMatrix();
	}
	const _float4x4* Get_BoneCombindTransformationMatrix_Ptr(const _char* pBoneName) const {
		return m_Bones[Get_BoneIndex(pBoneName)]->Get_CombinedTransformationMatrix_Ptr();
	}
	void Set_Bones(const vector<class CBone*>* vBone);
	const vector<class CBone*>* Get_Bones() { return &m_Bones; };
	void Set_AnimFrameReset()
	{
		for (auto& i : m_KeyFrameIndices[m_iCurrentAnimIndex])
			i = 0;
		m_CurrentTrackPosition = 0;
		Play_Animation(0.f);
	}

	void Set_CurrentTrackPosition(_double dPosition) { m_CurrentTrackPosition = dPosition; };
	_double& Get_CurrentTrackPosition() { return m_CurrentTrackPosition; }
	void SetUp_Animation(_uint iAnimationIndex, _bool isLoop = false)
	{
		m_iCurrentAnimIndex = iAnimationIndex;
		m_isLoop = isLoop;
	}
	void Set_NextAnimIndex(_uint iAnimIndex, _bool isLoop = false, _float fTime = 0.2f, _float fPlaySpeed = 1.f, _double dSubTime = 0., _double dStartPosition = 0)
	{
		if (m_iNextAnimIndex != iAnimIndex) { m_bInterupted = true; }
		m_iNextAnimIndex = iAnimIndex;
		m_fPlaySpeed = fPlaySpeed;
		m_fLinearTotalTime = fTime;
		m_dSubTime = dSubTime;
		m_isLoop = isLoop;
		m_NextStartTrackPosition = dStartPosition;
	}

	vector<class CMesh*>* Get_Meshs() { return &m_Meshes; }

public:
	virtual HRESULT Initialize_Prototype(TYPE eType, const _tchar* pModelFilePath, _fmatrix PreTransformMatrix);
	virtual HRESULT Initialize(void* pArg) override;
	virtual HRESULT Render(_uint iMeshIndex);




	/* 뼈를 움직인다.(CBone`s m_TransformationMatrix행렬을 갱신한다.) */
	/* 모든 뼈가 가지고 있는 m_CombinedTransformationMatrix를 갱신한다. */
	_bool Play_Animation(_float fTimeDelta);
	_bool Animation_Interpolation(_float fTimeDelta);

	HRESULT Read_BinFile(const _tchar* pModelFilePath);

public:
	HRESULT Bind_Material(class CShader* pShader, const _char* pConstantName, aiTextureType eMaterialType, _uint iMeshIndex);
	HRESULT Check_Material(aiTextureType eMaterialType, _uint iMeshIndex);
	HRESULT Bind_MeshBoneMatrices(class CShader* pShader, const _char* pConstantName, _uint iMeshIndex);

	HRESULT Bind_Bone_Mesh(CModel* pOtherModel);

private:
	DATA_BINSCENE* m_pBin_Scene = nullptr;
	TYPE							m_eType = { TYPE_END };

private: /* 메시의 정보를 저장한다. */
	_uint							m_iNumMeshes = { 0 };
	vector<class CMesh*>			m_Meshes;
	_float4x4						m_PreTransformMatrix = {};

private:
	_uint							m_iNumMaterials = { 0 };
	vector<MESH_MATERIAL>			m_Materials;

private:
	vector<class CBone*>			m_Bones;
	vector< _float4x4>				m_StartBonesTransforms;
	vector<class CBone*>			m_NewBones;

private:
	_bool							m_isLoop = { false };
	_uint							m_iCurrentAnimIndex = { 0 };
	_uint							m_iNumAnimations = { 0 };
	vector<class CAnimation*>		m_Animations;

	/* 현재 애니메이션의 재생 위치. */
	_double							m_CurrentTrackPosition = {};
	_double							m_NextStartTrackPosition = { 0 };
	/*For. NextIndex*/
	_uint							m_iNextAnimIndex = 0;
	_float							m_fLinearTotalTime = 0.f;
	_float							m_fLinearCurrentTime = 0.f;
	/*For. Lineared*/
	_bool							m_bLinearFinished = true;
	_bool							m_bInterupted = false;

	/* 각 애니메이션당 각 채널들의 현재 키프레임인덱스 */
	vector<vector<_uint>>			m_KeyFrameIndices;
	_float							m_fPlaySpeed = 1.f;
	_double							m_dSubTime = 0.;

public:
	HRESULT	Ready_Meshes();
	HRESULT Ready_Materials(const _tchar* pModelFilePath);
	HRESULT Ready_Bones();
	HRESULT Ready_Animations();

	HRESULT	Safe_Release_Scene();

public:
	static CModel* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, TYPE eType, const _tchar* pModelFilePath, _fmatrix PreTransformMatrix = XMMatrixIdentity());
	virtual CComponent* Clone(void* pArg) override;
	virtual void Free() override;
};

END