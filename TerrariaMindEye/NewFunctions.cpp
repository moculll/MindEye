#include "pch.h"
#include "NewFunctions.h"

extern NewFunctions::D3DENDSCENE NewFunctions::fpD3dEndScene = NULL;
extern NewFunctions::D3DDRAWINDEXEDPRIMITIVE NewFunctions::fpD3dDrawIndexedPrimitive = NULL;

HRESULT STDMETHODCALLTYPE NewFunctions::D3dEndScene(LPDIRECT3DDEVICE9 p_pDevice)
{
    // TODO：执行自己的代码
    // D3DRECT r = {400 - 16, 300 - 16, 400 + 16, 300 + 16};
    // D3DCOLOR color = D3DCOLOR_ARGB(100, 245, 125, 215);
    // p_pDevice->Clear(1, &r, D3DCLEAR_TARGET, color, 0, 0);

    // 执行原函数
    return NewFunctions::fpD3dEndScene(p_pDevice);
}

HRESULT STDMETHODCALLTYPE NewFunctions::D3dDrawIndexedPrimitive(LPDIRECT3DDEVICE9 p_pDevice, D3DPRIMITIVETYPE PrimitiveType, INT BaseVertexIndex, UINT MinVertexIndex, UINT NumVertices, UINT startIndex, UINT primCount)
{
    // TODO：执行自己的代码

    // 执行原函数
    return NewFunctions::fpD3dDrawIndexedPrimitive(p_pDevice, PrimitiveType, BaseVertexIndex, MinVertexIndex, NumVertices, startIndex, primCount);
}
