#include "pch.h"
#include "hooks.h"
#include "gui.h"
#include "features/misc.h"
#include "features/visuals.h"
#include "features/aimbot.h"
#include <MinHook.h>
#include <imgui.h>
#include <imgui_impl_dx11.h>
#include <imgui_impl_win32.h>
#include <iostream>

// Function pointer types
typedef HRESULT(__stdcall* Present_t)(IDXGISwapChain*, UINT, UINT);

// Original functions
Present_t oPresent = nullptr;

// Global variables for DirectX
ID3D11Device* g_pd3dDevice = nullptr;
ID3D11DeviceContext* g_pd3dContext = nullptr;
ID3D11RenderTargetView* g_mainRenderTargetView = nullptr;
HWND g_hwnd = nullptr;
bool g_Initialized = false;
bool g_ShowMenu = true;

// Forward declare WndProc hook if needed (usually for input)
WNDPROC oWndProc = nullptr;
extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

LRESULT __stdcall WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    // Handle INSERT key toggle
    if (uMsg == WM_KEYDOWN && wParam == VK_INSERT) {
        g_ShowMenu = !g_ShowMenu;
        
        // Show/hide and unlock/lock cursor based on menu state
        if (g_ShowMenu) {
            // Show cursor and unlock from window
            while (ShowCursor(TRUE) < 0);
            ClipCursor(NULL);
        } else {
            // Hide cursor when menu is closed
            while (ShowCursor(FALSE) >= 0);
        }
        
        return 0;
    }

    // If menu is open, let ImGui handle input
    if (g_ShowMenu) {
        if (ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam))
            return true;
    }

    return CallWindowProc(oWndProc, hWnd, uMsg, wParam, lParam);
}

HRESULT __stdcall Hooks::Present(IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags) {
    if (!g_Initialized) {
        if (FAILED(pSwapChain->GetDevice(__uuidof(ID3D11Device), (void**)&g_pd3dDevice)))
            return oPresent(pSwapChain, SyncInterval, Flags);

        g_pd3dDevice->GetImmediateContext(&g_pd3dContext);

        DXGI_SWAP_CHAIN_DESC sd;
        pSwapChain->GetDesc(&sd);
        g_hwnd = sd.OutputWindow;

        // Create Render Target View
        ID3D11Texture2D* pBackBuffer;
        pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);
        g_pd3dDevice->CreateRenderTargetView(pBackBuffer, NULL, &g_mainRenderTargetView);
        pBackBuffer->Release();

        // Setup ImGui
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO(); (void)io;
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
        io.MouseDrawCursor = true;

        ImGui::StyleColorsDark();

        // Setup Platform/Renderer backends
        ImGui_ImplWin32_Init(g_hwnd);
        ImGui_ImplDX11_Init(g_pd3dDevice, g_pd3dContext);

        // Hook WndProc
        oWndProc = (WNDPROC)SetWindowLongPtr(g_hwnd, GWLP_WNDPROC, (LONG_PTR)WndProc);

        g_Initialized = true;
    }

    // Update mouse cursor visibility
    ImGuiIO& io = ImGui::GetIO();
    io.MouseDrawCursor = g_ShowMenu;

    ImGui_ImplDX11_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    // Draw Menu only if visible
    if (g_ShowMenu) {
        GUI::Render();
    }
    
    // Run cheat features
    Misc::Run();
    // Visuals::Run(); // Disabled - causes crash
    // Aimbot::Run(); // Disabled - causes crash

    ImGui::Render();
    g_pd3dContext->OMSetRenderTargets(1, &g_mainRenderTargetView, NULL);
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

    return oPresent(pSwapChain, SyncInterval, Flags);
}

void Hooks::Init() {
    // Create dummy device to get vtable
    D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_11_0;
    DXGI_SWAP_CHAIN_DESC sd;
    ZeroMemory(&sd, sizeof(sd));
    sd.BufferCount = 1;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = GetForegroundWindow(); // Just use foreground window for dummy
    sd.SampleDesc.Count = 1;
    sd.Windowed = TRUE;
    sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

    IDXGISwapChain* swapChain = nullptr;
    ID3D11Device* device = nullptr;
    ID3D11DeviceContext* context = nullptr;

    if (FAILED(D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, 0, &featureLevel, 1, D3D11_SDK_VERSION, &sd, &swapChain, &device, NULL, &context))) {
        return;
    }

    // Get Present address from vtable
    void** vtable = *(void***)swapChain;
    void* presentAddress = vtable[8]; // Present is index 8

    // Cleanup dummy
    swapChain->Release();
    device->Release();
    context->Release();

    // Initialize MinHook
    if (MH_Initialize() != MH_OK) {
        return;
    }

    // Create Hook
    if (MH_CreateHook(presentAddress, &Hooks::Present, (LPVOID*)&oPresent) != MH_OK) {
        return;
    }

    // Enable Hook
    if (MH_EnableHook(MH_ALL_HOOKS) != MH_OK) {
        return;
    }
}

void Hooks::Shutdown() {
    MH_DisableHook(MH_ALL_HOOKS);
    MH_Uninitialize();
    
    if (g_hwnd) {
        SetWindowLongPtr(g_hwnd, GWLP_WNDPROC, (LONG_PTR)oWndProc);
    }
    
    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
    
    if (g_mainRenderTargetView) { g_mainRenderTargetView->Release(); g_mainRenderTargetView = nullptr; }
    if (g_pd3dContext) { g_pd3dContext->Release(); g_pd3dContext = nullptr; }
    if (g_pd3dDevice) { g_pd3dDevice->Release(); g_pd3dDevice = nullptr; }
}
