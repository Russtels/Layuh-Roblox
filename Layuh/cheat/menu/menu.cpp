#include "menu.h"
#include "imgui.h"
#include "imgui_impl_dx9.h"
#include "imgui_impl_win32.h"
#include "custom.hpp"
#include "images.hpp"
#include "fonts.hpp"
#include <dwmapi.h>
#include <d3d9.h>
#include "d3dx9tex.h"
#include <tchar.h>
#include "map"
#include "imgui/bytes.h"
#include <nlohmann/json.hpp>
#include <iostream>
#include <string>
#include <vector>
#include <WinHttp.h>
#include <sstream>
#include <thread>
#include <unordered_map>
#include <unordered_set>
#include <mutex>
#include <atomic>
#include <chrono>
#include <random>
#include <fstream>
#include <windows.h>
#include "entry.h"
#include "cheat/menu/ui/render/drawing/image.h"
#include "../zlib/zlib.h"
#include <condition_variable>
#include <queue>
#include <set>
#pragma comment(lib, "dwmapi.lib")
#pragma comment(lib, "d3dx9.lib")
#pragma comment(lib, "winhttp.lib")
#pragma warning(disable : C4267)
#pragma warning(disable : C4244)


static char prediction_factor_buf[32] = "";
static float last_prediction_factor = 0.0f;
bool use_displayname = false;

static ImVec2 watermark_pos(10, 10);
static bool is_dragging = false;
static ImVec2 drag_offset(0, 0);
using json = nlohmann::json;

inline nlohmann::json color_to_json(const ImColor& c) {
    return { c.Value.x, c.Value.y, c.Value.z, c.Value.w };
}
inline ImColor color_from_json(const nlohmann::json& j) {
    if (j.is_array() && j.size() == 4)
        return ImColor(j[0].get<float>(), j[1].get<float>(), j[2].get<float>(), j[3].get<float>());
    return ImColor(1.f, 1.f, 1.f, 1.f);
}

bool copy_to_clipboard(const std::string& text) {
    if (!OpenClipboard(nullptr))
        return false;

    EmptyClipboard();

    size_t size = text.size() + 1;
    HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, size);
    if (!hMem) {
        CloseClipboard();
        return false;
    }

    memcpy(GlobalLock(hMem), text.c_str(), size);
    GlobalUnlock(hMem);

    if (!SetClipboardData(CF_TEXT, hMem)) {
        CloseClipboard();
        GlobalFree(hMem);
        return false;
    }

    CloseClipboard();
    return true;
}

LPDIRECT3DTEXTURE9 Layuh_preview = nullptr;
LPDIRECT3DTEXTURE9 Layuh_preview_skeleton = nullptr;

LPDIRECT3DTEXTURE9 LoadTextureFromMemory2(LPDIRECT3DDEVICE9 pDevice, const std::vector<uint8_t>& data) {
    if (data.empty()) return nullptr;
    LPDIRECT3DTEXTURE9 texture = nullptr;
    D3DXCreateTextureFromFileInMemory(pDevice, data.data(), data.size(), &texture);
    return texture;
}

std::string get_local_player_name()
{
    if (!globals::local_player)
        return oxorany("Unknown");

    std::string player_name = utils::get_instance_name(globals::local_player);
    return player_name.empty() ? oxorany("Unknown") : player_name;
}


std::string readstring2(std::uint64_t address)
{
    std::string string;
    char character = 0;
    int char_size = sizeof(character);
    int offset = 0;

    string.reserve(204);

    while (offset < 200)
    {
        character = read<char>(address + offset);

        if (character == 0)
            break;

        offset += char_size;
        string.push_back(character);
    }

    return string;
}

std::string get_server_join_command() {
    if (!globals::datamodel)
        return oxorany("ERROR: DataModel not found");

    uintptr_t game_id = 0;
    if (offsets::PlaceId != 0) {
        game_id = read<uintptr_t>(globals::datamodel + offsets::PlaceId);
    }

    std::string instance_id;
    if (offsets::JobId != 0) {
        uintptr_t job_id_ptr = read<uintptr_t>(globals::datamodel + offsets::JobId);

        if (job_id_ptr) {
            size_t str_len = 0;
            if (offsets::StringLength != 0) {
                str_len = read<size_t>(job_id_ptr + offsets::StringLength);
            }

            instance_id = readstring2(job_id_ptr);

            instance_id.erase(
                std::remove_if(instance_id.begin(), instance_id.end(),
                    [](unsigned char c) { return !std::isprint(c); }),
                instance_id.end()
            );
        }
    }

    std::stringstream ss;
    if (game_id != 0) {
        if (!instance_id.empty())
            ss << oxorany("Roblox.GameLauncher.joinGameInstance(\"") << game_id << "\", \"" << instance_id << "\")";
        else
            ss << oxorany("Roblox.GameLauncher.joinGameInstance(\"") << game_id << "\", \"ERROR: Could not read JobId\")";
    }
    else {
        ss << oxorany("ERROR: Could not read game information");
    }

    return ss.str();
}

std::vector<std::string> get_server_list(uint64_t placeId, const std::string& currentJobId) {
    std::vector<std::string> jobIds;
    std::wstring host = oxorany(L"games.roblox.com");
    std::wstring path = oxorany(L"/v1/games/") + std::to_wstring(placeId) + oxorany(L"/servers/Public?sortOrder=Asc&limit=100");

    HINTERNET hSession = WinHttpOpen(L"ServerHop/1.0", WINHTTP_ACCESS_TYPE_DEFAULT_PROXY, NULL, NULL, 0);
    if (!hSession) return jobIds;
    HINTERNET hConnect = WinHttpConnect(hSession, host.c_str(), INTERNET_DEFAULT_HTTPS_PORT, 0);
    if (!hConnect) { WinHttpCloseHandle(hSession); return jobIds; }
    HINTERNET hRequest = WinHttpOpenRequest(hConnect, oxorany(L"GET"), path.c_str(), NULL, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, WINHTTP_FLAG_SECURE);
    if (!hRequest) { WinHttpCloseHandle(hConnect); WinHttpCloseHandle(hSession); return jobIds; }

    BOOL bResults = WinHttpSendRequest(hRequest, WINHTTP_NO_ADDITIONAL_HEADERS, 0, WINHTTP_NO_REQUEST_DATA, 0, 0, 0)
        && WinHttpReceiveResponse(hRequest, NULL);

    std::string response;
    if (bResults) {
        DWORD dwSize = 0;
        do {
            DWORD dwDownloaded = 0;
            WinHttpQueryDataAvailable(hRequest, &dwSize);
            if (!dwSize) break;
            std::vector<char> buffer(dwSize + 1);
            ZeroMemory(buffer.data(), dwSize + 1);
            WinHttpReadData(hRequest, buffer.data(), dwSize, &dwDownloaded);
            response.append(buffer.data(), dwDownloaded);
        } while (dwSize > 0);
    }

    WinHttpCloseHandle(hRequest);
    WinHttpCloseHandle(hConnect);
    WinHttpCloseHandle(hSession);

    try {
        auto json = nlohmann::json::parse(response);
        for (const auto& server : json["data"]) {
            std::string jobId = server["id"];
            if (jobId != currentJobId) {
                jobIds.push_back(jobId);
            }
        }
    }
    catch (...) {
    }

    return jobIds;
}

int current_tab = 0;
uintptr_t g_spectate_target = 0;

