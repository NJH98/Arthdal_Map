#include "..\Public\Base.h"

CBase::CBase()
{
}

unsigned int CBase::AddRef()
{
	return ++m_iRefCnt;
}

unsigned int CBase::Release()
{
	if (0 == m_iRefCnt)
	{
		int a = 10;
		Free();

		delete this;

		return 0;
	}
	else	
		return m_iRefCnt--;	
}

void CBase::Free()
{
}
