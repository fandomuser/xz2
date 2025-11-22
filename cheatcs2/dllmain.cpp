#include "pch.h"
#include <Windows.h>
#include <iostream>
#include "src/hooks.h"

// Forward declaration of the main cheat thread
DWORD WINAPI MainThread(LPVOID lpReserved);

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        DisableThreadLibraryCalls(hModule);
        CreateThread(nullptr, 0, MainThread, hModule, 0, nullptr);
        break;
    case DLL_PROCESS_DETACH:
        Hooks::Shutdown();
        break;
    }
    return TRUE;
}

DWORD WINAPI MainThread(LPVOID lpReserved)
{
    // Allocate console for debugging
    AllocConsole();
    FILE* f;
    freopen_s(&f, "CONOUT$", "w", stdout);
    
    std::cout << "CS2 Cheat Injected!" << std::endl;

    Hooks::Init();

    // Keep thread alive
    while (!GetAsyncKeyState(VK_END)) {
        Sleep(100);
    }

    Hooks::Shutdown();
    
    if (f) fclose(f);
    FreeConsole();
    FreeLibraryAndExitThread((HMODULE)lpReserved, 0);
    return 0;
}
