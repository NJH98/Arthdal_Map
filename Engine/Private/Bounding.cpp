#include "..\Public\Bounding.h"

CBounding::CBounding(ID3D11Device * pDevice, ID3D11DeviceContext * pContext)
{
}

HRESULT CBounding::Render(PrimitiveBatch<VertexPositionColor>* pBatch)
{
	return S_OK;
}

_bool CBounding::Intersect(CCollider::TYPE eColliderType, CBounding * pBounding)
{
	return _bool();
}

HRESULT CBounding::Initialize(CBounding::BOUNDING_DESC* pBoundingDesc)
{
	return S_OK;
}

void CBounding::Update(_fmatrix WorldMatrix)
{

}

void CBounding::Free()
{
	__super::Free();

	Safe_Release(m_pContext);
	Safe_Release(m_pDevice);
}
