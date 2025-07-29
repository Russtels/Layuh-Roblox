#pragma once
#define d_toggle_bind 0x2D /* VK_INSERT */

namespace overlay
{
    inline void setup_style()
    {

        ImGui::StyleColorsDark();
        ImGuiStyle* style = &ImGui::GetStyle();
        style->Colors[ImGuiCol_WindowBg] = ImColor(10, 10, 10, 255);
        style->Colors[ImGuiCol_Border] = ImColor(45, 45, 45, 255);
        style->Colors[ImGuiCol_Separator] = ImColor(133, 3, 35);
        style->Colors[ImGuiCol_SeparatorHovered] = ImColor(133, 3, 35);
        style->Colors[ImGuiCol_SeparatorActive] = ImColor(133, 3, 35);
        style->Colors[ImGuiCol_ChildBg] = ImColor(15, 15, 15, 255);
        style->Colors[ImGuiCol_FrameBg] = ImColor(25, 25, 25, 255);
        style->Colors[ImGuiCol_PopupBg] = ImColor(25, 25, 25, 255);
        style->Colors[ImGuiCol_Header] = ImColor(0, 0, 0, 0);
        style->Colors[ImGuiCol_HeaderHovered] = ImColor(0, 0, 0, 0);
        style->Colors[ImGuiCol_HeaderActive] = ImColor(0, 0, 0, 0);
        style->Colors[ImGuiCol_Button] = ImColor(25, 25, 25, 255);
        style->Colors[ImGuiCol_ButtonActive] = ImColor(25, 25, 25, 255);
        style->Colors[ImGuiCol_ButtonHovered] = ImColor(25, 25, 25, 255);
        style->Colors[ImGuiCol_CheckMark] = ImColor(133, 3, 35);
        style->Colors[ImGuiCol_SliderGrab] = ImColor(133, 3, 35);
        style->WindowTitleAlign = { 0.5f, 0.5f };
        style->WindowRounding = 0.0f;
        style->ChildRounding = 0.0f;
        style->FrameRounding = 0.0f;
        style->WindowBorderSize = 1.0f;
        style->FrameBorderSize = 1.0f;
        style->ChildBorderSize = 1.0f;
        style->AntiAliasedFill = false;
        style->AntiAliasedLines = false;
        style->Alpha = 1.0f;
        

    }


	void render();

	inline bool enabled = true;
}