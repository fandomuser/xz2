#pragma once

struct Config
{
    // Aimbot
    bool aimbot_enabled = false;
    bool aimbot_silent = false;
    float aimbot_fov = 5.0f;
    float aimbot_smooth = 2.0f;

    // Visuals
    bool visuals_enabled = false;
    bool visuals_box = false;
    bool visuals_name = false;
    bool visuals_health = false;

    // Misc
    bool misc_bhop = false;
    bool misc_thirdperson = false;
};

extern Config g_Config;
