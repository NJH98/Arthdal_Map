#include "..\Public\VIBuffer.h"

CVIBuffer::CVIBuffer(ID3D11Device * pDevice, ID3D11DeviceContext * pContext)
	: CComponent { pDevice, pContext }
{

}

CVIBuffer::CVIBuffer(const CVIBuffer & Prototype)
	: CComponent{ Prototype }
	, m_pVB { Prototype.m_pVB }
	, m_pIB { Prototype.m_pIB }
	, m_iNumVertexBuffers { Prototype.m_iNumVertexBuffers }
	, m_iNumVertices { Prototype.m_iNumVertices }
	, m_iVertexStride { Prototype.m_iVertexStride }
	, m_iNumIndices { Prototype.m_iNumIndices }
	, m_iIndexStride { Prototype.m_iIndexStride }
	, m_eIndexFormat { Prototype.m_eIndexFormat }
	, m_eTopology { Prototype.m_eTopology }
	, m_pVertexPositions { Prototype.m_pVertexPositions }
{
	Safe_AddRef(m_pVB);
	Safe_AddRef(m_pIB);	
}

HRESULT CVIBuffer::Initialize_Prototype()
{	
	return S_OK;
}

HRESULT CVIBuffer::Initialize(void * pArg)
{
	return S_OK;
}

HRESULT CVIBuffer::Bind_Buffers()
{
	/* 정점과 인덱스를 통해 모델을 그려나가기위해 필요한 재료들을 장치에 입력시켜놓는다. */

	ID3D11Buffer*		pVertexBuffers[] = {
		m_pVB, 		
		/*m_pVBInstance, */
	};

	_uint		iVertexStrides[] = {
		m_iVertexStride,
		/*m_pVBInstance,*/
	};

	_uint		iOffsets[] = {
		0,
		/*0,*/
	};

	m_pContext->IASetVertexBuffers(0, m_iNumVertexBuffers, pVertexBuffers, iVertexStrides, iOffsets);
	m_pContext->IASetIndexBuffer(m_pIB, m_eIndexFormat, 0);
	m_pContext->IASetPrimitiveTopology(m_eTopology);

	/* 내 정점이 어떠한 멤버들을 가지고 있어요. */
	// m_pContext->SetFVF(D3DFVF_XYZ | D3DFVF_TEX1);

	/* 1. 정점과 인덱스를 통해서 모델을 그리기위해서는 장치가 스스로해주는 일(x) -> 우리가 직접 위치에한 변환, 노말로 빛연산, 텍스쿠드로 샘플링. (Shader)*/
	/* 2. 장치가 직접 지원하는 고정기능렌더링파이프라인사라졌고 사용자정의 렌더링파이프라인을 통해서 그려야하ㅡㄴㄴ 상황. (Shader) */
	/* 3. 이렇게 내가 작성한 셰이더가 내가 지금 그릴려고하는 정점을 잘 받아줄 수 있는 상황인가? 파악해야할 필요가 있다. */
	/* 내가 그릴려고하는 정점의 정보가 이용하려고하는 셰이더에 입력이 가능한가? */

	//ID3D11InputLayout*			pILO;
	//m_pDevice->CreateInputLayout(
	//	내 정점은 어떤멤버들을가지고 있어요, 
	//	내 정점은 몇개의 멤버변수를가지고 있어요,
	//	내가 사용하려고하는 셰이더의 정점 입력 구성 정보, 
	//	셰이더에서 받고자하는 정점의 크기, 
	//	&pILO);

	//m_pContext->IASetInputLayout(pILO);

	return S_OK;
}

HRESULT CVIBuffer::Render()
{
	m_pContext->DrawIndexed(m_iNumIndices, 0, 0);

	return S_OK;
}

HRESULT CVIBuffer::Create_Buffer(ID3D11Buffer ** ppOut)
{
	/* D3D11_BUFFER_DESC : 생성하고자하는 버퍼의 속성(크기를 몇바이트로?, 버퍼를 정적? 동적?) 을 설정한다. */
	/* D3D11_SUBRESOURCE_DATA : 멤버로 void* 를 가지고 있다. void*에 내가 생성하고자흔 ㄴ버퍼에 담을 값등르 내가 미리할당하여 채우고, 할당한 공간의 주솔르 대입해주낟.  */
	return m_pDevice->CreateBuffer(&m_BufferDesc, &m_InitialData, ppOut);
}

void CVIBuffer::Free()
{
	__super::Free();

	if(false == m_isCloned)
		Safe_Delete_Array(m_pVertexPositions);

	Safe_Release(m_pVB);
	Safe_Release(m_pIB);
}
