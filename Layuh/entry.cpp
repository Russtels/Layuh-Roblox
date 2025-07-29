#include "entry.h"
#include "cheat/aimbot/aimbot.h"
#include "cheat/globals/roblox.h"
#include <wininet.h>
#pragma comment(lib, "wininet.lib")
#include "utils.hpp"
#include <iostream>
#include "auth.hpp"

void sessionStatus();

using namespace KeyAuth;
std::string name = oxorany("Astraxproducts's Application");
std::string ownerid = oxorany("FyeaZV8G67");
std::string version = oxorany("1.0");
std::string url = oxorany("https://keyauth.win/api/1.3/"); 
std::string path = oxorany("");
api KeyAuthApp(name, ownerid, version, url, path);

std::string readstring(std::uint64_t address)
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

void sessionStatus() {
    KeyAuthApp.check(true);
    if (!KeyAuthApp.response.success) {
        exit(0);
    }

    if (KeyAuthApp.response.isPaid) {
        while (true) {
            Sleep(20000);
            KeyAuthApp.check();
            if (!KeyAuthApp.response.success) {
                exit(0);
            }
        }
    }
}

struct Notification {
    std::string message;
    ImVec4 color;
    float creation_time;
    float duration;
    float alpha;
    float y_offset;
    bool removing;
};

class NotificationSystem {
private:
    std::vector<Notification> notifications;
    float padding = 10.0f;
    float margin = 10.0f;
    float fade_in_time = 0.15f;
    float fade_out_time = 0.15f;
    float animation_speed = 500.0f;

public:
    void add_notification(const std::string& message, ImVec4 color = ImVec4(1.0f, 1.0f, 1.0f, 1.0f), float duration = 3.0f) {
        Notification notif;
        notif.message = message;
        notif.color = color;
        notif.creation_time = ImGui::GetTime();
        notif.duration = duration;
        notif.alpha = 0.0f;
        notif.y_offset = 0.0f;
        notif.removing = false;
        notifications.push_back(notif);
    }

