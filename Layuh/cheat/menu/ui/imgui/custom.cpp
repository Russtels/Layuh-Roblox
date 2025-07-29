#include "custom.hpp"
#include <map>

struct tab_anim
{
    int hovered_anim;
    int active_anim;
};
bool custom::tab(const char* text, bool selected, ImFont* icon_font)
{
    ImGuiWindow* window = ImGui::GetCurrentWindow();
    if (window->SkipItems)
    {
        return false;
    }

    ImGuiContext& g = *GImGui;
    const ImGuiStyle& style = g.Style;
    const ImGuiID id = window->GetID(text);
    ImVec2 pos = window->DC.CursorPos;
    ImVec2 size = { ImGui::GetContentRegionAvail().x, 30 };
    const ImRect bb(pos, ImVec2(pos.x + size.x, pos.y + size.y));

    ImGui::ItemSize(size, 0);
    if (!ImGui::ItemAdd(bb, id))
    {
        return false;
    }

    bool hovered, held;
    bool pressed = ImGui::ButtonBehavior(bb, id, &hovered, &held, NULL);

    static std::map <ImGuiID, tab_anim> anim;
    auto it_anim = anim.find(id);
    if (it_anim == anim.end())
    {
        anim.insert({ id, {0, 0} });
        it_anim = anim.find(id);
    }

    if (hovered)
        it_anim->second.hovered_anim += 8 * (1.f - ImGui::GetIO().DeltaTime);
    else
        it_anim->second.hovered_anim -= 8 * (1.f - ImGui::GetIO().DeltaTime);

    if (it_anim->second.hovered_anim > 50)
        it_anim->second.hovered_anim = 50;
    else if (it_anim->second.hovered_anim < 0)
        it_anim->second.hovered_anim = 0;

    if (selected)
        it_anim->second.active_anim += 25 * (1.f - ImGui::GetIO().DeltaTime);
    else
        it_anim->second.active_anim -= 25 * (1.f - ImGui::GetIO().DeltaTime);

    if (it_anim->second.active_anim > 255)
        it_anim->second.active_anim = 255;
    else if (it_anim->second.active_anim < 0)
        it_anim->second.active_anim = 0;


    ImGui::GetWindowDrawList()->AddRect(bb.Min, bb.Max, ImGui::GetColorU32(ImGuiCol_Border), 5.0f);
    ImGui::GetWindowDrawList()->AddRectFilled(ImVec2(bb.Min.x, bb.Min.y), ImVec2(bb.Min.x + size.x, bb.Min.y + size.y), ImColor(133, 3, 35, it_anim->second.active_anim), 5.0f);

    ImVec2 textSize = ImGui::CalcTextSize(text);

    ImGui::PushFont(icon_font);
    ImGui::GetWindowDrawList()->AddText(ImVec2(bb.Min.x + 9, bb.Min.y + (size.y - textSize.y) / 2), selected ? ImColor(255, 255, 255, 255) : ImColor(155, 155, 155, 255), text);
    ImGui::PopFont();

    return pressed;
}

void custom::begin_child(const char* name, ImVec2 size)
{
    ImGuiWindow* window = ImGui::GetCurrentWindow();
    if (window->SkipItems)
    {
        return;
    }

    ImGui::BeginChild(std::string(name).append(".main").c_str(), size, true);

    ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(155, 155, 155, 255));
    ImGui::Text(name);
    ImGui::PopStyleColor();

    ImGui::Separator();

    ImGui::Dummy(ImVec2(0, 1.0f));
}

void custom::end_child()
{
    ImGui::EndChild();
}

