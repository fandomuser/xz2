#include "pch.h"
#include "misc.h"
#include "../config.h"
#include "../offsets.h"

namespace Misc
{
    static bool jumpState = false;

    void Bunnyhop()
    {
        if (!g_Config.misc_bhop) return;

        // Get client.dll base address
        HMODULE clientDll = GetModuleHandleA("client.dll");
        if (!clientDll) return;

        __try
        {
            // Calculate dwForceJump address
            uintptr_t forceJumpAddr = (uintptr_t)clientDll + Offsets::dwForceJump;
            
            // Check if address is valid
            if (IsBadReadPtr((void*)forceJumpAddr, sizeof(int))) return;
            if (IsBadWritePtr((void*)forceJumpAddr, sizeof(int))) return;

            // Check if SPACE is pressed
            if (GetAsyncKeyState(VK_SPACE) & 0x8000)
            {
                DWORD oldProtect;
                // Change memory protection to allow writing
                if (VirtualProtect((void*)forceJumpAddr, sizeof(int), PAGE_EXECUTE_READWRITE, &oldProtect))
                {
                    // Toggle jump state
                    if (!jumpState)
                    {
                        Sleep(10);
                        // Write 65537 to force jump
                        *(int*)forceJumpAddr = 65537;
                        jumpState = true;
                    }
                    else
                    {
                        Sleep(10);
                        // Write 256 to reset
                        *(int*)forceJumpAddr = 256;
                        jumpState = false;
                    }
                    
                    // Restore original protection
                    VirtualProtect((void*)forceJumpAddr, sizeof(int), oldProtect, &oldProtect);
                }
            }
        }
        __except (EXCEPTION_EXECUTE_HANDLER)
        {
            return;
        }
    }

    void Thirdperson()
    {
        // Disabled for now - requires proper camera service implementation
        // CS2 thirdperson needs CCSPlayerController -> CameraServices offset
        return;
    }

    void Run()
    {
        Bunnyhop();
        Thirdperson();
    }
}
