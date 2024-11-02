#pragma once

#include "Engine_Defines.h"

/* 1. 모든 클래스의 부모가 되는 클래스입니다. (Super Class) */
/* 2. 레퍼런스 카운트를 관리한다. */
/* 2-1. 레퍼런스 카운트를 증가시킨다. AddRef() */
/* 2-2. 레퍼런스 카운트를 감소시킨다. or 삭제한다. Release() */

BEGIN(Engine)

class ENGINE_DLL CBase abstract
{
protected:
	CBase();
	virtual ~CBase() = default;

public:
	unsigned int AddRef();
	unsigned int Release();

private:
	unsigned int				m_iRefCnt = { 0 };

public:
	/* 생성관련된 함수는 필요없다. */

	/* 삭제관련된 함수.*/
	virtual void Free();

};

END


