// Generated using https://github.com/a2x/cs2-dumper
// 2025-11-20 01:33:09.903018400 UTC

#pragma once
#include <cstddef>

// Client.dll offsets
namespace Offsets {
    // Module base addresses
    constexpr std::ptrdiff_t dwEntityList = 0x1D11CF8;
    constexpr std::ptrdiff_t dwLocalPlayerPawn = 0x1BECF38;
    constexpr std::ptrdiff_t dwViewAngles = 0x1E3A7F0;
    constexpr std::ptrdiff_t dwViewMatrix = 0x1E303D0;
    constexpr std::ptrdiff_t dwLocalPlayerController = 0x1E1BBD8;
    constexpr std::ptrdiff_t dwForceJump = 0x186CD60; // From buttons.hpp for bhop
    
    // Entity offsets
    constexpr std::ptrdiff_t m_iHealth = 0x344;
    constexpr std::ptrdiff_t m_iTeamNum = 0x3E3;
    constexpr std::ptrdiff_t m_vOldOrigin = 0x131C;
    constexpr std::ptrdiff_t m_pGameSceneNode = 0x328;
    constexpr std::ptrdiff_t m_vecAbsOrigin = 0xD0;
    constexpr std::ptrdiff_t m_modelState = 0x170;
    constexpr std::ptrdiff_t m_vecViewOffset = 0xCB8;
    constexpr std::ptrdiff_t m_angEyeAngles = 0x1590;
    constexpr std::ptrdiff_t m_hPlayerPawn = 0x80C;
    constexpr std::ptrdiff_t m_iszPlayerName = 0x660;
    constexpr std::ptrdiff_t m_iIDEntIndex = 0x1458;
    constexpr std::ptrdiff_t m_fFlags = 0x3EC;
    constexpr std::ptrdiff_t m_vecVelocity = 0x414;
    
    // Bones
    constexpr std::ptrdiff_t m_boneArray = 0x1E0;
}
