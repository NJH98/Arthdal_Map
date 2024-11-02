#pragma once

#include "Base.h"

BEGIN(Engine)

class CAnimation final : public CBase
{
private:
	CAnimation();
	virtual ~CAnimation() = default;

public:
	HRESULT Initialize(DATA_BINANIM* pAIAnimation, vector<_uint>& KeyFrameIndices, class CModel* pModel);
	_bool Update_TransformationMatrices(const vector<class CBone*>& Bones, _double* pCurrentTrackPosition, vector<_uint>& CurrentKeyFrameIndices, _bool isLoop, _float fTimeDelta, _float fPlaySpeed = 1.f, _double dSubTime = 0.);

public://get
	vector<class CChannel*>	Get_Channels(void) { return m_Channels; }
	const _char* Get_Name() const { return m_szName; }
	_uint& Get_KeyFrameIdxex() { return m_iKeyFrameIndex; }
private:
	char				m_szName[MAX_PATH] = "";
	/* 이 애니메이션이 사용하는 뼈의 갯수 */
	_uint						m_iNumChannels = { 0 };
	/* 실제 뼈들의 정보*/
	/* Channel : 뼈 (시간에 따른 이 뼈의 상태 행렬을 보관한다) */
	vector<class CChannel*>		m_Channels;

	/* 이 애니메이션을 구동하기위해 거리는 전체 거리. */
	_double						m_Duration = {};

	/* 애니메이션의 초당 재생속도 */
	_double						m_SpeedPerSec = {};

	//키프레임 인덱스
	_uint				m_iKeyFrameIndex = { 0 };


public:
	static CAnimation* Create(DATA_BINANIM* pAIAnimation, vector<_uint>& KeyFrameIndices, class CModel* pModel);
	CAnimation* Clone();
	virtual void Free() override;
};

END