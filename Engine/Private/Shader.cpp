#include "..\Public\Shader.h"

CShader::CShader(ID3D11Device * pDevice, ID3D11DeviceContext * pContext)
	: CComponent { pDevice, pContext }
{
}

CShader::CShader(const CShader & Prototype)
	: CComponent{ Prototype }
	, m_pEffect { Prototype.m_pEffect }
	, m_iNumPasses { Prototype.m_iNumPasses }
	, m_InputLayouts{ Prototype.m_InputLayouts }
{
	Safe_AddRef(m_pEffect);

	for (auto& pInputLayout : m_InputLayouts)
		Safe_AddRef(pInputLayout);
}

HRESULT CShader::Initialize_Prototype(const _tchar * pShaderFilePath, const D3D11_INPUT_ELEMENT_DESC* pElementDesc, _uint iNumElements)
{
	_uint			iHlslFlag = { 0 };

#ifdef _DEBUG
	iHlslFlag = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
	iHlslFlag = D3DCOMPILE_OPTIMIZATION_LEVEL1;
#endif

	ID3DBlob*		pErrorMsg = { nullptr };

	/* 내가 작성한 쉐이더 파일을 빌드하여 객체화(ID3DX11Effect*) 했다. */
	if (FAILED(D3DX11CompileEffectFromFile(pShaderFilePath, nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, iHlslFlag, 0, m_pDevice, &m_pEffect, nullptr)))
		return E_FAIL;

	ID3DX11EffectTechnique*		pTechnique = m_pEffect->GetTechniqueByIndex(0);
	if (nullptr == pTechnique)
		return E_FAIL;

	D3DX11_TECHNIQUE_DESC		TechniqueDesc{};

	pTechnique->GetDesc(&TechniqueDesc);

	m_iNumPasses = TechniqueDesc.Passes;

	for (size_t i = 0; i < m_iNumPasses; i++)
	{
		ID3DX11EffectPass*	pPass = pTechnique->GetPassByIndex(0);
		if (nullptr == pPass)
			return E_FAIL;

		D3DX11_PASS_DESC	PassDesc{};

		pPass->GetDesc(&PassDesc);

		/*  
		LPCSTR SemanticName;
		UINT SemanticIndex;
		DXGI_FORMAT Format;
		UINT InputSlot;
		UINT AlignedByteOffset;
		D3D11_INPUT_CLASSIFICATION InputSlotClass;
		UINT InstanceDataStepRate;
		*/

		

		/* D3D11_INPUT_ELEMENT_DESC : 정점을 구성하는 멤버변수 하나의 정보를 표현하는 구조체 .*/
		
		//D3D11_INPUT_ELEMENT_DESC	ElementDesc[] =
		//{
		//	{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		//	{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		//};
		
		ID3D11InputLayout*			pInputLayout;

		if (FAILED(m_pDevice->CreateInputLayout(pElementDesc, iNumElements, PassDesc.pIAInputSignature, PassDesc.IAInputSignatureSize, &pInputLayout)))
			return E_FAIL;

		m_InputLayouts.emplace_back(pInputLayout);
	}

	return S_OK;
}

HRESULT CShader::Initialize(void * pArg)
{
	return S_OK;
}

HRESULT CShader::Begin(_uint iPassIndex)
{
	if (iPassIndex >= m_iNumPasses)
		return E_FAIL;

	m_pContext->IASetInputLayout(m_InputLayouts[iPassIndex]);

	ID3DX11EffectPass*	pPass = m_pEffect->GetTechniqueByIndex(0)->GetPassByIndex(iPassIndex);
	if (nullptr == pPass)
		return E_FAIL;

	pPass->Apply(0, m_pContext);		
	
	return S_OK;
}

HRESULT CShader::Bind_Matrix(const _char * pConstantName, const _float4x4 * pMatrix)
{
	/* 쉐이더 안에 선언되어있는 전역변수의 컴객체 핸들을 얻어온다. */
	/* 전달해준 이름을 가진 전역변수가 있다면 유효한 포인터를 얻어온다.(타입에 대한 고려x) */
	ID3DX11EffectVariable*		pVariable = m_pEffect->GetVariableByName(pConstantName);
	if (nullptr == pVariable)
		return E_FAIL;

	///* 특정 변수들에 대해서는 허용되지 않거나 변환과정을 스스로 수행한다.  */
	//pVariable->SetRawValue(void* , 0, 넘기고자하는 데이터의 크기.);	

	/* 얻어온 전역변수가 행렬이었다면 행렬 타입에 대한 핸들을 다시 얻어온다. */
	ID3DX11EffectMatrixVariable*	pMatrixVariable = pVariable->AsMatrix();
	if (nullptr == pMatrixVariable)
		return E_FAIL;	

	return pMatrixVariable->SetMatrix((_float*)pMatrix);
}

