#pragma once

#include "Base.h"

BEGIN(Engine)

class CTimer final : public CBase
{
private:
	CTimer();
	virtual ~CTimer() = default;

public:
	float Get_TimeDelta() {	return m_fTimeDelta;	}

public:
	HRESULT Initialize();
	void Update();

private:
	LARGE_INTEGER			m_CurTime = {};
	LARGE_INTEGER			m_OldTime = {};
	LARGE_INTEGER			m_OriginTime = {};
	LARGE_INTEGER			m_CpuTick = {};

	/* 메세지 루프를 한바퀴 도는데 걸리는 시간. */
	float					m_fTimeDelta = { 0.f };

public:
	static CTimer* Create();
	virtual void Free() override;
};

END