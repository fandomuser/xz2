#pragma once
#include <Windows.h>
#include <d3d11.h>
#include <dxgi.h>

namespace Hooks {
    void Init();
    void Shutdown();

    // Hooked functions
    HRESULT __stdcall Present(IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags);
}