    void render() {
        if (notifications.empty())
            return;

        float current_time = ImGui::GetTime();
        ImVec2 screen_size = ImGui::GetIO().DisplaySize;
        float current_height = screen_size.y - margin;

        ImDrawList* draw_list = ImGui::GetOverlayDrawList();

        for (int i = notifications.size() - 1; i >= 0; i--) {
            auto& notif = notifications[i];

            float elapsed = current_time - notif.creation_time;

            if (elapsed > notif.duration && !notif.removing) {
                notif.removing = true;
            }

            if (notif.removing) {
                notif.alpha = ImMax(0.0f, notif.alpha - ImGui::GetIO().DeltaTime / fade_out_time);
                notif.y_offset += ImGui::GetIO().DeltaTime * animation_speed;

                if (notif.alpha <= 0.0f) {
                    notifications.erase(notifications.begin() + i);
                    continue;
                }
            }
            else {
                notif.alpha = ImMin(1.0f, elapsed / fade_in_time);
                notif.y_offset = ImMax(0.0f, notif.y_offset - ImGui::GetIO().DeltaTime * animation_speed);
            }

            ImVec2 text_size = ImGui::CalcTextSize(notif.message.c_str());
            float width = text_size.x + padding * 2;
            float max_width = screen_size.x * 0.3f;

            if (width > max_width) {
                width = max_width;

                ImVec2 text_size_wrapped;
                float wrap_width = max_width - padding * 2;
                float wrap_spacing = 4.0f;

                const char* text_begin = notif.message.c_str();
                const char* text_end = text_begin + notif.message.length();

                ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, wrap_spacing));
                text_size_wrapped = ImGui::CalcTextSize(oxorany("A"));
                float line_height = text_size_wrapped.y + wrap_spacing;

                int line_count = 1;
                float line_width = 0.0f;
                for (const char* s = text_begin; s < text_end; s++) {
                    char c = *s;
                    if (c == '\n') {
                        line_count++;
                        line_width = 0;
                    }
                    else {
                        line_width += ImGui::CalcTextSize(&c, &c + 1).x;
                        if (line_width > wrap_width) {
                            line_count++;
                            line_width = 0;
                        }
                    }
                }

                ImGui::PopStyleVar();

                float height = line_count * line_height + padding * 2;
                ImVec2 window_pos(screen_size.x - width - margin + notif.y_offset, current_height - height);

                draw_list->AddRectFilled(
                    ImVec2(window_pos.x + 2, window_pos.y + 2),
                    ImVec2(window_pos.x + width + 2, window_pos.y + height + 2),
                    ImColor(0.0f, 0.0f, 0.0f, notif.alpha * 0.25f),
                    4.0f
                );

                draw_list->AddRectFilled(
                    window_pos,
                    ImVec2(window_pos.x + width, window_pos.y + height),
                    ImColor(0.08f, 0.08f, 0.08f, notif.alpha * 0.9f),
                    4.0f
                );

                draw_list->AddRect(
                    window_pos,
                    ImVec2(window_pos.x + width, window_pos.y + height),
                    ImColor(0.3f, 0.3f, 0.3f, notif.alpha),
                    4.0f,
                    0,
                    1.5f
                );

                draw_list->AddRectFilled(
                    window_pos,
                    ImVec2(window_pos.x + 3.0f, window_pos.y + height),
                    ImColor(0.0f, 0.74f, 0.76f, notif.alpha)
                );

                ImGui::PushClipRect(
                    window_pos,
                    ImVec2(window_pos.x + width, window_pos.y + height),
                    true
                );

                ImGui::PushTextWrapPos(window_pos.x + width - padding);
                draw_list->AddText(
                    ImVec2(window_pos.x + padding, window_pos.y + padding),
                    ImColor(notif.color.x, notif.color.y, notif.color.z, notif.alpha),
                    notif.message.c_str()
                );
                ImGui::PopTextWrapPos();
                ImGui::PopClipRect();

                current_height -= (height + margin);
            }
            else {
                float height = text_size.y + padding * 2;
                ImVec2 window_pos(screen_size.x - width - margin + notif.y_offset, current_height - height);

                draw_list->AddRectFilled(
                    ImVec2(window_pos.x + 2, window_pos.y + 2),
                    ImVec2(window_pos.x + width + 2, window_pos.y + height + 2),
                    ImColor(0.0f, 0.0f, 0.0f, notif.alpha * 0.25f),
                    4.0f
                );

                draw_list->AddRectFilled(
                    window_pos,
                    ImVec2(window_pos.x + width, window_pos.y + height),
                    ImColor(0.08f, 0.08f, 0.08f, notif.alpha * 0.9f),
                    4.0f
                );

                draw_list->AddRect(
                    window_pos,
                    ImVec2(window_pos.x + width, window_pos.y + height),
                    ImColor(0.3f, 0.3f, 0.3f, notif.alpha),
                    4.0f,
                    0,
                    1.5f
                );

                draw_list->AddRectFilled(
                    window_pos,
                    ImVec2(window_pos.x + 3.0f, window_pos.y + height),
                    ImColor(0.0f, 0.74f, 0.76f, notif.alpha)
                );

                draw_list->AddText(
                    ImVec2(window_pos.x + padding, window_pos.y + padding),
                    ImColor(notif.color.x, notif.color.y, notif.color.z, notif.alpha),
                    notif.message.c_str()
                );

                current_height -= (height + margin);
            }
        }
    }
};

inline NotificationSystem g_notification_system;


void notify(const std::string& message, ImVec4 color) {
    g_notification_system.add_notification(message, color, 5.0f);
}

void render_notifications() {
    g_notification_system.render();
}

auto base = 0; 

