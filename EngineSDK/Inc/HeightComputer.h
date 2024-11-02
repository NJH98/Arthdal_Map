#pragma once

#include "Base.h"

BEGIN(Engine)

class CHeightComputer final : public CBase
{
private:
	CHeightComputer();
	virtual ~CHeightComputer() = default;

public:
	HRESULT Initialize();


public:
	static CHeightComputer* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, HWND hWnd);
	virtual void Free() override;
};

END