struct checbox_anim
{
    int active_outline_anim;
    int active_text_anim;
    int active_rect_alpha;
};
bool custom::checkbox(const char* label, bool* v)
{
    ImGuiWindow* window = ImGui::GetCurrentWindow();
    if (window->SkipItems)
        return false;

    ImGuiContext& g = *GImGui;
    const ImGuiStyle& style = g.Style;
    const ImGuiID id = window->GetID(label);
    const ImVec2 label_size = ImGui::CalcTextSize(label, NULL, true);
    const float size = ImGui::GetWindowWidth() - 20;

    const float square_sz = ImGui::GetFrameHeight();
    const ImVec2 pos = window->DC.CursorPos;
    const ImRect rect(pos, pos + ImVec2(size, 15));
    ImGui::ItemSize(rect, style.FramePadding.y);
    if (!ImGui::ItemAdd(rect, id))
    {
        IMGUI_TEST_ENGINE_ITEM_INFO(id, label, g.LastItemData.StatusFlags | ImGuiItemStatusFlags_Checkable | (*v ? ImGuiItemStatusFlags_Checked : 0));
        return false;
    }

    bool hovered, held;
    bool pressed = ImGui::ButtonBehavior(rect, id, &hovered, &held);
    if (pressed)
    {
        *v = !(*v);
        ImGui::MarkItemEdited(id);
    }

    static std::map <ImGuiID, checbox_anim> anim;
    auto it_anim = anim.find(id);
    if (it_anim == anim.end())
    {
        anim.insert({ id, {0, 0, 0} });
        it_anim = anim.find(id);
    }

    if (*v)
        it_anim->second.active_outline_anim += 3 * (1.f - ImGui::GetIO().DeltaTime);
    else
        it_anim->second.active_outline_anim -= 3 * (1.f - ImGui::GetIO().DeltaTime);

    if (it_anim->second.active_outline_anim > 20)
        it_anim->second.active_outline_anim = 20;
    else if (it_anim->second.active_outline_anim < 0)
        it_anim->second.active_outline_anim = 0;

    if (*v)
        it_anim->second.active_text_anim += 8 * (1.f - ImGui::GetIO().DeltaTime);
    else
        it_anim->second.active_text_anim -= 8 * (1.f - ImGui::GetIO().DeltaTime);

    if (it_anim->second.active_text_anim > 100)
        it_anim->second.active_text_anim = 100;
    else if (it_anim->second.active_text_anim < 0)
        it_anim->second.active_text_anim = 0;

    if (*v)
        it_anim->second.active_rect_alpha += 25 * (1.f - ImGui::GetIO().DeltaTime);
    else
        it_anim->second.active_rect_alpha -= 25 * (1.f - ImGui::GetIO().DeltaTime);

    if (it_anim->second.active_rect_alpha > 255)
        it_anim->second.active_rect_alpha = 255;
    else if (it_anim->second.active_rect_alpha < 0)
        it_anim->second.active_rect_alpha = 0;


    window->DrawList->AddRectFilled(rect.Max - ImVec2(15, 15), rect.Max, ImGui::GetColorU32(ImGuiCol_FrameBg), 2.0f);

    window->DrawList->AddRectFilled(rect.Max - ImVec2(15, 15), rect.Max, ImColor(133, 3, 35, it_anim->second.active_rect_alpha), 2.0f);

    ImGui::RenderCheckMark(rect.Max - ImVec2(12, 12), ImColor(255, 255, 255, it_anim->second.active_rect_alpha), 9.0f);

    window->DrawList->AddRect(rect.Max - ImVec2(15, 15), rect.Max, ImColor(255, 255, 255, (15 + it_anim->second.active_outline_anim)), 2.0f);

    if (*v)
    {
        window->DrawList->AddText(rect.Min + ImVec2(0, 1), ImColor(255, 255, 255, (155 + it_anim->second.active_text_anim)), label);
    }
    else
    {
        window->DrawList->AddText(rect.Min + ImVec2(0, 1), ImColor(155, 155, 155, (155 + it_anim->second.active_text_anim)), label);
    }

    return true;
}

void custom::selector(const char* label, int& value, int max_value, const char* labels[], int label_count)
{
    ImGui::BeginGroup();
    {
        ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(155, 155, 155, 255));
        ImGui::Text(label);
        ImGui::PopStyleColor();

        if (ImGui::Button("<"))
        {
            value -= 1;
            if (value < 0)
            {
                value = max_value;
            }
        }

        ImGui::SameLine();

        ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(155, 155, 155, 255));
        if (value >= 0 && value <= label_count)
        {
            ImGui::Text(labels[value]);
        }
        ImGui::PopStyleColor();

        ImGui::SameLine();

        if (ImGui::Button(">"))
        {
            value += 1;
            if (value > max_value)
            {
                value = 0;
            }
        }
    }
    ImGui::EndGroup();
}