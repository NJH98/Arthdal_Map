#include "GlobalData.h"

CGlobalData::CGlobalData()
{
}

HRESULT CGlobalData::Initialize()
{
	return S_OK;
}

CGlobalData* CGlobalData::Create()
{
	CGlobalData* pInstance = new CGlobalData();

	if (FAILED(pInstance->Initialize()))
	{
		MSG_BOX(TEXT("Failed to Cloned : CGlobalData"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CGlobalData::Free()
{
	__super::Free();
}