bool CMenu::draw_menu() {

    if (!Layuh_preview) {
        Layuh_preview = LoadTextureFromMemory2(dx_device, Layuh_bytes);
        Layuh_preview_skeleton = LoadTextureFromMemory2(dx_device, Layuh_skeleton_bytes);
    }

    ImGui::SetNextWindowSize({ 665, 650 });
    ImGui::Begin(oxorany("Kryptex External"), nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoTitleBar);
    {

        std::string player_name = get_local_player_name();
        ImGui::Text(oxorany("Kryptex Beta | https://discord.gg/StZUmwDQnJ |"));
        ImGui::Separator();

        if (ImGui::Button(oxorany(("Aimbot")), ImVec2(130, 25))) current_tab = 0;
        ImGui::SameLine(0, 0);
        if (ImGui::Button(oxorany(("Visuals")), ImVec2(130, 25))) current_tab = 1;
        ImGui::SameLine(0, 0);
        if (ImGui::Button(oxorany(("Exploits")), ImVec2(130, 25))) current_tab = 2;
        ImGui::SameLine(0, 0);
        if (ImGui::Button(oxorany(("Server List")), ImVec2(130, 25))) current_tab = 3;
        ImGui::SameLine(0, 0);
        if (ImGui::Button(oxorany(("Settings")), ImVec2(130, 25))) current_tab = 4;

        const float window_width = ImGui::GetContentRegionAvailWidth() - ImGui::GetStyle().WindowPadding.x;
        const float window_height = ImGui::GetContentRegionAvail().y - ImGui::GetStyle().WindowPadding.y;

        switch (current_tab) {
        case 0:
            imgui_custom->begin_child(oxorany(("Aimbot Configuration")), ImVec2(window_width * 0.5f, window_height));
            {

                ImGui::Checkbox(oxorany("Aimbot"), &vars::aimbot::aimbot_enabled);
                drawingapi::drawing.hotkey(oxorany("Key"), &vars::aimbot::aimbot_key, &vars::aimbot::aimbot_mode, ImVec2(80, 23));
                ImGui::Checkbox(oxorany("Fov Visible"), &vars::aimbot::fov_circle);
                ImGui::SameLine(105);
                ImGui::ColorEdit3(oxorany("Fov Color"), (float*)&vars::visuals::fov_circle_color, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel);
                ImGui::SliderFloat(oxorany("Fov Size"), &vars::aimbot::fov_value, 20, 1000, "%.1f");
                ImGui::Checkbox(oxorany("Tracer"), &vars::aimbot::target_line);
                ImGui::SameLine(105);
                ImGui::ColorEdit3(oxorany("Tracer Color"), (float*)&vars::aimbot::target_line_color, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel);
                ImGui::SliderFloat(oxorany("Smoothness"), &vars::aimbot::aimbot_smoothing, 0.0f, 19.9f, "%.1f");
                ImGui::Checkbox(oxorany("Prediction"), &vars::aimbot::prediction);
                //if (last_prediction_factor != vars::aimbot::prediction_factor) {
                //    snprintf(prediction_factor_buf, sizeof(prediction_factor_buf), "%.3f", vars::aimbot::prediction_factor);
                //    last_prediction_factor = vars::aimbot::prediction_factor;
                //}

                //if (ImGui::InputText(oxorany("Prediction Factor"), prediction_factor_buf, sizeof(prediction_factor_buf), ImGuiInputTextFlags_CharsDecimal)) {
                //    float new_value = std::clamp(static_cast<float>(atof(prediction_factor_buf)), 0.001f, 5.0f);
                //    vars::aimbot::prediction_factor = new_value;
                //    last_prediction_factor = new_value;
                //}
                if (vars::aimbot::prediction) {
			    ImGui::SliderFloat(oxorany("Prediction Factor"), &vars::aimbot::prediction_factor, 0.001f, 5.0f, "%.3f");
                ImGui::Checkbox(oxorany("Prediction Distance Based"), &vars::aimbot::prediction_distance_based);
                if (vars::aimbot::prediction_distance_based) {
                    ImGui::SliderFloat(oxorany("Minimum Prediction Start Distance"), &vars::aimbot::min_pred_distance, 1.0f, 50.0f, "%.1f");
                }
                }
                const char* aimbot_options[] = { "Mouse", "Camera", "Free Aim (Use Aimbot Toggle)", "Silent (THANKS Arsky)", "None" };
                ImGui::Combo(oxorany("Aimbot Method"), &vars::aimbot::aimbot_method, aimbot_options, IM_ARRAYSIZE(aimbot_options));
                ImGui::PushItemWidth(135);
                ImGui::SliderFloat(oxorany("Max Distance"), &vars::aimbot::max_distance, 5, 10000, "%.f");
                ImGui::PopItemWidth();
                ImGui::Text(oxorany("Aimbot Targetting"));
                const char* hitbox_options[] = { "Head", "Body", "Random" };
                ImGui::Combo(oxorany("Target Hitbox"), &vars::aimbot::target_hitbox, hitbox_options, IM_ARRAYSIZE(hitbox_options));



            }
            imgui_custom->end_child();

            ImGui::SameLine();

            imgui_custom->begin_child(oxorany(("Other Aimbot Configuration")), ImVec2(window_width * 0.5f, window_height));
            {
                ImGui::Checkbox(oxorany("Team Check"), &vars::aimbot::team_check);
                ImGui::Checkbox(oxorany("Knock Check (Dahood)"), &vars::aimbot::knock_check);
                ImGui::Checkbox(oxorany("Grabbed Check (Dahood)"), &vars::aimbot::grab_check);
                ImGui::Checkbox(oxorany("Arsenal Flick Fix"), &vars::aimbot::arsenal_tp_fix);
                ImGui::Spacing();
                ImGui::Separator();
                ImGui::Spacing();
				ImGui::Text(oxorany("Triggerbot"));
				ImGui::Checkbox(oxorany("Enabled"), &vars::aimbot::triggerbot);
                ImGui::SliderFloat(oxorany("Max Distance"), &vars::aimbot::triggerbot_max_distance, 1.0f, 300.0f, "%.1f");
				ImGui::SliderFloat(oxorany("Delay (ms)"), &vars::aimbot::triggerbot_delay, 1.0f, 500.0f, "%.1f");
                drawingapi::drawing.hotkey(oxorany("Key"), &vars::aimbot::triggerbot_key, &vars::aimbot::triggerbot_key_mode, ImVec2(80, 23));
                ImGui::Spacing();
                ImGui::Separator();
                ImGui::Spacing();
                ImGui::Text(oxorany("HvH Functions"));
                ImGui::Checkbox(oxorany("Orbit"), &vars::aimbot::circle_target::enabled);
                ImGui::SliderFloat(oxorany("Speed"), &vars::aimbot::circle_target::speed, 0, 360, "%.1f");
                ImGui::SliderFloat(oxorany("Radius"), &vars::aimbot::circle_target::radius, 0, 30, "%.1f");
                ImGui::SliderFloat(oxorany("Height"), &vars::aimbot::circle_target::height_offset, 0, 50, "%.1f");
                ImGui::Checkbox(oxorany("Return to Position"), &vars::aimbot::circle_target::teleport_back);
            }
            imgui_custom->end_child();
            break;

        case 1:
            imgui_custom->begin_child(oxorany(("Visual Configuration")), ImVec2(window_width / 2, window_height));
            {
                ImGui::Checkbox(oxorany("Enabled"), &vars::esp::esp_enabled);
                ImGui::Checkbox(oxorany("Box"), &vars::esp::esp_box);
                ImGui::SameLine(105);
                ImGui::ColorEdit3(oxorany("Box Color"), (float*)&vars::esp::box_color, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel);

                ImGui::Checkbox(oxorany("Skeleton"), &vars::esp::esp_skeleton);
                ImGui::SameLine(105);
                ImGui::ColorEdit3(oxorany("Skeleton Color"), (float*)&vars::esp::skeleton_color, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel);
                ImGui::SliderFloat(oxorany("Skeleton Thickness"), &vars::esp::esp_skeleton_thickness, 0.1f, 5.0f, "%.1f");

                ImGui::Checkbox(oxorany("Fill Box"), &vars::esp::esp_fill_box);
                ImGui::SameLine(105);
                ImGui::ColorEdit3(oxorany("Fill Box Color"), (float*)&vars::esp::fill_box_color, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel);

                ImGui::Checkbox(oxorany("Name"), &vars::esp::esp_name);
                ImGui::SameLine(105);
                ImGui::ColorEdit3(oxorany("Name Color"), (float*)&vars::esp::name_color, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel);

                ImGui::Checkbox(oxorany("Health Bar"), &vars::esp::esp_health_bar);
                ImGui::SameLine(105);
                ImGui::ColorEdit3(oxorany("Health Bar Color"), (float*)&vars::esp::health_bar_color, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel);
                
                ImGui::Checkbox(oxorany("Health Text"), &vars::esp::esp_health_text);
                ImGui::SameLine(105);
                ImGui::ColorEdit3(oxorany("Health Text Color"), (float*)&vars::esp::health_text_color, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel);

                ImGui::Checkbox(oxorany("Distance"), &vars::esp::esp_distance);
                ImGui::SameLine(105);
                ImGui::ColorEdit3(oxorany("Distance Color"), (float*)&vars::esp::distance_color, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel);

                ImGui::Checkbox(oxorany("Tracers"), &vars::esp::esp_tracer);
                ImGui::SameLine(105);
                ImGui::ColorEdit3(oxorany("Tracer Color"), (float*)&vars::esp::tracer_color, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel);
                ImGui::SliderFloat(oxorany("Tracer Thickness"), &vars::esp::tracer_thickness, 0.1f, 5.0f, "%.1f");

                ImGui::Checkbox(oxorany("OOF Arrows"), &vars::esp::esp_oof_arrows);
                ImGui::SameLine(105);
                ImGui::ColorEdit3(oxorany("OOF Arrows Color"), (float*)&vars::esp::oof_arrow_color, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel);

            }
            imgui_custom->end_child();

            ImGui::SameLine();

            ImGui::BeginGroup();
            {
                imgui_custom->begin_child(oxorany(("Extra Visual Configuration")), ImVec2(window_width / 2, window_height / 2 - 4));
                {
                    ImGui::Checkbox(oxorany("Show Esp Preview"), &vars::esp::esp_preview);
                    ImGui::Checkbox(oxorany("Show LocalPlayer"), &vars::esp::esp_local);
                    ImGui::Checkbox(oxorany("Team Check"), &vars::esp::team_check);

					ImGui::Spacing();
					ImGui::Separator();
					ImGui::Spacing();

                    //ImGui::Checkbox(oxorany("Box Glow"), &vars::esp::box_glow);
                    const char* tracer_start_mode[] = { "Top", "Middle", "Bottom" };
                    const char* tracer_end_mode[] = { "Top", "Middle" };
                    ImGui::Combo(oxorany("Tracer Start"), &vars::esp::tracer_start, tracer_start_mode, IM_ARRAYSIZE(tracer_start_mode));
                    ImGui::Combo(oxorany("Tracer End"), &vars::esp::tracer_end, tracer_end_mode, IM_ARRAYSIZE(tracer_end_mode));
                
                }
                imgui_custom->end_child();

                ImGui::Spacing();

                imgui_custom->begin_child(oxorany(("Radar Configuration")), ImVec2(window_width / 2, window_height / 2 - 4));
                {
                    ImGui::Checkbox(oxorany("Radar"), &vars::esp::esp_radar);
                    ImGui::SameLine(125);
                    ImGui::ColorEdit3(oxorany("Radar Color"), (float*)&vars::esp::radar_color, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel);
                    ImGui::Checkbox(oxorany("Show Names"), &vars::esp::radar_show_names);
                    ImGui::SameLine(125);
                    ImGui::ColorEdit3(oxorany("Radar Names Color"), (float*)&vars::esp::radar_names_color, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel);
                    ImGui::Checkbox(oxorany("Show Distance"), &vars::esp::radar_show_distance);
                    ImGui::SameLine(125);
                    ImGui::ColorEdit3(oxorany("Distance Color"), (float*)&vars::esp::radar_distance_color, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel);
                }
                imgui_custom->end_child();
            }
            ImGui::EndGroup();

            if (vars::esp::esp_preview) {
            ImGui::SetNextWindowPos(ImVec2(ImGui::GetWindowPos().x + ImGui::GetWindowSize().x + 5.0f, ImGui::GetWindowPos().y + 25.0f));
            ImGui::SetNextWindowSize({ window_width * 0.5f, window_height - 100 });
            ImGui::Begin(oxorany(("Preview")), nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);
            {
                ImVec2 preview_center = ImVec2(ImGui::GetWindowPos().x + ImGui::GetWindowSize().x * 0.5f, ImGui::GetWindowPos().y + ImGui::GetWindowSize().y * 0.5f);
                ImGui::Text(oxorany("Esp Preview"));
                ImGui::Separator();

                if (Layuh_preview) {
                    ImVec2 texture_size = ImVec2(160, 300);
                    ImVec2 texture_pos = ImVec2(preview_center.x - texture_size.x * 0.5f, preview_center.y - texture_size.y * 0.5f);
                    ImGui::GetWindowDrawList()->AddImage(Layuh_preview, texture_pos, ImVec2(texture_pos.x + texture_size.x, texture_pos.y + texture_size.y));
                }
                if (vars::esp::esp_skeleton)
                {
                    ImVec2 texture_size = ImVec2(160, 300);
                    ImVec2 texture_pos = ImVec2(preview_center.x - texture_size.x * 0.5f, preview_center.y - texture_size.y * 0.5f);
                    ImGui::GetWindowDrawList()->AddImage(Layuh_preview_skeleton, texture_pos, ImVec2(texture_pos.x + texture_size.x, texture_pos.y + texture_size.y));
                }

                ImDrawList* drawList = ImGui::GetWindowDrawList();
                ImU32 boxColor = ImGui::GetColorU32(ImVec4(vars::esp::box_color));
                ImU32 fillBoxColor = ImGui::GetColorU32(ImVec4(vars::esp::fill_box_color));
                ImU32 healthBarColor = ImGui::GetColorU32(ImVec4(vars::esp::health_bar_color));
                ImU32 nameColor = ImGui::GetColorU32(ImVec4(vars::esp::name_color));
                ImU32 distanceColor = IM_COL32(200, 200, 200, 255);

                ImVec2 box_min = ImVec2(preview_center.x - 80, preview_center.y - 150);
                ImVec2 box_max = ImVec2(preview_center.x + 80, preview_center.y + 150);

                if (vars::esp::esp_fill_box) {
                    drawList->AddRectFilled(box_min, box_max, fillBoxColor, 0.0f);
                }
                if (vars::esp::esp_box) {
                    drawList->AddRect(box_min, box_max, boxColor, 0.0f, 0, 2.0f);
                }

                if (vars::esp::esp_name) {
                    const char* userName = oxorany("Jugg");
                    ImVec2 textSize = ImGui::CalcTextSize(userName);
                    ImVec2 text_pos = ImVec2(preview_center.x - textSize.x * 0.5f, box_min.y - textSize.y - 6);
                    drawList->AddText(text_pos, nameColor, userName);
                }

                static float anim_time = 0.0f;
                anim_time += ImGui::GetIO().DeltaTime;
                float health_anim = (sinf(anim_time * 1.5f) * 0.5f + 0.5f);
                float health_value = 100.0f * health_anim;
                float health_bar_height = (box_max.y - box_min.y) * health_anim;

                if (vars::esp::esp_health_bar) {
                    ImVec2 bar_min = ImVec2(box_min.x - 10, box_max.y - health_bar_height);
                    ImVec2 bar_max = ImVec2(box_min.x - 4, box_max.y);
                    drawList->AddRectFilled(ImVec2(bar_min.x, box_min.y), ImVec2(bar_max.x, box_max.y), IM_COL32(40, 40, 40, 180), 2.0f);
                    drawList->AddRectFilled(bar_min, bar_max, healthBarColor, 2.0f);
                    drawList->AddRect(ImVec2(bar_min.x, box_min.y), ImVec2(bar_max.x, box_max.y), IM_COL32(0, 0, 0, 255), 2.0f);
                }

                if (vars::esp::esp_health_text) {
                    char health_text[16];
                    snprintf(health_text, sizeof(health_text), oxorany("%.0f HP"), health_value);
                    ImVec2 textSize = ImGui::CalcTextSize(health_text);
                    ImVec2 text_pos = ImVec2(box_min.x - 10 - textSize.x - 4, box_max.y - health_bar_height - textSize.y * 0.5f);
                    drawList->AddText(text_pos, vars::esp::health_text_color, health_text);
                }

                if (vars::esp::esp_distance) {
                    float fake_distance = 10.0f + 90.0f * (1.0f - health_anim);
                    char dist_text[32];
                    snprintf(dist_text, sizeof(dist_text), oxorany("%.1f m"), fake_distance);
                    ImVec2 textSize = ImGui::CalcTextSize(dist_text);
                    ImVec2 text_pos = ImVec2(preview_center.x - textSize.x * 0.5f, box_max.y + 8);
                    drawList->AddText(text_pos, vars::esp::distance_color, dist_text);
                }
            }
            ImGui::End();
            }
            break;

        case 2:
            imgui_custom->begin_child(oxorany(("Trolling Configuration")), ImVec2(window_width * 0.5f, window_height));
            {
                ImGui::Checkbox(oxorany("Have Sex with Aimbot Target"), &vars::misc::sex_enabled);
                const char* sex_modes[] = { "Doggy", "Blowjob", "Reverse Doggy", "Reverse Blowjob" };
                ImGui::Combo(oxorany("Sex Mode"), &vars::misc::sex_mode, sex_modes, IM_ARRAYSIZE(sex_modes));
                ImGui::SliderFloat(oxorany("Sex Distance"), &vars::misc::sex_distance, 0, 15, "%.1f");
                ImGui::SliderFloat(oxorany("Sex Height"), &vars::misc::sex_height, 0, 5, "%.1f");
                ImGui::SliderFloat(oxorany("Sex Speed"), &vars::misc::sex_speed, 0.1, 20, "%.1f");
                ImGui::Spacing();
                ImGui::Separator();
                ImGui::Spacing();
                ImGui::Text(oxorany("Fun Configuration"));
				ImGui::Checkbox(oxorany("Play Mines"), &vars::misc::mines_gamble);
                ImGui::Checkbox(oxorany("Play Slots"), &vars::misc::slots_gamle);
                ImGui::Checkbox(oxorany("Play Blackjack"), &vars::misc::blackjack_gamble);

                if (vars::misc::mines_gamble) {
                    static ImVec2 mines_window_pos(300, 300);
                    static ImVec2 mines_window_size(400, 460);
                    static std::vector<std::vector<bool>> mines_field(5, std::vector<bool>(5, false));
                    static std::vector<std::vector<bool>> revealed_cells(5, std::vector<bool>(5, false));
                    static int mines_count = 5;
                    static bool game_over = false;
                    static bool game_won = false;
                    static int countdown = 5;
                    static float last_countdown_time = 0.0f;
                    static int safe_cells_remaining = 25 - mines_count;
                    static bool need_reset = true;

                    auto reset_game = [&]() {
                        for (int i = 0; i < 5; i++) {
                            for (int j = 0; j < 5; j++) {
                                mines_field[i][j] = false;
                                revealed_cells[i][j] = false;
                            }
                        }

                        std::random_device rd;
                        std::mt19937 gen(rd());
                        std::uniform_int_distribution<int> row_dist(0, 4);
                        std::uniform_int_distribution<int> col_dist(0, 4);

                        int placed_mines = 0;
                        while (placed_mines < mines_count) {
                            int row = row_dist(gen);
                            int col = col_dist(gen);

                            if (!mines_field[row][col]) {
                                mines_field[row][col] = true;
                                placed_mines++;
                            }
                        }

                        game_over = false;
                        game_won = false;
                        safe_cells_remaining = 25 - mines_count;
                        need_reset = false;
                        };

                    ImGui::SetNextWindowPos(mines_window_pos, ImGuiCond_Once);
                    ImGui::SetNextWindowSize(mines_window_size);
                    ImGui::Begin(oxorany("Mines"), nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoTitleBar);
                    ImGui::Text(oxorany("Mines (inferno.rocks ^o^)"));
                    ImGui::Separator();
                    ImGui::Spacing();  
                    if (need_reset) {
                        reset_game();
                    }

                    if (ImGui::SliderInt(oxorany("Mine Count"), &mines_count, 1, 15)) {
                        need_reset = true;
                    }

                    ImGui::Spacing();
                    ImGui::Separator();
                    ImGui::Spacing();

                    float button_size = (mines_window_size.x - 50) / 5;

                    if (!game_over && !game_won) {
                        for (int i = 0; i < 5; i++) {
                            for (int j = 0; j < 5; j++) {
                                ImGui::PushID(i * 5 + j);
                                if (!revealed_cells[i][j]) {
                                    if (ImGui::Button(oxorany("Mine"), ImVec2(button_size, button_size))) {
                                        revealed_cells[i][j] = true;

                                        if (mines_field[i][j]) {
                                            game_over = true;
                                            for (int r = 0; r < 5; r++) {
                                                for (int c = 0; c < 5; c++) {
                                                    revealed_cells[r][c] = true;
                                                }
                                            }
                                        }
                                        else {
                                            safe_cells_remaining--;
                                            if (safe_cells_remaining <= 0) {
                                                game_won = true;
                                                for (int r = 0; r < 5; r++) {
                                                    for (int c = 0; c < 5; c++) {
                                                        revealed_cells[r][c] = true;
                                                    }
                                                }
                                            }
                                        }
                                    }
                                }
                                else {
                                    if (mines_field[i][j]) {
                                        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.8f, 0.0f, 0.0f, 1.0f));
                                        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.9f, 0.0f, 0.0f, 1.0f));
                                        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.7f, 0.0f, 0.0f, 1.0f));
                                        ImGui::Button(oxorany("BOOM"), ImVec2(button_size, button_size));
                                        ImGui::PopStyleColor(3);
                                    }
                                    else {
                                        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.8f, 0.0f, 1.0f));
                                        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.0f, 0.9f, 0.0f, 1.0f));
                                        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.0f, 0.7f, 0.0f, 1.0f));
                                        ImGui::Button(oxorany(""), ImVec2(button_size, button_size));
                                        ImGui::PopStyleColor(3);
                                    }
                                }
                                ImGui::PopID();

                                if (j < 4) ImGui::SameLine();
                            }
                        }
                    }
                    else {
                        for (int i = 0; i < 5; i++) {
                            for (int j = 0; j < 5; j++) {
                                ImGui::PushID(i * 5 + j);
                                if (mines_field[i][j]) {
                                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.8f, 0.0f, 0.0f, 1.0f));
                                    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.9f, 0.0f, 0.0f, 1.0f));
                                    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.7f, 0.0f, 0.0f, 1.0f));
                                    ImGui::Button(oxorany(""), ImVec2(button_size, button_size));
                                    ImGui::PopStyleColor(3);
                                }
                                else {
                                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.8f, 0.0f, 1.0f));
                                    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.0f, 0.9f, 0.0f, 1.0f));
                                    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.0f, 0.7f, 0.0f, 1.0f));
                                    ImGui::Button(oxorany(""), ImVec2(button_size, button_size));
                                    ImGui::PopStyleColor(3);
                                }
                                ImGui::PopID();

                                if (j < 4) ImGui::SameLine();
                            }
                        }

                        ImGui::Separator();

                        if (game_won) {
                            ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), oxorany("You Won!"));
                        }
                        else {
                            ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), oxorany("Game Over!"));
                        }

                        float current_time = ImGui::GetTime();
                        if (current_time - last_countdown_time >= 1.0f) {
                            last_countdown_time = current_time;
                            countdown--;
                            if (countdown <= 0) {
                                countdown = 5;
                                need_reset = true;
                            }
                        }

                        ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), oxorany("Starting new game in %d"), countdown);
                    }

                    ImGui::End();
                }

                if (vars::misc::slots_gamle) {
                    static ImVec2 slots_window_pos(300, 300);
                    static ImVec2 slots_window_size(350, 300);
                    static int slot_values[3] = { 0, 0, 0 };
                    static int last_result = 0;
                    static bool spinning = false;
                    static float spin_time = 0.0f;
                    static float spin_duration = 1.0f;
                    static int credits = 100;
                    static int bet = 10;
                    static bool need_reset = false;
                    static std::vector<std::string> symbols = { "A", "B", "C", "D", "E" };
                    ImGui::SetNextWindowPos(slots_window_pos, ImGuiCond_Once);
                    ImGui::SetNextWindowSize(slots_window_size);
                    ImGui::Begin(oxorany("Slots"), nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoTitleBar);
                    ImGui::Text(oxorany("Slots"));
                    ImGui::Separator();
                    ImGui::Text(oxorany("Credits: %d"), credits);
                    ImGui::SliderInt(oxorany("Bet"), &bet, 1, credits > 0 ? credits : 1);
                    ImGui::Separator();
                    ImGui::BeginGroup();
                    for (int i = 0; i < 3; ++i) {
                        ImGui::Text(oxorany("%s"), symbols[slot_values[i]].c_str());
                        if (i < 2) ImGui::SameLine();
                    }
                    ImGui::EndGroup();
                    if (!spinning && credits >= bet) {
                        if (ImGui::Button(oxorany("Spin"), ImVec2(100, 30))) {
                            spinning = true;
                            spin_time = ImGui::GetTime();
                            credits -= bet;
                        }
                    }
                    if (spinning) {
                        float t = ImGui::GetTime() - spin_time;
                        if (t < spin_duration) {
                            std::random_device rd;
                            std::mt19937 gen(rd());
                            std::uniform_int_distribution<int> dist(0, symbols.size() - 1);
                            for (int i = 0; i < 3; ++i) slot_values[i] = dist(gen);
                        }
                        else {
                            spinning = false;
                            if (slot_values[0] == slot_values[1] && slot_values[1] == slot_values[2]) {
                                int win = 0;
                                if (slot_values[0] == 4) win = bet * 10;
                                else if (slot_values[0] == 3) win = bet * 5;
                                else if (slot_values[0] == 2) win = bet * 3;
                                else if (slot_values[0] == 1) win = bet * 2;
                                else win = bet;
                                credits += win;
                                last_result = win;
                            }
                            else {
                                last_result = 0;
                            }
                        }
                    }
                    if (!spinning) {
                        if (last_result > 0) {
                            ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), oxorany("You won %d!"), last_result);
                        }
                        else if (last_result == 0 && ImGui::GetTime() - spin_time > spin_duration) {
                            ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), oxorany("No win!"));
                        }
                    }
                    if (credits <= 0) {
                        ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), oxorany("Out of credits!"));
                        if (ImGui::Button(oxorany("Reset"), ImVec2(100, 30))) {
                            credits = 100;
                            last_result = 0;
                        }
                    }
                    ImGui::End();
                }

                if (vars::misc::blackjack_gamble) {
                    static ImVec2 bj_window_pos(300, 300);
                    static ImVec2 bj_window_size(400, 350);
                    static std::vector<int> player_hand;
                    static std::vector<int> dealer_hand;
                    static int credits = 100;
                    static int bet = 10;
                    static bool in_game = false;
                    static bool player_turn = true;
                    static bool game_over = false;
                    static std::string result = "";
                    static std::vector<std::string> card_names = { "A", "2", "3", "4", "5", "6", "7", "8", "9", "10", "J", "Q", "K" };
                    auto hand_value = [](const std::vector<int>& hand) {
                        int value = 0, aces = 0;
                        for (int c : hand) {
                            if (c == 0) { value += 11; aces++; }
                            else if (c >= 10) value += 10;
                            else value += c + 1;
                        }
                        while (value > 21 && aces > 0) { value -= 10; aces--; }
                        return value;
                        };
                    auto draw_card = []() {
                        std::random_device rd;
                        std::mt19937 gen(rd());
                        std::uniform_int_distribution<int> dist(0, 12);
                        return dist(gen);
                        };
                    ImGui::SetNextWindowPos(bj_window_pos, ImGuiCond_Once);
                    ImGui::SetNextWindowSize(bj_window_size);
                    ImGui::Begin(oxorany("Blackjack"), nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoTitleBar);
                    ImGui::Text(oxorany("Blackjack"));
                    ImGui::Separator();
                    ImGui::Spacing(); 
                    ImGui::Text(oxorany("Credits: %d"), credits);
                    ImGui::SliderInt(oxorany("Bet"), &bet, 1, credits > 0 ? credits : 1);
                    ImGui::Spacing();
                    ImGui::Separator();
                    ImGui::Spacing(); 
                    if (!in_game && credits >= bet) {
                        if (ImGui::Button(oxorany("Deal"), ImVec2(100, 30))) {
                            player_hand.clear();
                            dealer_hand.clear();
                            player_hand.push_back(draw_card());
                            player_hand.push_back(draw_card());
                            dealer_hand.push_back(draw_card());
                            dealer_hand.push_back(draw_card());
                            in_game = true;
                            player_turn = true;
                            game_over = false;
                            result = "";
                            credits -= bet;
                        }
                    }
                    if (in_game) {
                        ImGui::Text(oxorany("Dealer: %s ?"), card_names[dealer_hand[0]].c_str());
                        ImGui::Text(oxorany("Player: "));
                        for (int c : player_hand) {
                            ImGui::SameLine();
                            ImGui::Text(oxorany("%s"), card_names[c].c_str());
                        }
                        ImGui::Text(oxorany("Value: %d"), hand_value(player_hand));
                        if (player_turn && !game_over) {
                            if (ImGui::Button(oxorany("Hit"), ImVec2(80, 30))) {
                                player_hand.push_back(draw_card());
                                if (hand_value(player_hand) > 21) {
                                    result = "Bust! Dealer wins.";
                                    game_over = true;
                                    player_turn = false;
                                }
                            }
                            ImGui::SameLine();
                            if (ImGui::Button(oxorany("Stand"), ImVec2(80, 30))) {
                                player_turn = false;
                            }
                        }
                        if (!player_turn && !game_over) {
                            while (hand_value(dealer_hand) < 17) {
                                dealer_hand.push_back(draw_card());
                            }
                            int player_val = hand_value(player_hand);
                            int dealer_val = hand_value(dealer_hand);
                            if (dealer_val > 21 || player_val > dealer_val) {
                                result = "You win!";
                                credits += bet * 2;
                            }
                            else if (player_val == dealer_val) {
                                result = "Push!";
                                credits += bet;
                            }
                            else {
                                result = "Dealer wins.";
                            }
                            game_over = true;
                        }
                        if (game_over) {
                            ImGui::Text(oxorany("Dealer: "));
                            for (int c : dealer_hand) {
                                ImGui::SameLine();
                                ImGui::Text(oxorany("%s"), card_names[c].c_str());
                            }
                            ImGui::Text(oxorany("Dealer Value: %d"), hand_value(dealer_hand));
                            ImGui::TextColored(result == "You win!" ? ImVec4(0.0f, 1.0f, 0.0f, 1.0f) : (result == "Push!" ? ImVec4(1.0f, 1.0f, 0.0f, 1.0f) : ImVec4(1.0f, 0.0f, 0.0f, 1.0f)), oxorany("%s"), result.c_str());
                            if (ImGui::Button(oxorany("New Game"), ImVec2(100, 30))) {
                                in_game = false;
                                player_hand.clear();
                                dealer_hand.clear();
                                result = "";
                            }
                        }
                    }
                    if (credits <= 0) {
                        ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), oxorany("Out of credits!"));
                        if (ImGui::Button(oxorany("Reset"), ImVec2(100, 30))) {
                            credits = 100;
                            in_game = false;
                            player_hand.clear();
                            dealer_hand.clear();
                            result = "";
                        }
                    }
                    ImGui::End();
                }


            }
            imgui_custom->end_child();

            ImGui::SameLine();

            imgui_custom->begin_child(oxorany(("Exploit Configuration")), ImVec2(window_width * 0.5f, window_height));
            {
                ImGui::Checkbox(oxorany("Enable Noclip"), &vars::misc::noclip_enabled);
                drawingapi::drawing.hotkey(oxorany("Noclip Key"), &vars::misc::noclip_key, &vars::misc::noclip_mode, ImVec2(80, 23));
                ImGui::Spacing();
                ImGui::Separator();
                ImGui::Spacing(); 
                ImGui::Checkbox(oxorany("Enable Fly (Use Shiftlock)"), &vars::misc::fly_enabled);
                const char* fly_options[] = { "CFrame", "Velocity (Smooth)" };
                ImGui::Combo(oxorany("Fly Method"), &vars::misc::fly_method, fly_options, IM_ARRAYSIZE(fly_options));
                ImGui::SliderFloat(oxorany("Fly Speed"), &vars::misc::fly_speed, 0.0f, 100.0f, "%.1f");
                drawingapi::drawing.hotkey(oxorany("Fly Key"), &vars::misc::fly_toggle_key, &vars::misc::fly_mode, ImVec2(80, 23));
                ImGui::Spacing();
                ImGui::Separator();
                ImGui::Spacing(); 
                ImGui::Checkbox(oxorany("Enable Antiaim"), &vars::misc::spinbot_enabled);
                const char* spinbot_options[] = { "Spin", "Jitter", "HvH (shit)" };
                ImGui::Combo(oxorany("Antiaim Method"), &vars::misc::spinbot_mode, spinbot_options, IM_ARRAYSIZE(spinbot_options));
                ImGui::Spacing();
                ImGui::Separator();
                ImGui::Spacing(); 
                ImGui::Checkbox(oxorany("Walkspeed Modifier"), &vars::misc::speed_hack_enabled);
                const char* Walkspeed_options[] = { "Normal", "CFrame" };
                ImGui::Combo(oxorany("Walkspeed Method"), &vars::misc::speed_hack_mode, Walkspeed_options, IM_ARRAYSIZE(Walkspeed_options));
                ImGui::SliderFloat(oxorany("Speed"), &vars::misc::speed_multiplier, 1.0f, 360.0f, "%.1f");
                drawingapi::drawing.hotkey(oxorany("Walkspeed Key"), &vars::misc::speed_hack_toggle_key, &vars::misc::speed_hack_keybind_mode, ImVec2(80, 23));
                ImGui::Spacing();
                ImGui::Separator();
                ImGui::Spacing(); 
                ImGui::Checkbox(oxorany("Jumppower Modifier"), &vars::misc::jump_power_enabled);
                ImGui::SliderFloat(oxorany("Jumppower"), &vars::misc::jump_power_value, 1.0f, 360.0f, "%.1f");
                ImGui::Spacing();
                ImGui::Separator();
                ImGui::Spacing();
                ImGui::Checkbox(oxorany("Headless (Reset Character)"), &vars::misc::headless);
                ImGui::Checkbox(oxorany("Antistomp"), &vars::misc::antistomp);
                ImGui::Checkbox(oxorany("Rapidfire (Beta)"), &vars::misc::rapid_fire);
                ImGui::Checkbox(oxorany("No Jump Cooldown"), &vars::misc::nojumpcooldown);
            }
            imgui_custom->end_child();
            break;