HRESULT CShader::Bind_Matrices(const _char * pConstantName, const _float4x4 * pMatrices, _uint iNumMatrices)
{
	/* 쉐이더 안에 선언되어있는 전역변수의 컴객체 핸들을 얻어온다. */
	/* 전달해준 이름을 가진 전역변수가 있다면 유효한 포인터를 얻어온다.(타입에 대한 고려x) */
	ID3DX11EffectVariable*		pVariable = m_pEffect->GetVariableByName(pConstantName);
	if (nullptr == pVariable)
		return E_FAIL;

	///* 특정 변수들에 대해서는 허용되지 않거나 변환과정을 스스로 수행한다.  */
	//pVariable->SetRawValue(void* , 0, 넘기고자하는 데이터의 크기.);	

	/* 얻어온 전역변수가 행렬이었다면 행렬 타입에 대한 핸들을 다시 얻어온다. */
	ID3DX11EffectMatrixVariable*	pMatrixVariable = pVariable->AsMatrix();
	if (nullptr == pMatrixVariable)
		return E_FAIL;

	return pMatrixVariable->SetMatrixArray((_float*)pMatrices, 0, iNumMatrices);
}

HRESULT CShader::Bind_SRV(const _char * pConstantName, ID3D11ShaderResourceView * pSRV)
{
	/* 쉐이더 안에 선언되어있는 전역변수의 컴객체 핸들을 얻어온다. */
	/* 전달해준 이름을 가진 전역변수가 있다면 유효한 포인터를 얻어온다.(타입에 대한 고려x) */
	ID3DX11EffectVariable*		pVariable = m_pEffect->GetVariableByName(pConstantName);
	if (nullptr == pVariable)
		return E_FAIL;

	ID3DX11EffectShaderResourceVariable* pSRVariable = pVariable->AsShaderResource();
	if (nullptr == pSRVariable)
		return E_FAIL;
	
	return pSRVariable->SetResource(pSRV);	
}

HRESULT CShader::Bind_SRVs(const _char * pConstantName, ID3D11ShaderResourceView ** ppSRVs, _uint iNumSRVs)
{
	ID3DX11EffectVariable*		pVariable = m_pEffect->GetVariableByName(pConstantName);
	if (nullptr == pVariable)
		return E_FAIL;

	ID3DX11EffectShaderResourceVariable* pSRVariable = pVariable->AsShaderResource();
	if (nullptr == pSRVariable)
		return E_FAIL;

	return pSRVariable->SetResourceArray(ppSRVs, 0, iNumSRVs);
}

HRESULT CShader::Bind_RawValue(const _char * pConstantName, const void * pData, _uint iLength)
{
	ID3DX11EffectVariable*		pVariable = m_pEffect->GetVariableByName(pConstantName);
	if (nullptr == pVariable)
		return E_FAIL;	

	return pVariable->SetRawValue(pData, 0, iLength);	
}

CShader * CShader::Create(ID3D11Device * pDevice, ID3D11DeviceContext * pContext, const _tchar * pShaderFilePath, const D3D11_INPUT_ELEMENT_DESC* pElementDesc, _uint iNumElements)
{
	CShader*		pInstance = new CShader(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype(pShaderFilePath, pElementDesc, iNumElements)))
	{
		MSG_BOX(TEXT("Failed to Created : CShader"));
		Safe_Release(pInstance);
	}

	return pInstance;
}



CComponent * CShader::Clone(void * pArg)
{
	CShader*		pInstance = new CShader(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed to Cloned : CShader"));
		Safe_Release(pInstance);
	}

	return pInstance;
}


void CShader::Free()
{
	__super::Free();

	for (auto& pInputLayout : m_InputLayouts)
		Safe_Release(pInputLayout);
	m_InputLayouts.clear();

	Safe_Release(m_pEffect);


}
