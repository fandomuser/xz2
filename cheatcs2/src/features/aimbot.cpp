#include "pch.h"
#include "aimbot.h"
#include "../config.h"
#include "../offsets.h"
#include <cmath>

struct Vector3 {
    float x, y, z;

    Vector3() : x(0), y(0), z(0) {}
    Vector3(float x, float y, float z) : x(x), y(y), z(z) {}

    Vector3 operator-(const Vector3& other) const {
        return Vector3(x - other.x, y - other.y, z - other.z);
    }

    float Length() const {
        return sqrtf(x * x + y * y + z * z);
    }
};

struct Vector2 {
    float x, y;

    Vector2() : x(0), y(0) {}
    Vector2(float x, float y) : x(x), y(y) {}

    Vector2 operator-(const Vector2& other) const {
        return Vector2(x - other.x, y - other.y);
    }

    float Length() const {
        return sqrtf(x * x + y * y);
    }
};

Vector2 CalcAngle(const Vector3& src, const Vector3& dst)
{
    Vector3 delta = dst - src;
    float hyp = sqrtf(delta.x * delta.x + delta.y * delta.y);

    Vector2 angles;
    angles.x = atan2f(-delta.z, hyp) * (180.0f / 3.14159265f);
    angles.y = atan2f(delta.y, delta.x) * (180.0f / 3.14159265f);

    return angles;
}

float GetFov(const Vector2& viewAngles, const Vector2& aimAngles)
{
    Vector2 delta = aimAngles - viewAngles;
    return delta.Length();
}

namespace Aimbot
{
    void Run()
    {
        if (!g_Config.aimbot_enabled) return;

        // Only aim when mouse1 is held
        if (!(GetAsyncKeyState(VK_LBUTTON) & 0x8000)) return;

        // Get client.dll base address
        HMODULE clientDll = GetModuleHandleA("client.dll");
        if (!clientDll) return;

        // Get local player controller
        uintptr_t localPlayerController = *(uintptr_t*)((uintptr_t)clientDll + Offsets::dwLocalPlayerController);
        if (!localPlayerController) return;

        // Get local player pawn
        uintptr_t localPlayerPawn = *(uintptr_t*)((uintptr_t)clientDll + Offsets::dwLocalPlayerPawn);
        if (!localPlayerPawn) return;

        // Get local team
        int localTeam = *(int*)(localPlayerController + Offsets::m_iTeamNum);

        // Get local eye position
        uintptr_t gameSceneNode = *(uintptr_t*)(localPlayerPawn + Offsets::m_pGameSceneNode);
        if (!gameSceneNode) return;

        Vector3 localPos = *(Vector3*)(gameSceneNode + Offsets::m_vecAbsOrigin);
        Vector3 viewOffset = *(Vector3*)(localPlayerPawn + Offsets::m_vecViewOffset);
        Vector3 localEyePos = Vector3(localPos.x + viewOffset.x, localPos.y + viewOffset.y, localPos.z + viewOffset.z);

        // Get current view angles
        uintptr_t viewAnglesAddr = (uintptr_t)clientDll + Offsets::dwViewAngles;
        Vector2 currentAngles = *(Vector2*)viewAnglesAddr;

        // Get entity list
        uintptr_t entityList = *(uintptr_t*)((uintptr_t)clientDll + Offsets::dwEntityList);
        if (!entityList) return;

        float bestFov = g_Config.aimbot_fov;
        Vector2 bestAngles = currentAngles;
        bool foundTarget = false;

        // Loop through entities
        for (int i = 1; i < 64; i++)
        {
            // Get entity controller
            uintptr_t entityController = *(uintptr_t*)(entityList + (i * 0x78));
            if (!entityController) continue;

            // Get player pawn from controller
            uintptr_t playerPawn = *(uintptr_t*)(entityController + Offsets::m_hPlayerPawn);
            if (!playerPawn || playerPawn == localPlayerPawn) continue;

            // Get team
            int team = *(int*)(playerPawn + Offsets::m_iTeamNum);
            
            // Skip teammates
            if (team == localTeam) continue;

            // Get health
            int health = *(int*)(playerPawn + Offsets::m_iHealth);
            if (health <= 0 || health > 100) continue;

            // Get head position
            uintptr_t enemySceneNode = *(uintptr_t*)(playerPawn + Offsets::m_pGameSceneNode);
            if (!enemySceneNode) continue;

            Vector3 enemyPos = *(Vector3*)(enemySceneNode + Offsets::m_vecAbsOrigin);
            Vector3 enemyHead = enemyPos;
            enemyHead.z += 65.0f; // Target head

            // Calculate angle to target
            Vector2 aimAngles = CalcAngle(localEyePos, enemyHead);

            // Calculate FOV
            float fov = GetFov(currentAngles, aimAngles);

            if (fov < bestFov)
            {
                bestFov = fov;
                bestAngles = aimAngles;
                foundTarget = true;
            }
        }

        if (foundTarget)
        {
            // Apply smooth
            Vector2 delta = bestAngles - currentAngles;
            delta.x /= g_Config.aimbot_smooth;
            delta.y /= g_Config.aimbot_smooth;

            Vector2 newAngles = Vector2(currentAngles.x + delta.x, currentAngles.y + delta.y);

            // Normalize angles
            while (newAngles.y > 180.0f) newAngles.y -= 360.0f;
            while (newAngles.y < -180.0f) newAngles.y += 360.0f;
            
            if (newAngles.x > 89.0f) newAngles.x = 89.0f;
            if (newAngles.x < -89.0f) newAngles.x = -89.0f;

            // Set view angles
            if (!g_Config.aimbot_silent)
            {
                *(Vector2*)viewAnglesAddr = newAngles;
            }
        }
    }
}