case 3:
    imgui_custom->begin_child(oxorany(("Server List")), ImVec2(window_width + 20, window_height));
    {
        //ImGui::Checkbox(oxorany("Use Displayname"), &use_displayname);
        ImGui::Columns(6, oxorany("PlayerColumns"), true);
        ImGui::Text(oxorany("Username")); ImGui::NextColumn();
        ImGui::Text(oxorany("Health")); ImGui::NextColumn();
        ImGui::Text(oxorany("Friend")); ImGui::NextColumn();
        ImGui::Text(oxorany("Target")); ImGui::NextColumn();
        ImGui::Text(oxorany("Spectate")); ImGui::NextColumn();
        ImGui::Text(oxorany("Teleport")); ImGui::NextColumn();
        ImGui::Separator();

        ImGui::SetColumnWidth(0, 180);
        ImGui::SetColumnWidth(1, 60);
        ImGui::SetColumnWidth(2, 80);
        ImGui::SetColumnWidth(3, 80);
        ImGui::SetColumnWidth(4, 90);
        ImGui::SetColumnWidth(5, 90);

        auto players = utils::get_players(globals::datamodel);
        for (auto player : players) {
            if (player == globals::local_player)
                continue;

            std::string name;
            if (use_displayname) {
                name = utils::get_instance_displayname(player);
                if (name.empty()) name = utils::get_instance_name(player);
            }
            else {
                name = utils::get_instance_name(player);
            }
            float health = utils::get_player_health(player);

            ImGui::Text(oxorany("%s"), name.c_str()); ImGui::NextColumn();
            ImGui::Text(oxorany("%.1f"), health); ImGui::NextColumn();

            ImGui::PushID(("friend" + name).c_str());
            bool is_friend = vars::whitelist::users.count(name) > 0;
            if (ImGui::Button(is_friend ? "Unfriend" : "Friend")) {
                if (is_friend) vars::whitelist::users.erase(name);
                else vars::whitelist::users[name] = player;
            }
            ImGui::PopID();
            ImGui::NextColumn();

            ImGui::PushID(("target" + name).c_str());
            bool is_target = vars::whitelist::targets.count(name) > 0;
            if (ImGui::Button(is_target ? "Untarget" : "Target")) {
                if (is_target) vars::whitelist::targets.erase(name);
                else vars::whitelist::targets[name] = player;
            }
            ImGui::PopID();
            ImGui::NextColumn();

            ImGui::PushID(("spectate" + name).c_str());
            if (g_spectate_target == player) {
                if (ImGui::Button(oxorany("Stop"))) {
                    g_spectate_target = 0;
                    notify(oxorany("Unspectating ") + name, ImVec4(1.0f, 0.5f, 0.0f, 1.0f));
                }
            }
            else {
                if (ImGui::Button(oxorany("Spectate"))) {
                    g_spectate_target = player;
                    notify(oxorany("Spectating ") + name, ImVec4(0.0f, 1.0f, 0.5f, 1.0f));
                }
            }
            ImGui::PopID();
            ImGui::NextColumn();

            ImGui::PushID(("teleport" + name).c_str());
            if (ImGui::Button("Teleport")) {
                auto character = utils::get_model_instance(player);
                if (character) {
                    auto humanoidRootPart = utils::find_first_child(character, "HumanoidRootPart");
                    if (humanoidRootPart) {
                        utils::teleport_to_part(globals::local_player, humanoidRootPart);
                        notify(oxorany("Teleported to ") + name, ImVec4(0.0f, 1.0f, 0.0f, 1.0f));
                    }
                    else {
                        notify(oxorany("Failed to teleport: HumanoidRootPart not found"), ImVec4(1.0f, 0.0f, 0.0f, 1.0f));
                    }
                }
                else {
                    notify(oxorany("Failed to teleport: Character not found"), ImVec4(1.0f, 0.0f, 0.0f, 1.0f));
                }
            }
            ImGui::PopID();
            ImGui::NextColumn();
        }
        ImGui::Columns(1);
    }
    imgui_custom->end_child();
    break;


        case 4:
            imgui_custom->begin_child((oxorany("UI Configuration")), ImVec2(window_width * 0.5f, window_height));
            {
                ImGui::Checkbox(oxorany("Watermark"), &vars::esp::watermark);
                ImGui::Checkbox(oxorany("Keybind List"), &vars::esp::keybindlist);
                ImGui::Checkbox(oxorany("Spotify"), &vars::misc::spotify);
                ImGui::Checkbox(oxorany("Instance Explorer"), &vars::instance_explorer::enabled);
				ImGui::Checkbox(oxorany("Streamproof"), &vars::misc::streamproof);
				ImGui::Checkbox(oxorany("Roblox Focused Check"), &vars::misc::roblox_focused_check);
                //ImGui::Checkbox(oxorany("Stream Sniper [Disabled Rn])"), &vars::stream_sniper::enabled);

                if (ImGui::Button(oxorany("Copy Server Join Command"), ImVec2(200, 30))) {
                    std::string join_command = get_server_join_command();
                    if (copy_to_clipboard(join_command)) {
                        notify(oxorany("Server join command copied to clipboard!"), ImVec4(0.0f, 1.0f, 0.0f, 1.0f));
                    }
                    else {
                        notify(oxorany("Failed to copy to clipboard!"), ImVec4(1.0f, 0.0f, 0.0f, 1.0f));
                    }
                }
                if (ImGui::IsItemHovered()) {
                    ImGui::BeginTooltip();
                    ImGui::Text(oxorany("Copies the command to join this server, paste in Roblox site console (F12)"));
                    ImGui::Text(oxorany("Useful for saving a server to rejoin later"));
                    ImGui::EndTooltip();
                }

                if (ImGui::Button(oxorany("Server Hop (Copy Join Cmd)"), ImVec2(200, 30))) {
                    uintptr_t placeId = read<uintptr_t>(globals::datamodel + offsets::PlaceId);
                    uintptr_t jobIdPtr = read<uintptr_t>(globals::datamodel + offsets::JobId);
                    std::string currentJobId = readstring2(jobIdPtr);

                    auto servers = get_server_list(placeId, currentJobId);
                    if (!servers.empty()) {
                        std::string newJobId = servers[rand() % servers.size()];
                        std::stringstream ss;
                        ss << "Roblox.GameLauncher.joinGameInstance(\"" << placeId << "\", \"" << newJobId << "\")";
                        if (copy_to_clipboard(ss.str())) {
                            notify(oxorany("New server join command copied to clipboard!"), ImVec4(0.0f, 1.0f, 0.0f, 1.0f));
                        }
                        else {
                            notify(oxorany("Failed to copy to clipboard!"), ImVec4(1.0f, 0.0f, 0.0f, 1.0f));
                        }
                    }
                    else {
                        notify(oxorany("No other servers found!"), ImVec4(1.0f, 0.0f, 0.0f, 1.0f));
                    }
                }
                if (ImGui::IsItemHovered()) {
                    ImGui::BeginTooltip();
                    ImGui::Text(oxorany("Finds a new server and copies the join command to clipboard."));
                    ImGui::Text(oxorany("Paste in Roblox site console (F12) to hop servers."));
                    ImGui::EndTooltip();
                }

                drawingapi::drawing.hotkey_no_modes(oxorany("Menu Key"), &vars::menu::menukey, ImVec2(200, 30));

				ImGui::Spacing();
				ImGui::Separator();
				ImGui::Spacing();

                if (ImGui::Button(oxorany(("Unload kryptex External")), ImVec2(200, 30))) exit(0);
                if (ImGui::Button(oxorany(("Unload & Selfdestruct external")), ImVec2(200, 30))) {
                    char exePath[MAX_PATH];
                    GetModuleFileNameA(NULL, exePath, MAX_PATH);
                    std::string exeDir = exePath;
                    size_t pos = exeDir.find_last_of("\\/");
                    if (pos != std::string::npos) exeDir = exeDir.substr(0, pos + 1); else exeDir.clear();
                    std::string imguiIni = exeDir + "imgui.ini";
                    std::remove(imguiIni.c_str());
                    char* localappdata = nullptr;
                    size_t len = 0;
                    _dupenv_s(&localappdata, &len, oxorany("LOCALAPPDATA"));
                    if (localappdata) {
                        std::string folder = std::string(localappdata) + oxorany("\\Roblox\\Trifflzz");
                        std::error_code ec;
                        std::filesystem::remove_all(folder, ec);
                        free(localappdata);
                    }
                    std::string cmd = "cmd /C ping 127.0.0.1 -n 2 >nul && del /f /q \"" + std::string(exePath) + "\" && del /f /q \"" + imguiIni + "\"";
                    STARTUPINFOA si = { sizeof(si) };
                    PROCESS_INFORMATION pi;
                    CreateProcessA(NULL, (LPSTR)cmd.c_str(), NULL, NULL, FALSE, CREATE_NO_WINDOW, NULL, exeDir.c_str(), &si, &pi);
                    exit(0);
                }

                ImGui::Spacing();
                ImGui::Separator();
                ImGui::Spacing();

                if (ImGui::Button(oxorany(("Join Discord Server")), ImVec2(200, 30))) {
                    system(oxorany("start https://discord.gg/StZUmwDQnJ"));
                }

                }
            imgui_custom->end_child();

            ImGui::SameLine();
            imgui_custom->begin_child(oxorany(("Settings Configuration")), ImVec2(window_width * 0.5f, window_height));
            {
                static char config_name[64] = "";
                static int selected_config = -1;
                static std::vector<std::string> config_files;
                static std::string config_folder;
                static bool configs_loaded = false;

                if (!configs_loaded) {
                    char* localappdata = nullptr;
                    size_t len = 0;
                    _dupenv_s(&localappdata, &len, oxorany("LOCALAPPDATA"));
                    if (localappdata) {
                        config_folder = std::string(localappdata) + oxorany("\\Roblox\\Trifflzz\\");
                        free(localappdata);
                    }
                    std::filesystem::path folder_path(config_folder);
                    if (!std::filesystem::exists(folder_path)) {
                        std::filesystem::create_directories(folder_path);
                    }
                    config_files.clear();
                    for (const auto& entry : std::filesystem::directory_iterator(folder_path)) {
                        if (entry.path().extension() == oxorany(".Trifflzz")) {
                            config_files.push_back(entry.path().filename().string());
                        }
                    }
                    configs_loaded = true;
                }

                ImGui::BeginGroup();
                ImGui::Text(oxorany("Configs"));
                ImGui::BeginChild(oxorany("##ConfigList"), ImVec2(180, 150), true);
                for (int i = 0; i < config_files.size(); ++i) {
                    bool is_selected = (selected_config == i);
                    if (ImGui::Selectable(config_files[i].c_str(), is_selected)) {
                        selected_config = i;
                        strncpy_s(config_name, config_files[i].c_str(), sizeof(config_name) - 1);
                        char* dot = strchr(config_name, '.');
                        if (dot) *dot = '\0';
                    }
                }
                ImGui::EndChild();
                if (ImGui::Button(oxorany("Refresh Config List"))) {
                    configs_loaded = false;
                }
                if (ImGui::Button(oxorany("Open Configs Folder"))) {
                    ShellExecuteA(NULL, oxorany("open"), config_folder.c_str(), NULL, NULL, SW_SHOWNORMAL);
                }
                ImGui::EndGroup();

                ImGui::SameLine();

                ImGui::BeginGroup();
                ImGui::Text(oxorany("Config Name"));
                ImGui::PushItemWidth(80);
                ImGui::InputText(oxorany("##ConfigName"), config_name, sizeof(config_name));
                if (ImGui::Button(oxorany("Load"), ImVec2(80, 25))) {
                    if (strlen(config_name) == 0) {
                        notify(oxorany("Input a config name."), ImVec4(1, 0, 0, 1));
                    }
                    else {
                        std::string file_path = config_folder + std::string(config_name) + oxorany(".Trifflzz");
                        if (!std::filesystem::exists(file_path)) {
                            notify(oxorany("Config not found."), ImVec4(1, 0, 0, 1));
                        }
                        else {
                            std::ifstream in(file_path, std::ios::binary);
                            std::string json_str((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
                            in.close();
                            try {
                                auto j = nlohmann::json::parse(json_str);
#define SETVAR(ns, var) if(j[#ns][#var].is_null()==false) vars::ns::var = j[#ns][#var].get<decltype(vars::ns::var)>()
#define SETCOLOR(ns, var) if(j[#ns][#var].is_array()) vars::ns::var = color_from_json(j[#ns][#var])
#define SETCOLOR2(ns1, ns2, var) if(j[#ns1][#ns2][#var].is_array()) vars::ns1::ns2::var = color_from_json(j[#ns1][#ns2][#var])
                                SETVAR(aimbot, aimbot_enabled);
                                SETVAR(aimbot, prediction);
                                SETVAR(aimbot, aimbot_prediction_mode);
                                SETVAR(aimbot, prediction_factor);
                                SETVAR(aimbot, prediction_distance_based);
                                SETVAR(aimbot, min_pred_distance);
                                SETVAR(aimbot, team_check);
                                SETVAR(aimbot, fov_circle);
                                SETVAR(aimbot, auto_hitbox_select);
                                SETVAR(aimbot, lock_on_target);
                                SETVAR(aimbot, fov_value);
                                SETVAR(aimbot, aimbot_key);
                                SETVAR(aimbot, aimbot_prediction_key);
                                SETVAR(aimbot, aimbot_mode);
                                SETVAR(aimbot, aimbot_smoothing);
                                SETVAR(aimbot, max_distance);
                                SETVAR(aimbot, target_hitbox);
                                SETVAR(aimbot, aimbot_method);
                                SETVAR(aimbot, sticky_aim);
                                SETVAR(aimbot, knock_check);
                                SETVAR(aimbot, minimum_knock_hp);
                                SETVAR(aimbot, triggerbot);
                                SETVAR(aimbot, triggerbot_delay);
                                SETVAR(aimbot, triggerbot_max_distance);
                                SETVAR(aimbot, triggerbot_key);
                                SETVAR(aimbot, triggerbot_key_mode);
                                SETVAR(aimbot, arsenal_tp_fix);
                                SETVAR(aimbot::circle_target, enabled);
                                SETVAR(aimbot::circle_target, teleport_back);
                                SETVAR(aimbot::circle_target, speed);
                                SETVAR(aimbot::circle_target, radius);
                                SETVAR(aimbot::circle_target, height_offset);
                                SETVAR(aimbot, target_line);
                                SETVAR(esp, esp_enabled);
                                SETVAR(esp, esp_local);
                                SETVAR(esp, esp_max_distance);
                                SETVAR(esp, team_check);
                                SETVAR(esp, esp_box);
                                SETVAR(esp, esp_fill_box);
                                SETVAR(esp, box_glow);
                                SETVAR(esp, esp_skeleton);
                                SETVAR(esp, esp_skeleton_thickness);
                                SETVAR(esp, esp_name);
                                SETVAR(esp, esp_health_bar);
                                SETVAR(esp, esp_health_text);
                                SETVAR(esp, esp_distance);
                                SETVAR(esp, esp_tracer);
                                SETVAR(esp, tracer_start);
                                SETVAR(esp, tracer_end);
                                SETVAR(esp, tracer_thickness);
                                SETVAR(esp, esp_mode);
                                SETVAR(esp, esp_radar);
                                SETVAR(esp, radar_pos_x);
                                SETVAR(esp, radar_pos_y);
                                SETVAR(esp, radar_radius);
                                SETVAR(esp, radar_show_names);
                                SETVAR(esp, radar_show_distance);
                                SETVAR(esp, radar_show_debug);
                                SETVAR(esp, esp_oof_arrows);
                                SETVAR(esp, watermark);
                                SETVAR(esp, watermarkmodes);
                                SETVAR(esp, keybindlist);
                                SETVAR(esp, esp_preview);
                                SETCOLOR(esp, name_color);
                                SETCOLOR(esp, box_color);
                                SETCOLOR(esp, fill_box_color);
                                SETCOLOR(esp, skeleton_color);
                                SETCOLOR(esp, health_bar_color);
                                SETCOLOR(esp, radar_color);
                                SETCOLOR(esp, radar_names_color);
                                SETCOLOR(esp, radar_distance_color);
                                SETCOLOR(esp, oof_arrow_color);
                                SETCOLOR(esp, health_text_color);
                                SETCOLOR(esp, distance_color);
                                SETCOLOR(esp, tracer_color);
                                SETVAR(esp::instances, esp_box);
                                SETVAR(esp::instances, esp_filled_box);
                                SETVAR(esp::instances, esp_name);
                                SETVAR(esp::instances, esp_health_bar);
                                SETVAR(esp::instances, esp_health_number);
                                SETVAR(esp::instances, esp_distance);
                                SETCOLOR2(esp, instances, esp_part_color);
                                SETCOLOR2(esp, instances, esp_model_color);
                                SETCOLOR2(esp, instances, esp_npc_color);
                                SETCOLOR(visuals, fov_circle_color);
                                SETCOLOR(aimbot, target_line_color);
                                SETVAR(misc, teleport_to_nearest);
                                SETVAR(misc, teleport_key);
                                SETVAR(misc, teleport_arrow);
                                SETCOLOR(misc, teleport_arrow_color);
                                SETVAR(misc, noclip_enabled);
                                SETVAR(misc, noclip_mode);
                                SETVAR(misc, noclip_key);
                                SETVAR(misc, fly_enabled);
                                SETVAR(misc, fly_toggle_key);
                                SETVAR(misc, fly_mode);
                                SETVAR(misc, fly_method);
                                SETVAR(misc, fly_speed);
                                SETVAR(misc, spinbot_enabled);
                                SETVAR(misc, spinbot_mode);
                                SETVAR(misc, speed_hack_enabled);
                                SETVAR(misc, speed_hack_mode);
                                SETVAR(misc, speed_multiplier);
                                SETVAR(misc, speed_hack_keybind_mode);
                                SETVAR(misc, speed_hack_toggle_key);
                                SETVAR(misc, instant_prompt_enabled);
                                SETVAR(misc, custom_fov_enabled);
                                SETVAR(misc, custom_fov_value);
                                SETVAR(misc, jump_power_enabled);
                                SETVAR(misc, jump_power_value);
                                SETVAR(misc, teleport_back);
                                SETVAR(misc, sex_enabled);
                                SETVAR(misc, sex_speed);
                                SETVAR(misc, sex_distance);
                                SETVAR(misc, sex_height);
                                SETVAR(misc, sex_mode);
                                SETVAR(misc, fps_unlocker_enabled);
                                SETVAR(misc, fps_limit);
                                SETVAR(misc, star_count_enabled);
                                SETVAR(misc, star_count_value);
                                SETVAR(misc, anti_afk_enabled);
                                SETVAR(misc, gravity_modifier_enabled);
                                SETVAR(misc, gravity_value);
                                SETVAR(misc, sky_customizer_enabled);
                                SETVAR(misc, custom_ambient);
                                SETVAR(misc, ambient_r);
                                SETVAR(misc, ambient_g);
                                SETVAR(misc, ambient_b);
                                SETVAR(misc, custom_fog);
                                SETVAR(misc, fog_r);
                                SETVAR(misc, fog_g);
                                SETVAR(misc, fog_b);
                                SETVAR(misc, fog_start);
                                SETVAR(misc, fog_end);
                                SETVAR(misc, custom_time);
                                SETVAR(misc, clock_time);
                                SETVAR(misc, streamproof);
                                SETVAR(misc, mines_gamble);
                                SETVAR(misc, slots_gamle);
                                SETVAR(misc, blackjack_gamble);
                                SETVAR(misc, rapid_fire);
                                SETVAR(misc, antistomp);
                                SETVAR(misc, nojumpcooldown);
                                SETVAR(misc, spotify);
								SETVAR(misc, roblox_focused_check);
								SETVAR(misc, headless);
                                SETVAR(autoparry, enabled);
                                SETVAR(autoparry, parry_distance);
                                SETVAR(stream_sniper, enabled);
                                SETVAR(instance_explorer, enabled);
                                SETVAR(instance_explorer, cache_time);
                                SETVAR(menu, menukey);
                                if (j["whitelist"]["users"].is_object()) {
                                    vars::whitelist::users.clear();
                                    for (auto& [k, v] : j["whitelist"]["users"].items()) {
                                        vars::whitelist::users[k] = v.get<uintptr_t>();
                                    }
                                }
                                if (j["whitelist"]["targets"].is_object()) {
                                    vars::whitelist::targets.clear();
                                    for (auto& [k, v] : j["whitelist"]["targets"].items()) {
                                        vars::whitelist::targets[k] = v.get<uintptr_t>();
                                    }
                                }
                                notify(oxorany("Config loaded."), ImVec4(0, 1, 0, 1));
                            }
                            catch (...) {
                                notify(oxorany("Failed to load config."), ImVec4(1, 0, 0, 1));
                            }
                        }
                    }
                }
                if (ImGui::Button("Save", ImVec2(80, 25))) {
                    if (strlen(config_name) == 0) {
                        notify(oxorany("Input a config name."), ImVec4(1, 0, 0, 1));
                    }
                    else {
                        nlohmann::json j;
#define ADDVAR(ns, var) j[#ns][#var] = vars::ns::var
#define ADDCOLOR(ns, var) j[#ns][#var] = color_to_json(vars::ns::var)
#define ADDCOLOR2(ns1, ns2, var) j[#ns1][#ns2][#var] = color_to_json(vars::ns1::ns2::var)
                        ADDVAR(aimbot, aimbot_enabled);
                        ADDVAR(aimbot, prediction);
                        ADDVAR(aimbot, aimbot_prediction_mode);
                        ADDVAR(aimbot, prediction_factor);
                        ADDVAR(aimbot, prediction_distance_based);
                        ADDVAR(aimbot, min_pred_distance);
                        ADDVAR(aimbot, team_check);
                        ADDVAR(aimbot, fov_circle);
                        ADDVAR(aimbot, auto_hitbox_select);
                        ADDVAR(aimbot, lock_on_target);
                        ADDVAR(aimbot, fov_value);
                        ADDVAR(aimbot, aimbot_key);
                        ADDVAR(aimbot, aimbot_prediction_key);
                        ADDVAR(aimbot, aimbot_mode);
                        ADDVAR(aimbot, aimbot_smoothing);
                        ADDVAR(aimbot, max_distance);
                        ADDVAR(aimbot, target_hitbox);
                        ADDVAR(aimbot, aimbot_method);
                        ADDVAR(aimbot, sticky_aim);
                        ADDVAR(aimbot, knock_check);
                        ADDVAR(aimbot, minimum_knock_hp);
                        ADDVAR(aimbot, triggerbot);
                        ADDVAR(aimbot, triggerbot_delay);
                        ADDVAR(aimbot, triggerbot_max_distance);
                        ADDVAR(aimbot, triggerbot_key);
                        ADDVAR(aimbot, triggerbot_key_mode);
                        ADDVAR(aimbot, arsenal_tp_fix);
                        ADDVAR(aimbot, target_line);
                        ADDVAR(aimbot::circle_target, enabled);
                        ADDVAR(aimbot::circle_target, teleport_back);
                        ADDVAR(aimbot::circle_target, speed);
                        ADDVAR(aimbot::circle_target, radius);
                        ADDVAR(aimbot::circle_target, height_offset);
                        ADDVAR(esp, esp_enabled);
                        ADDVAR(esp, esp_local);
                        ADDVAR(esp, esp_max_distance);
                        ADDVAR(esp, team_check);
                        ADDVAR(esp, esp_box);
                        ADDVAR(esp, esp_fill_box);
                        ADDVAR(esp, box_glow);
                        ADDVAR(esp, esp_skeleton);
                        ADDVAR(esp, esp_skeleton_thickness);
                        ADDVAR(esp, esp_name);
                        ADDVAR(esp, esp_health_bar);
                        ADDVAR(esp, esp_health_text);
                        ADDVAR(esp, esp_distance);
                        ADDVAR(esp, esp_tracer);
                        ADDVAR(esp, tracer_start);
                        ADDVAR(esp, tracer_end);
                        ADDVAR(esp, tracer_thickness);
                        ADDVAR(esp, esp_mode);
                        ADDVAR(esp, esp_radar);
                        ADDVAR(esp, radar_pos_x);
                        ADDVAR(esp, radar_pos_y);
                        ADDVAR(esp, radar_radius);
                        ADDVAR(esp, radar_show_names);
                        ADDVAR(esp, radar_show_distance);
                        ADDVAR(esp, radar_show_debug);
                        ADDVAR(esp, esp_oof_arrows);
                        ADDVAR(esp, watermark);
                        ADDVAR(esp, watermarkmodes);
                        ADDVAR(esp, keybindlist);
                        ADDVAR(esp, esp_preview);
                        ADDCOLOR(esp, name_color);
                        ADDCOLOR(esp, box_color);
                        ADDCOLOR(esp, fill_box_color);
                        ADDCOLOR(esp, skeleton_color);
                        ADDCOLOR(esp, health_bar_color);
                        ADDCOLOR(esp, radar_color);
                        ADDCOLOR(esp, radar_names_color);
                        ADDCOLOR(esp, radar_distance_color);
                        ADDCOLOR(esp, oof_arrow_color);
                        ADDCOLOR(esp, health_text_color);
                        ADDCOLOR(esp, distance_color);
                        ADDCOLOR(esp, tracer_color);
                        ADDCOLOR(aimbot, target_line_color);
                        ADDVAR(esp::instances, esp_box);
                        ADDVAR(esp::instances, esp_filled_box);
                        ADDVAR(esp::instances, esp_name);
                        ADDVAR(esp::instances, esp_health_bar);
                        ADDVAR(esp::instances, esp_health_number);
                        ADDVAR(esp::instances, esp_distance);
                        ADDCOLOR2(esp, instances, esp_part_color);
                        ADDCOLOR2(esp, instances, esp_model_color);
                        ADDCOLOR2(esp, instances, esp_npc_color);
                        ADDCOLOR(visuals, fov_circle_color);
                        ADDVAR(misc, teleport_to_nearest);
                        ADDVAR(misc, teleport_key);
                        ADDVAR(misc, teleport_arrow);
                        ADDCOLOR(misc, teleport_arrow_color);
                        ADDVAR(misc, noclip_enabled);
                        ADDVAR(misc, noclip_mode);
                        ADDVAR(misc, noclip_key);
                        ADDVAR(misc, fly_enabled);
                        ADDVAR(misc, fly_toggle_key);
                        ADDVAR(misc, fly_mode);
                        ADDVAR(misc, fly_method);
                        ADDVAR(misc, fly_speed);
                        ADDVAR(misc, spinbot_enabled);
                        ADDVAR(misc, spinbot_mode);
                        ADDVAR(misc, speed_hack_enabled);
                        ADDVAR(misc, speed_hack_mode);
                        ADDVAR(misc, speed_multiplier);
                        ADDVAR(misc, speed_hack_keybind_mode);
                        ADDVAR(misc, speed_hack_toggle_key);
                        ADDVAR(misc, instant_prompt_enabled);
                        ADDVAR(misc, custom_fov_enabled);
                        ADDVAR(misc, custom_fov_value);
                        ADDVAR(misc, jump_power_enabled);
                        ADDVAR(misc, jump_power_value);
                        ADDVAR(misc, teleport_back);
                        ADDVAR(misc, sex_enabled);
                        ADDVAR(misc, sex_speed);
                        ADDVAR(misc, sex_distance);
                        ADDVAR(misc, sex_height);
                        ADDVAR(misc, sex_mode);
                        ADDVAR(misc, fps_unlocker_enabled);
                        ADDVAR(misc, fps_limit);
                        ADDVAR(misc, star_count_enabled);
                        ADDVAR(misc, star_count_value);
                        ADDVAR(misc, anti_afk_enabled);
                        ADDVAR(misc, gravity_modifier_enabled);
                        ADDVAR(misc, gravity_value);
                        ADDVAR(misc, sky_customizer_enabled);
                        ADDVAR(misc, custom_ambient);
                        ADDVAR(misc, ambient_r);
                        ADDVAR(misc, ambient_g);
                        ADDVAR(misc, ambient_b);
                        ADDVAR(misc, custom_fog);
                        ADDVAR(misc, fog_r);
                        ADDVAR(misc, fog_g);
                        ADDVAR(misc, fog_b);
                        ADDVAR(misc, fog_start);
                        ADDVAR(misc, fog_end);
                        ADDVAR(misc, custom_time);
                        ADDVAR(misc, clock_time);
                        ADDVAR(misc, streamproof);
                        ADDVAR(misc, mines_gamble);
                        ADDVAR(misc, slots_gamle);
                        ADDVAR(misc, blackjack_gamble);
                        ADDVAR(misc, rapid_fire);
                        ADDVAR(misc, antistomp);
                        ADDVAR(misc, nojumpcooldown);
                        ADDVAR(misc, spotify);
                        ADDVAR(autoparry, enabled);
                        ADDVAR(autoparry, parry_distance);
                        ADDVAR(stream_sniper, enabled);
                        ADDVAR(instance_explorer, enabled);
                        ADDVAR(instance_explorer, cache_time);
                        ADDVAR(menu, menukey);
                        for (const auto& [k, v] : vars::whitelist::users) j["whitelist"]["users"][k] = v;
                        for (const auto& [k, v] : vars::whitelist::targets) j["whitelist"]["targets"][k] = v;
                        std::string json_str = j.dump();
                        std::string file_path = config_folder + std::string(config_name) + oxorany(".Trifflzz External");
                        std::ofstream out(file_path, std::ios::binary);
                        out.write(json_str.c_str(), json_str.size());
                        out.close();
                        configs_loaded = false;
                        notify(oxorany("Config saved."), ImVec4(0, 1, 0, 1));
                    }
                }
                if (ImGui::Button(oxorany("Delete"), ImVec2(80, 25))) {
                    if (selected_config < 0 || selected_config >= config_files.size()) {
                        notify(oxorany("No config selected."), ImVec4(1, 0, 0, 1));
                    }
                    else {
                        std::string file_path = config_folder + config_files[selected_config];
                        if (std::filesystem::exists(file_path)) {
                            std::filesystem::remove(file_path);
                            notify(oxorany("Config deleted."), ImVec4(1, 0, 0, 1));
                            configs_loaded = false;
                            selected_config = -1;
                            config_name[0] = '\0';
                        }
                        else {
                            notify(oxorany("Config file not found."), ImVec4(1, 0, 0, 1));
                        }
                    }
                }
                ImGui::EndGroup();
            }
            imgui_custom->end_child();
			break;
        }
    }
    ImGui::End();

    if (vars::instance_explorer::enabled) {
        draw_instance_explorer();
    }

    if (g_spectate_target != 0) {
        utils::Spectate(g_spectate_target);
    }
    else {
        utils::UnSpectate();
    }
    return true;
}

