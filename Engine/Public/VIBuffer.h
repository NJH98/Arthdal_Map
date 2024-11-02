#pragma once

#include "Component.h"

/* 여러 형태를 띄는 모델(VB, IB)들의 부모가 되는 클래스다. */

BEGIN(Engine)

class ENGINE_DLL CVIBuffer abstract : public CComponent
{
protected:
	CVIBuffer(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CVIBuffer(const CVIBuffer& Prototype);
	virtual ~CVIBuffer() = default;

public:
	virtual HRESULT Initialize_Prototype();
	virtual HRESULT Initialize(void* pArg);
	virtual HRESULT Bind_Buffers();
	virtual HRESULT Render();

protected:
	ID3D11Buffer*				m_pVB = { nullptr };
	ID3D11Buffer*				m_pIB = { nullptr };

	D3D11_BUFFER_DESC			m_BufferDesc{};
	D3D11_SUBRESOURCE_DATA		m_InitialData{};

protected:
	_uint						m_iNumVertexBuffers = {};
	_uint						m_iNumVertices = {};
	_uint						m_iVertexStride = {};

	_uint						m_iNumIndices = {};
	_uint						m_iIndexStride = {};
	DXGI_FORMAT					m_eIndexFormat = {};
	D3D11_PRIMITIVE_TOPOLOGY	m_eTopology = {};

	_float3*				m_pVertexPositions = { nullptr };



protected:
	/* 정점, 인덱스 버퍼를 할당한다. */
	HRESULT Create_Buffer(ID3D11Buffer** ppOut);

public:
	virtual CComponent* Clone(void* pArg) = 0;
	virtual void Free() override;

};

END