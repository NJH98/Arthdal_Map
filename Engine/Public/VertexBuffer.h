#pragma once
#include "Engine_Defines.h"
#include "GameInstance.h"

BEGIN(Engine)

class VertexBuffer
{
public:
	VertexBuffer();
	~VertexBuffer();

	ComPtr<ID3D11Buffer> GetComPtr() { return m_vertexBuffer; }
	_uint32 GetStride() { return m_iStride; }
	_uint32 GetOffset() { return m_iOffset; }
	_uint32 GetCount() { return m_iCount; }
	_uint32 GetSlot() { return m_iSlot; }

	template<typename T>
	void Create(const vector<T>& vertices, _uint32 slot = 0, bool cpuWrite = false, bool gpuWrite = false)
	{
		m_iStride = sizeof(T);
		m_iCount = static_cast<_uint32>(vertices.size());

		m_iSlot = slot;
		m_bCpuWrite = cpuWrite;
		m_bGpuWrite = gpuWrite;

		D3D11_BUFFER_DESC desc;
		ZeroMemory(&desc, sizeof(desc));
		desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		desc.ByteWidth = (_uint32)(m_iStride * m_iCount);

		if (cpuWrite == false && gpuWrite == false)
		{
			desc.Usage = D3D11_USAGE_IMMUTABLE; // CPU Read, GPU Read
		}
		else if (cpuWrite == true && gpuWrite == false)
		{
			desc.Usage = D3D11_USAGE_DYNAMIC; // CPU Write, GPU Read
			desc.CPUAccessFlags = D3D10_CPU_ACCESS_WRITE;
		}
		else if (cpuWrite == false && gpuWrite == true) // CPU Read, GPU Write
		{
			desc.Usage = D3D11_USAGE_DEFAULT;
		}
		else
		{
			desc.Usage = D3D11_USAGE_STAGING;
			desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ | D3D11_CPU_ACCESS_WRITE;
		}

		D3D11_SUBRESOURCE_DATA data;
		ZeroMemory(&data, sizeof(data));
		data.pSysMem = vertices.data();

		HRESULT hr = GET_DVC->CreateBuffer(&desc, &data, m_vertexBuffer.GetAddressOf());
		CHECK(hr);
	}

	void PushData()
	{
		GET_DC->IASetVertexBuffers(m_iSlot, 1, m_vertexBuffer.GetAddressOf(), &m_iStride, &m_iOffset);
	}

private:
	ComPtr<ID3D11Buffer> m_vertexBuffer;

	_uint32 m_iStride = 0;
	_uint32 m_iOffset = 0;
	_uint32 m_iCount = 0;

	_uint32 m_iSlot = 0;
	bool m_bCpuWrite = false;
	bool m_bGpuWrite = false;
};

END