uint64_t GetDataModel() {
    uintptr_t scheduler = read<uintptr_t>(drv::GetBase() + offsets::TaskSchedulerPointer);
    if (!scheduler) {
        return 0; 
    }

    uintptr_t jobStart = read<uintptr_t>(scheduler + offsets::JobStart);
    uintptr_t jobEnd = read<uintptr_t>(scheduler + offsets::JobEnd);

    if (!jobStart || !jobEnd || jobStart >= jobEnd) return 0;

    for (uintptr_t job = jobStart; job < jobEnd; job += 0x10) {
        uintptr_t jobAddress = read<uintptr_t>(job);

        if (!jobAddress) continue;

        std::string jobName = readstring(jobAddress + offsets::Job_Name);

        if (jobName == oxorany("RenderJob")) {
            auto RenderView = read<uintptr_t>(jobAddress + offsets::RenderJobToRenderView);

            if (!RenderView) {
                continue;
            }

            globals::visual_engine = read<uintptr_t>(RenderView + offsets::VisualEngine);
			uintptr_t BaseAddr = drv::GetBase();

            uintptr_t FakeDataModel = read<uintptr_t>(BaseAddr + offsets::FakeDataModelPointer);

            if (!FakeDataModel) {
                continue;
            }

            auto realDataModel = static_cast<uintptr_t>(read<std::uint64_t>(FakeDataModel + offsets::FakeDataModelToDataModel));
            if (!realDataModel) {
                continue;
            }

            globals::datamodel = realDataModel;
            return jobAddress;
        }
    }
    return 0;
}

void rescan_thread()
{
    std::uint64_t last_place_id = 0;

    if (globals::datamodel)
        last_place_id = read<std::uint64_t>(globals::datamodel + offsets::PlaceId);

    while (true)
    {
        std::this_thread::sleep_for(std::chrono::seconds(1));

        uintptr_t scheduler = read<uintptr_t>(drv::GetBase() + offsets::TaskSchedulerPointer);
        if (!scheduler)
            continue;

        uintptr_t jobStart = read<uintptr_t>(scheduler + offsets::JobStart);
        uintptr_t jobEnd = read<uintptr_t>(scheduler + offsets::JobEnd);

        if (!jobStart || !jobEnd || jobStart >= jobEnd)
            continue;

        uintptr_t datamodel = 0;

        for (uintptr_t job = jobStart; job < jobEnd; job += 0x10)
        {
            uintptr_t jobAddress = read<uintptr_t>(job);
            if (!jobAddress)
                continue;

            std::string jobName = readstring(jobAddress + offsets::Job_Name);

            if (jobName == oxorany("RenderJob"))
            {
                auto renderView = read<uintptr_t>(jobAddress + offsets::RenderJobToRenderView);
                if (!renderView)
                    continue;

                uintptr_t BaseAddr = drv::GetBase();

                uintptr_t FakeDataModel = read<uintptr_t>(BaseAddr + offsets::FakeDataModelPointer);

                if (!FakeDataModel) {
                    continue;
                }

                datamodel = read<std::uint64_t>(FakeDataModel + offsets::FakeDataModelToDataModel);
                if (datamodel)
                    break;
            }
        }

        if (!datamodel)
            continue;

        std::uint64_t current_place_id = read<std::uint64_t>(datamodel + offsets::PlaceId);

        if (current_place_id == 0)
            continue;

        if (datamodel != globals::datamodel || current_place_id != last_place_id)
        {
            drv::procid = drv::FindProcess(oxorany("RobloxPlayerBeta.exe"));
            auto base = drv::GetBase();
            auto RobloxPlayerDLL = drv::get_module(oxorany(L"RobloxPlayerBeta.dll"));
            globals::datamodel = datamodel;
            globals::visual_engine = read<uintptr_t>(drv::GetBase() + offsets::VisualEnginePointer);

            uintptr_t players = utils::find_first_child_byclass(globals::datamodel, oxorany("Players"));
            if (!players)
                continue;

            uintptr_t new_local_player = 0;
            int attempts = 0;

            while (!new_local_player && attempts < 15)
            {
                new_local_player = read<uintptr_t>(players + offsets::LocalPlayer);
                if (!new_local_player)
                    std::this_thread::sleep_for(std::chrono::milliseconds(100));
                attempts++;
            }

            if (new_local_player)
                globals::local_player = new_local_player;

            last_place_id = current_place_id;
        }
    }
}

