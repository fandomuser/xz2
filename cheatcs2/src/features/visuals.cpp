#include "pch.h"
#include "visuals.h"
#include "../config.h"
#include "../offsets.h"
#include <imgui.h>
#include <cstdio>

struct ViewMatrix_t {
    float matrix[4][4];
};

struct Vector3 {
    float x, y, z;
};

bool WorldToScreen(const Vector3& world, Vector3& screen, const ViewMatrix_t& viewMatrix, int screenWidth, int screenHeight)
{
    float w = viewMatrix.matrix[3][0] * world.x + viewMatrix.matrix[3][1] * world.y + viewMatrix.matrix[3][2] * world.z + viewMatrix.matrix[3][3];
    
    if (w < 0.001f)
        return false;

    float x = viewMatrix.matrix[0][0] * world.x + viewMatrix.matrix[0][1] * world.y + viewMatrix.matrix[0][2] * world.z + viewMatrix.matrix[0][3];
    float y = viewMatrix.matrix[1][0] * world.x + viewMatrix.matrix[1][1] * world.y + viewMatrix.matrix[1][2] * world.z + viewMatrix.matrix[1][3];

    screen.x = (screenWidth / 2.0f) + (screenWidth / 2.0f) * x / w;
    screen.y = (screenHeight / 2.0f) - (screenHeight / 2.0f) * y / w;
    screen.z = w;

    return true;
}

namespace Visuals
{
    void DrawESP()
    {
        if (!g_Config.visuals_enabled) return;

        // Get client.dll base address
        HMODULE clientDll = GetModuleHandleA("client.dll");
        if (!clientDll) return;

        __try
        {
            // Get view matrix with safety check
            uintptr_t viewMatrixAddr = (uintptr_t)clientDll + Offsets::dwViewMatrix;
            if (IsBadReadPtr((void*)viewMatrixAddr, sizeof(ViewMatrix_t))) return;
            
            ViewMatrix_t viewMatrix = *(ViewMatrix_t*)viewMatrixAddr;

            // Get screen size from ImGui
            ImGuiIO& io = ImGui::GetIO();
            int screenWidth = (int)io.DisplaySize.x;
            int screenHeight = (int)io.DisplaySize.y;

            // Get local player pawn
            uintptr_t localPlayerPawnAddr = (uintptr_t)clientDll + Offsets::dwLocalPlayerPawn;
            if (IsBadReadPtr((void*)localPlayerPawnAddr, sizeof(uintptr_t))) return;
            
            uintptr_t localPlayerPawn = *(uintptr_t*)localPlayerPawnAddr;
            if (!localPlayerPawn) return;

            // Get local team with safety check
            if (IsBadReadPtr((void*)(localPlayerPawn + Offsets::m_iTeamNum), sizeof(int))) return;
            int localTeam = *(int*)(localPlayerPawn + Offsets::m_iTeamNum);

            // Get entity list with safety check
            uintptr_t entityListAddr = (uintptr_t)clientDll + Offsets::dwEntityList;
            if (IsBadReadPtr((void*)entityListAddr, sizeof(uintptr_t))) return;
            
            uintptr_t entityList = *(uintptr_t*)entityListAddr;
            if (!entityList) return;

            ImDrawList* drawList = ImGui::GetBackgroundDrawList();

            // Loop through entities with safety
            for (int i = 1; i < 32; i++) // Reduced to 32 for safety
            {
                uintptr_t listEntryAddr = entityList + (8 * (i & 0x7FFF) >> 9) + 16;
                if (IsBadReadPtr((void*)listEntryAddr, sizeof(uintptr_t))) continue;
                
                uintptr_t listEntry = *(uintptr_t*)listEntryAddr;
                if (!listEntry) continue;

                uintptr_t entityAddr = listEntry + 120 * (i & 0x1FF);
                if (IsBadReadPtr((void*)entityAddr, sizeof(uintptr_t))) continue;
                
                uintptr_t entity = *(uintptr_t*)entityAddr;
                if (!entity || entity == localPlayerPawn) continue;

                // Safety check for team
                if (IsBadReadPtr((void*)(entity + Offsets::m_iTeamNum), sizeof(int))) continue;
                int team = *(int*)(entity + Offsets::m_iTeamNum);
                
                // Skip teammates
                if (team == localTeam) continue;

                // Safety check for health
                if (IsBadReadPtr((void*)(entity + Offsets::m_iHealth), sizeof(int))) continue;
                int health = *(int*)(entity + Offsets::m_iHealth);
                if (health <= 0 || health > 100) continue;

                // Get position with safety
                if (IsBadReadPtr((void*)(entity + Offsets::m_pGameSceneNode), sizeof(uintptr_t))) continue;
                uintptr_t gameSceneNode = *(uintptr_t*)(entity + Offsets::m_pGameSceneNode);
                if (!gameSceneNode) continue;

                if (IsBadReadPtr((void*)(gameSceneNode + Offsets::m_vecAbsOrigin), sizeof(Vector3))) continue;
                Vector3 origin = *(Vector3*)(gameSceneNode + Offsets::m_vecAbsOrigin);
                
                Vector3 head = origin;
                head.z += 75.0f;

                Vector3 screenPos, screenHead;
                if (!WorldToScreen(origin, screenPos, viewMatrix, screenWidth, screenHeight)) continue;
                if (!WorldToScreen(head, screenHead, viewMatrix, screenWidth, screenHeight)) continue;

                float height = screenPos.y - screenHead.y;
                float width = height / 2.0f;

                // Draw box ESP
                if (g_Config.visuals_box)
                {
                    drawList->AddRect(
                        ImVec2(screenHead.x - width / 2, screenHead.y),
                        ImVec2(screenHead.x + width / 2, screenPos.y),
                        IM_COL32(255, 0, 0, 255),
                        0.0f,
                        0,
                        1.5f
                    );
                }

                // Draw health ESP
                if (g_Config.visuals_health)
                {
                    char healthText[32];
                    sprintf_s(healthText, "%d HP", health);
                    drawList->AddText(
                        ImVec2(screenHead.x + width / 2 + 5, screenHead.y),
                        IM_COL32(0, 255, 0, 255),
                        healthText
                    );
                }
            }
        }
        __except (EXCEPTION_EXECUTE_HANDLER)
        {
            // Silently catch any access violations
            return;
        }
    }

    void Run()
    {
        DrawESP();
    }
}
