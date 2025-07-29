#include "Drawing.h"

void Drawing::text(std::string text, ImVec2 pos, ImColor color)
{
    auto draw_list = ImGui::GetOverlayDrawList();

    ImVec2 outline_offset = ImVec2(1.0f, 1.0f);
    ImColor outline_color = ImColor(1, 1, 1, 255);

    draw_list->AddText(ImVec2(pos.x + outline_offset.x, pos.y + outline_offset.y), outline_color, text.c_str());
    draw_list->AddText(ImVec2(pos.x - outline_offset.x, pos.y - outline_offset.y), outline_color, text.c_str());
    draw_list->AddText(ImVec2(pos.x + outline_offset.x, pos.y - outline_offset.y), outline_color, text.c_str());
    draw_list->AddText(ImVec2(pos.x - outline_offset.x, pos.y + outline_offset.y), outline_color, text.c_str());

    draw_list->AddText(pos, color, text.c_str());
}

void Drawing::line(const ImVec2& from, const ImVec2& to, const ImColor& color, float thickness)
{
    ImGui::GetOverlayDrawList()->AddLine(from, to, ImGui::GetColorU32(ImVec4(color.Value.x, color.Value.y, color.Value.z, color.Value.w)), thickness);
}

void Drawing::outlined_line(const ImVec2& from, const ImVec2& to, ImColor line_color, ImColor outline_color, float thickness = 1.0f)
{
    auto draw_list = ImGui::GetOverlayDrawList();

    draw_list->AddLine(ImVec2(from.x - 1, from.y - 1), ImVec2(to.x - 1, to.y - 1), outline_color, 0.1f);
    draw_list->AddLine(ImVec2(from.x + 1, from.y + 1), ImVec2(to.x + 1, to.y + 1), outline_color, 0.1f);
    draw_list->AddLine(ImVec2(from.x - 1, from.y + 1), ImVec2(to.x - 1, to.y + 1), outline_color, 0.1f);
    draw_list->AddLine(ImVec2(from.x + 1, from.y - 1), ImVec2(to.x + 1, to.y - 1), outline_color, 0.1f);

    draw_list->AddLine(from, to, line_color, thickness);
}

void Drawing::box(ImVec2 top_left, ImVec2 bottom_right, ImColor color, float thickness = 1.0f)
{
    auto draw_list = ImGui::GetOverlayDrawList();
    draw_list->AddRect(top_left, bottom_right, color, 0.0f, 0, thickness);
}

void Drawing::circle(ImVec2 center, float radius, ImColor color, float thickness = 1.0f) {
    auto draw_list = ImGui::GetOverlayDrawList();
    draw_list->AddCircle(center, radius, color, 0, thickness);
}


void Drawing::outlined_box(ImVec2 top_left, ImVec2 bottom_right, ImColor box_color, ImColor outline_color, float thickness = 1.0f)
{
    auto draw_list = ImGui::GetOverlayDrawList();

    draw_list->AddRect(ImVec2(top_left.x - 1, top_left.y - 1), ImVec2(bottom_right.x + 1, bottom_right.y + 1), outline_color, 0.0f, 0, thickness + 1.5f);
    draw_list->AddRect(ImVec2(top_left.x + 1, top_left.y + 1), ImVec2(bottom_right.x - 1, bottom_right.y - 1), outline_color, 0.0f, 0, thickness + 1.5f);

    draw_list->AddRect(top_left, bottom_right, box_color, 0.0f, 0, thickness);
}



void Drawing::filled_box(ImVec2 top_left, ImVec2 bottom_right, ImColor fill_color)
{
    auto draw_list = ImGui::GetOverlayDrawList();

    draw_list->AddRectFilled(top_left, bottom_right, fill_color, 0.0f, 0);
}

