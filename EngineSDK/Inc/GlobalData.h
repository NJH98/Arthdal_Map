#pragma once
#include "Base.h"
#include "GameObject.h"

BEGIN(Engine)

class CGlobalData : public CBase
{
private:
	CGlobalData();
	virtual ~CGlobalData() = default;

public:
	HRESULT Initialize();

	GLOBAL_DATA* Get_GlobalData() {
		return &m_GlobalData;
	}

private:
	GLOBAL_DATA m_GlobalData = {};

public:
	static CGlobalData* Create();
	virtual void Free()	override;
};

END