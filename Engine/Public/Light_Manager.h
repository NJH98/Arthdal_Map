#pragma once

#include "Base.h"

BEGIN(Engine)

class CLight_Manager final : public CBase
{
private:
	CLight_Manager();
	virtual ~CLight_Manager() = default;

public:
	LIGHT_DESC*			Get_LightDesc(_uint iIndex);
	class CLight*		Get_Light(_uint iIndex);
	void				Delete_Light(_uint iIndex);
	list<class CLight*> Get_LightList() { return m_Lights; }

public:
	HRESULT Initialize();
	HRESULT Add_Light(const LIGHT_DESC& LightDesc);
	HRESULT Render(class CShader* pShader, class CVIBuffer_Rect* pVIBuffer); 

private:
	list<class CLight*>				m_Lights;

public:
	static CLight_Manager* Create();
	virtual void Free() override;
};

END