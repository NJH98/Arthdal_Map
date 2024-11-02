#pragma once

#include "Base.h"

BEGIN(Engine)

/* 애님에시녀을 구동을 위해 사용하는 뼈 중 하나 */
/* 시간에 따른 상태 정보를 가지고 있는다. 초기 생성시 모든 시간에 따른 상태값들을 미리 저장해둔다. */
/* 추후 런타임간에 이 채널에게 현재 애니메이션의 재생 위치를 전달해주면 재생위치에 맞는 상태행렬(m_TransformatoinMatrix )을 만든다. */

/* KEYFRAME : 특정 재생위치에서 이 뼈가 취해야할 상태(행렬)를 분해(스자이)하여 표현한 구조체.*/

class CChannel final : public CBase
{
private:
	CChannel();
	virtual ~CChannel() = default;

public:
	HRESULT Initialize(const DATA_BINCHANNEL* pAIChannel, const class CModel* pModel);
	void Update_TransformationMatrix(const vector<class CBone*>& Bones, _uint* pCurrentKeyFrameIndex, _double CurrentTrackPosition);



public:
	KEYFRAME	Get_StartKeyFrame(void) { return m_KeyFrames[0]; }

private:
	_char				m_szName[MAX_PATH] = {};

	_uint				m_iBoneIndex = { 0 };
	_uint				m_iNumKeyFrames = {};
	vector<KEYFRAME>	m_KeyFrames;

public:
	static CChannel* Create(DATA_BINCHANNEL* pAIChannel, class CModel* pModel);
	virtual void Free() override;
};

END