#include "pch.h"
#include "gui.h"
#include "config.h"

namespace GUI
{
    void Render()
    {
        ImGui::Begin("CS2 Cheat", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize);
        ImGui::SetWindowSize(ImVec2(600, 400));

        if (ImGui::BeginTabBar("CheatTabs"))
        {
            if (ImGui::BeginTabItem("Aimbot"))
            {
                ImGui::Checkbox("Enable Aimbot", &g_Config.aimbot_enabled);
                ImGui::Checkbox("Silent Aim", &g_Config.aimbot_silent);
                ImGui::SliderFloat("FOV", &g_Config.aimbot_fov, 0.0f, 180.0f);
                ImGui::SliderFloat("Smooth", &g_Config.aimbot_smooth, 1.0f, 20.0f);
                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("Visuals"))
            {
                ImGui::Checkbox("Enable Visuals", &g_Config.visuals_enabled);
                ImGui::Checkbox("Box ESP", &g_Config.visuals_box);
                ImGui::Checkbox("Name ESP", &g_Config.visuals_name);
                ImGui::Checkbox("Health ESP", &g_Config.visuals_health);
                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("Misc"))
            {
                ImGui::Checkbox("Bunnyhop", &g_Config.misc_bhop);
                ImGui::Checkbox("Thirdperson", &g_Config.misc_thirdperson);
                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("Configs"))
            {
                ImGui::Text("Config Manager");
                ImGui::EndTabItem();
            }
            ImGui::EndTabBar();
        }

        ImGui::End();
    }
}