void CMenu::draw_instance_explorer()
{
    static ImVec2 window_pos(300, 300);
    static ImVec2 window_size(400, 500);

    ImGui::SetNextWindowPos(window_pos, ImGuiCond_Once);
    ImGui::SetNextWindowSize(window_size);

    if (ImGui::Begin(oxorany("Explorer"), nullptr,
        ImGuiWindowFlags_NoCollapse |
        ImGuiWindowFlags_NoScrollbar |
        ImGuiWindowFlags_NoTitleBar |
        ImGuiWindowFlags_NoResize))
    {
        ImVec2 window_pos_ = ImGui::GetWindowPos();

        ImDrawList* draw_list = ImGui::GetWindowDrawList();
        const int shadow_layers = 15;
        const float shadow_offset = 8.0f;
        const float shadow_opacity = 1.0f;
        const float shadow_rounding = 12.0f;

        for (int i = 0; i < shadow_layers; i++)
        {
            float alpha = 1.0f;
            ImVec4 shadow_color = ImVec4(0.0f, 0.0f, 0.0f, alpha);

            draw_list->AddRectFilled(
                ImVec2(window_pos_.x - shadow_offset + i, window_pos_.y - shadow_offset + i),
                ImVec2(window_pos_.x + window_size.x + shadow_offset - i, window_pos_.y + window_size.y + shadow_offset - i),
                ImGui::GetColorU32(shadow_color),
                shadow_rounding
            );
        }

        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.80f, 0.80f, 0.80f, 1.00f));
        ImGui::Text(oxorany("Instance Explorer"));

        ImGui::PopStyleColor();

        ImGui::Separator();
        ImGui::Spacing();

        using Clock = std::chrono::steady_clock;
        static auto lastCacheRefresh = Clock::now();
        auto now = Clock::now();
        if (std::chrono::duration_cast<std::chrono::milliseconds>(now - lastCacheRefresh).count() > 2000) {
            cache.clear();
            lastCacheRefresh = now;
        }

        std::vector<uintptr_t> datamodel_children;
        if (cache.find(globals::datamodel) == cache.end()) {
            datamodel_children = utils::children(globals::datamodel);
            cache[globals::datamodel] = datamodel_children;
        }
        else {
            datamodel_children = cache[globals::datamodel];
        }

        for (auto& child : datamodel_children) {
            draw_instance_tree(child);
        }

        ImGui::End();
    }
}