std::string http_get(const std::string& url) {
    HINTERNET hInternet = InternetOpenA(oxorany("Mozilla/5.0"), INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);
    if (!hInternet) return "";

    HINTERNET hFile = InternetOpenUrlA(hInternet, url.c_str(), NULL, 0, INTERNET_FLAG_RELOAD, 0);
    if (!hFile) {
        InternetCloseHandle(hInternet);
        return "";
    }

    char buffer[4096];
    DWORD bytesRead;
    std::string result;
    while (InternetReadFile(hFile, buffer, sizeof(buffer) - 1, &bytesRead) && bytesRead) {
        buffer[bytesRead] = 0;
        result += buffer;
    }

    InternetCloseHandle(hFile);
    InternetCloseHandle(hInternet);
    return result;
}

void spinner_message(const char* message, int duration_ms = 2000, int interval_ms = 100) {
    const char icons[] = { '/', '-', '\\', '|' };
    int icon_count = sizeof(icons) / sizeof(icons[0]);
    int steps = duration_ms / interval_ms;
    for (int i = 0; i < steps; ++i) {
        printf(oxorany("\r%c %s"), icons[i % icon_count], message);
        fflush(stdout);
        std::this_thread::sleep_for(std::chrono::milliseconds(interval_ms));
    }
    printf(oxorany("\r%s\r"), std::string(strlen(message) + 2, ' ').c_str());
}

int main()
{
    utils::initialize_console();
    offsets::autoupdate();

    std::string title = oxorany("Kryptex External - https://discord.gg/StZUmwDQnJ - ");
    if (!offsets::RobloxVersionString.empty())
        title += offsets::RobloxVersionString;
    SetConsoleTitleA(title.c_str()); 

    spinner_message(oxorany("Initializing to roblox"), 1000, 100);

    std::string localappdata;
    char* lp = nullptr;
    size_t sz = 0;
    _dupenv_s(&lp, &sz, oxorany("LOCALAPPDATA"));
    if (lp) {
        localappdata = lp;
        free(lp);
    }

    std::string config_path = localappdata + oxorany("\\Roblox");
    std::filesystem::create_directories(config_path);
    printf(oxorany("Setting up Folders\n"));

    printf(oxorany("Successfully created Config folder at [ %s ]\n"), config_path.c_str());

    spinner_message(oxorany("Finishing.."), 1000, 100);

    while (drv::procid == 0) {
        printf(oxorany("Waiting for Roblox to start...\n"));
        drv::procid = drv::FindProcess(oxorany("RobloxPlayerBeta.exe"));
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        system(oxorany("cls"));
    }

    drv::procid = drv::FindProcess(oxorany("RobloxPlayerBeta.exe"));

    if (!drv::Init()) {
        utils::console_print_color(__FILE__, oxorany("Failed to open process handle.\n"));
        return 1;
    }

    auto base = drv::GetBase();
    auto RobloxPlayerDLL = drv::get_module(oxorany(L"RobloxPlayerBeta.dll"));

    GetDataModel();

    auto place_id = read<std::uint64_t>(globals::datamodel + offsets::PlaceId);
    auto local_player = read<uintptr_t>(utils::find_first_child_byclass(globals::datamodel, oxorany("Players")) + offsets::LocalPlayer);

    globals::local_player = local_player;

    std::thread([&]() { for (;;) { rescan_thread(); } }).detach();

    overlay::render();

    return 0;
}