void Drawing::hotkey(const char* label, int* key, int* mode, ImVec2 size) {
    ImGui::PushID(label);
    ImGuiID id = ImGui::GetID("hotkey");

    static std::unordered_map<ImGuiID, bool> capturingMap;
    bool& capturing = capturingMap[id];

    auto GetKeyName = [key](int keyCode) -> const char* {
        switch (keyCode) {
        case VK_LBUTTON:   return "mouse1";
        case VK_RBUTTON:   return "mouse2";
        case VK_MBUTTON:   return "mouse3";
        case VK_XBUTTON1:  return "mouse4";
        case VK_XBUTTON2:  return "mouse5";
        case VK_SHIFT:     return "shift";
        case VK_CONTROL:   return "ctrl";
        case VK_MENU:      return "alt";
        case VK_LWIN:      return "left win";
        case VK_RWIN:      return "right win";
        case VK_TAB:       return "tab";
        case VK_CAPITAL:   return "caps lock";
        case VK_ESCAPE:    return "escape";
        case VK_SPACE:     return "space";
        case VK_RETURN:    return "enter";
        case VK_BACK:      return "backspace";
        case VK_INSERT:    return "insert";
        case VK_DELETE:    return "delete";
        case VK_END:       return "end";
        case VK_HOME:      return "home";
        case VK_LEFT:      return "left arrow";
        case VK_UP:        return "up arrow";
        case VK_RIGHT:     return "right arrow";
        case VK_DOWN:      return "down arrow";
        case VK_F1:        return "f1";
        case VK_F2:        return "f2";
        case VK_F3:        return "f3";
        case VK_F4:        return "f4";
        case VK_F5:        return "f5";
        case VK_F6:        return "f6";
        case VK_F7:        return "f7";
        case VK_F8:        return "f8";
        case VK_F9:        return "f9";
        case VK_F10:       return "f10";
        case VK_F11:       return "f11";
        case VK_F12:       return "f12";
        default:
            if (*key > 0 && *key < 256) {
                static char buffer[2];
                buffer[0] = static_cast<char>(*key);
                buffer[1] = '\0';
                return buffer;
            }
            return "unknown key";
        }
        };

    if (ImGui::Button(capturing ? "..." : *key ? GetKeyName(*key) : "Set Hotkey", size)) {
        if (!capturing) {
            capturing = true;
        }
    }

    if (capturing) {
        bool keyCaptured = false;
        for (int i = 0; i < 256; ++i) {
            if (GetAsyncKeyState(i) & 0x8000) {
                *key = i;
                keyCaptured = true;
                break;
            }
        }
        if (keyCaptured) {
            capturing = false;
        }
    }

    if (ImGui::BeginPopupContextItem("HotkeyContextMenu")) {
        if (ImGui::Selectable("Hold")) {
            *mode = 0;
        }
        if (ImGui::Selectable("Toggle ")) {
            *mode = 1;
        }
        ImGui::EndPopup();
    }

    if (ImGui::IsItemClicked(VK_RBUTTON)) {
        ImGui::OpenPopup("HotkeyContextMenu");
    }

    ImGui::SameLine();
    ImGui::Text("%s [%s]", label, *mode == 0 ? "Hold" : "Toggle");

    ImGui::PopID();
}

void Drawing::hotkey_no_modes(const char* label, int* key, ImVec2 size) {
    ImGui::PushID(label);
    ImGuiID id = ImGui::GetID("hotkey");

    static std::unordered_map<ImGuiID, bool> capturingMap;
    bool& capturing = capturingMap[id];

    auto GetKeyName = [](int keyCode) -> const char* {
        switch (keyCode) {
        case VK_LBUTTON:   return "mouse1";
        case VK_RBUTTON:   return "mouse2";
        case VK_MBUTTON:   return "mouse3";
        case VK_XBUTTON1:  return "mouse4";
        case VK_XBUTTON2:  return "mouse5";
        case VK_SHIFT:     return "shift";
        case VK_CONTROL:   return "ctrl";
        case VK_MENU:      return "alt";
        case VK_LWIN:      return "left win";
        case VK_RWIN:      return "right win";
        case VK_TAB:       return "tab";
        case VK_CAPITAL:   return "caps lock";
        case VK_ESCAPE:    return "escape";
        case VK_SPACE:     return "space";
        case VK_RETURN:    return "enter";
        case VK_BACK:      return "backspace";
        case VK_INSERT:    return "insert";
        case VK_DELETE:    return "delete";
        case VK_END:       return "end";
        case VK_HOME:      return "home";
        case VK_LEFT:      return "left arrow";
        case VK_UP:        return "up arrow";
        case VK_RIGHT:     return "right arrow";
        case VK_DOWN:      return "down arrow";
        case VK_F1:        return "f1";
        case VK_F2:        return "f2";
        case VK_F3:        return "f3";
        case VK_F4:        return "f4";
        case VK_F5:        return "f5";
        case VK_F6:        return "f6";
        case VK_F7:        return "f7";
        case VK_F8:        return "f8";
        case VK_F9:        return "f9";
        case VK_F10:       return "f10";
        case VK_F11:       return "f11";
        case VK_F12:       return "f12";
        default:
            if (keyCode > 0 && keyCode < 256) {
                static char buffer[2];
                buffer[0] = static_cast<char>(keyCode);
                buffer[1] = '\0';
                return buffer;
            }
            return "unknown key";
        }
        };

    if (ImGui::Button(capturing ? "..." : (*key ? GetKeyName(*key) : "Set Hotkey"), size)) {
        capturing = true;
    }

    if (capturing) {
        bool keyCaptured = false;
        for (int i = 0; i < 256; ++i) {
            if (GetAsyncKeyState(i) & 0x8000) {
                *key = i;
                keyCaptured = true;
                break;
            }
        }
        if (keyCaptured) {
            capturing = false;
        }
    }

    ImGui::SameLine();
    ImGui::Text("%s", label);

    ImGui::PopID();
}