std::unordered_map<uintptr_t, std::vector<uintptr_t>> cache;

void refreshCache() {
    using Clock = std::chrono::steady_clock;
    static auto lastCacheRefresh = Clock::now();
    auto now = Clock::now();
    if (std::chrono::duration_cast<std::chrono::milliseconds>(now - lastCacheRefresh).count() > 1000) {
        cache.clear();
        lastCacheRefresh = now;
    }
}

uintptr_t selected_instance = 0;

void CMenu::draw_instance_tree(uintptr_t instance)
{
    refreshCache();
    std::unordered_map<std::string, LPDIRECT3DTEXTURE9> class_icons = {
        {oxorany("Workspace"), workspace_image},
        {oxorany("Part"), part_image},
        {oxorany("MeshPart"), part_image},
        {oxorany("Model"), model_image},
        {oxorany("Camera"), camera_image},
        {oxorany("Folder"), folder_image},
        {oxorany("LocalScript"), local_script_image},
        {oxorany("Script"), script_image},
        {oxorany("Humanoid"), humanoid_image},
        {oxorany("Players"), players_image},
        {oxorany("Sound"), sound_image},
        {oxorany("Accessory"), accessory_image},
        {oxorany("Hat"), hat_image},
        {oxorany("Player"), player_image},
        {oxorany("ModuleScript"), module_script_image},
        {oxorany("ReplicatedStorage"), replicated_storage_image},
        {oxorany("RunService"), run_service_image},
        {oxorany("SpawnLocation"), spawn_location_image},
        {oxorany("ReplicatedFirst"), replicated_first_image},
        {oxorany("StarterGui"), starter_gui_image},
        {oxorany("StarterPack"), starter_pack_image},
        {oxorany("StarterPlayer"), starter_player_image},
        {oxorany("Stats"), stats_image},
        {oxorany("Chat"), chat_image},
        {oxorany("CoreGui"), core_gui_image},
        {oxorany("GuiService"), gui_service_image},
        {oxorany("DataStore"), data_store_image},
        {oxorany("RemoteEvent"), remote_event_image},
        {oxorany("RemoteFunction"), remote_function_image},
        {oxorany("UIListLayout"), ui_list_layout_image},
        {oxorany("TextLabel"), text_label_image},
        {oxorany("TextButton"), text_button_image},
        {oxorany("ImageLabel"), image_label_image},
        {oxorany("Frame"), frame_image},
        {oxorany("BillboardGui"), billboard_gui_image},
        {oxorany("SurfaceGui"), surface_gui_image},
        {oxorany("HttpRbxApiService"), http_rbx_api_service_image},
        {oxorany("InsertService"), insert_service_image},
    };

    static uintptr_t popup_instance = 0;

    ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.078f, 0.078f, 0.078f, 1.00f));
    ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(0.098f, 0.098f, 0.098f, 1.00f));
    ImGui::PushStyleColor(ImGuiCol_HeaderActive, ImVec4(0.118f, 0.118f, 0.118f, 1.00f));

    std::vector<uintptr_t> children_list;
    if (cache.find(instance) == cache.end()) {
        children_list = utils::children(instance);
        cache[instance] = children_list;
    }
    else {
        children_list = cache[instance];
    }

    std::string instance_name = utils::get_instance_name(instance);
    std::string class_name = utils::get_instance_classname(instance);
    std::string label = instance_name + oxorany("##") + std::to_string(instance);

    LPDIRECT3DTEXTURE9 icon = nullptr;
    if (class_icons.find(class_name) != class_icons.end()) {
        icon = class_icons[class_name];
    }

    if (children_list.empty()) {
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.5f, 0.5f, 0.5f, 1.0f));
    }

    ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow;
    if (selected_instance == instance) {
        flags |= ImGuiTreeNodeFlags_Selected;
    }
    if (children_list.empty()) {
        flags |= ImGuiTreeNodeFlags_Leaf;
    }

    bool node_open = ImGui::TreeNodeEx(label.c_str(), flags);

    if (ImGui::IsItemClicked(VK_LBUTTON)) {
        selected_instance = instance;
    }

    if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(VK_RBUTTON)) {
        popup_instance = instance;
        ImGui::SetNextWindowPos(ImGui::GetMousePos());
        ImGui::OpenPopup(oxorany("##InstancePropertiesPopup"));
    }

    if (icon) {
        ImGui::SameLine();
        ImGui::Image((ImTextureID)icon, ImVec2(15, 15));
    }

    if (popup_instance == instance) {
        if (ImGui::BeginPopup(oxorany("InstancePropertiesPopup"))) {
            ImGui::TextUnformatted((oxorany("Name: ") + instance_name).c_str());
            ImGui::TextUnformatted((oxorany("Class: ") + class_name).c_str());

            bool is_flagged = flagged_instances.contains(instance);
            if (ImGui::Checkbox(oxorany("Flag"), &is_flagged)) {
                if (is_flagged) {
                    flagged_instances.insert(instance);
                }
                else {
                    flagged_instances.erase(instance);
                }
            }

            if (class_name == oxorany("Part") || class_name == oxorany("MeshPart") || class_name == oxorany("SpawnLocation") || class_name == oxorany("UnionOperation")) {
                if (ImGui::Button(oxorany("Teleport"))) {
                    utils::teleport_to_part(globals::local_player, instance);
                }
            }

            if (class_name == oxorany("ProximityPrompt"))
            {
                uintptr_t holdDurations = read<uintptr_t>(instance + offsets::Primitive);
                static float holdDuration = read<float>(instance + offsets::ProximityPromptHoldDuraction);

                if (ImGui::SliderFloat(oxorany("Hold Duration"), &holdDuration, 0.0f, 10.0f)) {
                    write<float>(instance + offsets::ProximityPromptHoldDuraction, holdDuration);
                }

            }

            ImGui::EndPopup();
        }
    }

    if (node_open) {
        for (auto& child_address : children_list) {
            draw_instance_tree(child_address);
        }
        ImGui::TreePop();
    }

    if (children_list.empty()) {
        ImGui::PopStyleColor();
    }

    ImGui::PopStyleColor(3);
}

