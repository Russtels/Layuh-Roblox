#include "utils.h"
#include <unordered_map>

namespace utils {
    bool initialize_console(...)
    {
        char t[9];
        for (int i = 0; i < 8; i++)
            t[i] = (rand() % 36 < 10) ? ('0' + rand() % 10) : ('a' + rand() % 26);
        t[8] = '\0';

        std::random_device rd;
        std::mt19937 g(rd());
        std::shuffle(t, t + 8, g);

        wchar_t wt[9];
        MultiByteToWideChar(CP_UTF8, 0, t, -1, wt, 9);

        HANDLE h_stdout = GetStdHandle(STD_OUTPUT_HANDLE);
        DWORD mode;
        if (GetConsoleMode(h_stdout, &mode))
        {
            SetConsoleMode(h_stdout, mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
            //SetConsoleTitleW(wt);

            return true;
        }

        return false;
    }

    void return_message_box(LPCSTR string, ...)
    {
        MessageBeep(0);
        MessageBoxA(0, string, 0, 0);
    }

    void get_current_time(char* buffer, size_t buffer_size)
    {
        time_t rawtime;
        struct tm timeinfo;

        time(&rawtime);
        localtime_s(&timeinfo, &rawtime);

        strftime(buffer, buffer_size, "%H:%M:%S", &timeinfo);
    }

    const char* get_filename(const char* path)
    {
        const char* filename = path;
        while (*path)
        {
            if (*path == '/' || *path == '\\')
                filename = path + 1;
            path++;
        }
        return filename;
    }

    void console_print_success(const char* file, const char* string, ...)
    {
        char time_str[9];
        get_current_time(time_str, sizeof(time_str));

        printf("\033[38;2;192;192;192m[%s] [%s]\033[0m\033[38;2;0;255;0m [Success]: \033[0m", time_str, get_filename(file));

        va_list args;
        va_start(args, string);
        vprintf(string, args);
        va_end(args);
        printf("\n");
    }


    void console_print_color(const char* file, const char* string, ...)
    {
        char time_str[9];
        get_current_time(time_str, sizeof(time_str));

        printf("\033[38;2;192;192;192m[%s] [%s]\033[0m\033[38;2;97;192;255m [Debug]: \033[0m", time_str, get_filename(file));

        va_list args;
        va_start(args, string);
        vprintf(string, args);
        va_end(args);
        printf("\n");
    }

    void console_print_logo(const char* file, const char* string, ...)
    {
        char time_str[9];
        get_current_time(time_str, sizeof(time_str));

        va_list args;
        va_start(args, string);
        vprintf(string, args);
        va_end(args);
        printf("");
    }

    void console_print_rescan(const char* file, const char* string, ...)
    {
        char time_str[9];
        get_current_time(time_str, sizeof(time_str));

        printf("\033[38;2;192;192;192m[%s] [%s]\033[0m\033[38;2;153;255;199m [Rescan]: \033[0m", time_str, get_filename(file));

        va_list args;
        va_start(args, string);
        vprintf(string, args);
        va_end(args);
        printf("\n");
    }

    std::string read_string(uintptr_t address)
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

    std::string length_read_string(uintptr_t string)
    {
        const auto length = read<int>(string + offsets::ClassDescriptor);

        if (length >= 16u)
        {
            const auto _new = read<uintptr_t>(string);
            return read_string(_new);
        }
        else
        {
            return read_string(string);
        }
    }

    std::string get_instance_name(uintptr_t instance_address)
    {
        const auto _get = read<uintptr_t>(instance_address + offsets::Name);

        if (_get)
            return length_read_string(_get);

        return "???";
    }

    std::string get_instance_classname(uintptr_t instance_address)
    {
        const auto ptr = read<uintptr_t>(instance_address + offsets::ClassDescriptor);
        const auto ptr2 = read<uintptr_t>(ptr + offsets::ChildrenEnd);

        if (ptr2)
            return read_string(ptr2);

        return "???";
    }

    uintptr_t find_first_child(uintptr_t instance_address, const std::string& child_name)
    {
        if (!instance_address)
            return 0;

        static std::unordered_map<uintptr_t, std::vector<std::pair<uintptr_t, std::string>>> cache;
        static std::unordered_map<uintptr_t, std::chrono::steady_clock::time_point> last_update;

        auto now = std::chrono::steady_clock::now();
        auto& children = cache[instance_address];
        auto& update_time = last_update[instance_address];

        if (children.empty() || now - update_time > std::chrono::seconds(2))
        {
            children.clear();
            try {
                auto start = read<uintptr_t>(instance_address + offsets::Children);
                if (!start)
                    return 0;

                auto end = read<uintptr_t>(start + offsets::ChildrenEnd);

                auto childArray = read<uintptr_t>(start);
                if (!childArray || childArray >= end)
                    return 0;

                for (uintptr_t current = childArray; current < end; current += 16)
                {
                    auto child_instance = read<uintptr_t>(current);
                    if (!child_instance)
                        continue;
                    std::string name = get_instance_name(child_instance);
                    children.emplace_back(child_instance, name);
                }
            }
            catch (...) {
                return 0;
            }
            update_time = now;
        }

        for (const auto& [child_instance, name] : children)
        {
            if (name == child_name)
                return child_instance;
        }

        return 0;
    }


    uintptr_t find_first_child_byclass(uintptr_t instance_address, const std::string& child_class)
    {
        if (!instance_address)
            return 0;

        static std::unordered_map<uintptr_t, std::vector<std::pair<uintptr_t, std::string>>> cache;
        static std::unordered_map<uintptr_t, std::chrono::steady_clock::time_point> last_update;

        auto now = std::chrono::steady_clock::now();
        auto& children = cache[instance_address];
        auto& update_time = last_update[instance_address];

        if (children.empty() || now - update_time > std::chrono::seconds(2))
        {
            children.clear();
            try {
                auto start = read<uintptr_t>(instance_address + offsets::Children);
                if (!start)
                    return 0;

                auto end = read<uintptr_t>(start + offsets::ChildrenEnd);
                auto childArray = read<uintptr_t>(start);
                if (!childArray || childArray >= end)
                    return 0;

                for (uintptr_t current = childArray; current < end; current += 16)
                {
                    auto child_instance = read<uintptr_t>(current);
                    if (!child_instance)
                        continue;
                    std::string classname = utils::get_instance_classname(child_instance);
                    children.emplace_back(child_instance, classname);
                }
            }
            catch (...) {
                return 0;
            }
            update_time = now;
        }

        for (const auto& [child_instance, classname] : children)
        {
            if (classname == child_class)
                return child_instance;
        }

        return 0;
    }


    uintptr_t get_model_instance(uintptr_t instance_address)
    {
        return read<uintptr_t>(instance_address + offsets::ModelInstance);
    }

    std::vector<uintptr_t> children(uintptr_t instance_address)
    {
        std::vector<uintptr_t> container;

        if (!instance_address)
            return container;

        auto start = read<uintptr_t>(instance_address + offsets::Children);

        if (!start)
            return container;

        auto end = read<uintptr_t>(start + offsets::ChildrenEnd);

        for (auto instances = read<uintptr_t>(start); instances != end; instances += 16)
            container.emplace_back(read<uintptr_t>(instances));

        return container;
    }

    std::vector< uintptr_t > get_players(uintptr_t datamodel_address) {

        std::vector< uintptr_t > player_instances;

        auto players_instance = utils::find_first_child_byclass(datamodel_address, "Players");
        if (!players_instance)
            return player_instances;

        auto children_addresses = children(players_instance);

        player_instances.reserve(children_addresses.size());

        for (const auto& child : children_addresses) {
            if (utils::get_instance_classname(child) == "Player") {
                player_instances.push_back(child);
            }
        }

        return player_instances;
    }

    void change_fov(uint64_t _datamodel, float _fov) {
        if (!_datamodel || !_fov) return;

        uint64_t workspace = utils::find_first_child(_datamodel, "Workspace");
        uint64_t camera = read< uint64_t >(workspace + offsets::FOV);

        for (int i = 0; i < 5; ++i) {
            write< float >(camera + offsets::Position, 1.9);
        }
    }

    void change_viewangles(uint64_t _datamodel, Vector shit) {
        if (!_datamodel) return;

        uint64_t workspace = utils::find_first_child(_datamodel, "Workspace");
        uint64_t camera = read< uint64_t >(workspace + offsets::FOV);

        for (int i = 0; i < 200; ++i) {
            write< Vector >(camera + 0xFC, Vector(0, 0));
        }
    }

    void teleport_to_part(uintptr_t localplayer, uintptr_t part) {
        if (!localplayer || !part)
            return;

        uintptr_t part_primitive = read<uintptr_t>(part + offsets::Primitive);
        uintptr_t character = utils::get_model_instance(localplayer);
        uintptr_t root_part = character ? utils::find_first_child(character, "HumanoidRootPart") : 0;

        if (root_part && part_primitive) {
            uintptr_t root_primitive = read<uintptr_t>(root_part + offsets::Primitive);

            // Read the full CFrame (12 floats) from the target part
            float cframe_data[12] = { 0 };
            for (int i = 0; i < 12; ++i) {
                cframe_data[i] = read<float>(part_primitive + offsets::CFrame + i * sizeof(float));
            }

            // Write the full CFrame to your HumanoidRootPart
            for (int i = 0; i < 12; ++i) {
                write<float>(root_primitive + offsets::CFrame + i * sizeof(float), cframe_data[i]);
            }
        }
    }


    bool world_to_screen(const Vector& world_pos, Vector2D& screen_pos, const matrix& view_matrix, int screen_width, int screen_height) {
        float w = view_matrix.m[3][0] * world_pos.x +
            view_matrix.m[3][1] * world_pos.y +
            view_matrix.m[3][2] * world_pos.z +
            view_matrix.m[3][3];

        if (w <= 0.001f)
            return false;

        float inv_w = 1.0f / w;

        screen_pos.x = (view_matrix.m[0][0] * world_pos.x +
            view_matrix.m[0][1] * world_pos.y +
            view_matrix.m[0][2] * world_pos.z +
            view_matrix.m[0][3]) * inv_w;

        screen_pos.y = (view_matrix.m[1][0] * world_pos.x +
            view_matrix.m[1][1] * world_pos.y +
            view_matrix.m[1][2] * world_pos.z +
            view_matrix.m[1][3]) * inv_w;

        screen_pos.x = (screen_width * 0.5f) * (screen_pos.x + 1.0f);
        screen_pos.y = (screen_height * 0.5f) * (1.0f - screen_pos.y);

        return true;
    }

    Matrix3 look_at(const Vector& cam_pos, const Vector& target_pos) {
        Vector forward = target_pos - cam_pos;
        forward.Normalize();

        Vector right = Vector{ 0, 1, 0 };
        right = right.Cross(forward);
        right.Normalize();

        Vector up = forward.Cross(right);

        Matrix3 matrix{};
        matrix.data[0] = (right.x * -1);  matrix.data[1] = up.x;  matrix.data[2] = -forward.x;
        matrix.data[3] = right.y;         matrix.data[4] = up.y;  matrix.data[5] = -forward.y;
        matrix.data[6] = (right.z * -1);  matrix.data[7] = up.z;  matrix.data[8] = -forward.z;

        return matrix;
    }

    std::vector<uintptr_t> get_all_players() {
        if (!globals::datamodel) return {};
        return utils::get_players(globals::datamodel);
    }

    float get_player_health(uintptr_t player) {
        uintptr_t character = utils::get_model_instance(player);
        if (!character) return 0.0f;
        uintptr_t humanoid = utils::find_first_child_byclass(character, "Humanoid");
        if (!humanoid) return 0.0f;
        return read<float>(humanoid + offsets::Health);
    }

    ImVec3 get_player_position(uintptr_t player) {
        uintptr_t character = utils::get_model_instance(player);
        if (!character) return ImVec3(0, 0, 0);
        uintptr_t root_part = utils::find_first_child(character, "HumanoidRootPart");
        if (!root_part) return ImVec3(0, 0, 0);
        uintptr_t primitive = read<uintptr_t>(root_part + offsets::Primitive);
        Vector pos = read<Vector>(primitive + offsets::Position);
        return ImVec3(pos.x, pos.y, pos.z);
    }

    void teleport_to_player(uintptr_t local_player, uintptr_t target_player) {
        uintptr_t character = utils::get_model_instance(target_player);
        if (!character) return;
        uintptr_t root_part = utils::find_first_child(character, "HumanoidRootPart");
        if (!root_part) return;
        utils::teleport_to_part(local_player, root_part);
    }

    void utils::Spectate(uintptr_t player) {
        uintptr_t workspace = utils::find_first_child_byclass(globals::datamodel, "Workspace");
        if (!workspace) return;
        uintptr_t camera = utils::find_first_child(workspace, "Camera");
        if (!camera) return;

        uintptr_t character = utils::get_model_instance(player);
        if (!character) return;
        uintptr_t hrp = utils::find_first_child(character, "HumanoidRootPart");
        if (!hrp) return;

        write<std::uint64_t>(camera + offsets::CameraSubject, hrp);
    }

    void utils::UnSpectate() {
        uintptr_t workspace = utils::find_first_child_byclass(globals::datamodel, "Workspace");
        if (!workspace) return;
        uintptr_t camera = utils::find_first_child(workspace, "Camera");
        if (!camera) return;

        uintptr_t character = utils::get_model_instance(globals::local_player);
        if (!character) return;
        uintptr_t humanoid = utils::find_first_child_byclass(character, "Humanoid");
        if (!humanoid) return;

        // Set CameraSubject to Humanoid for proper shiftlock/camera control
        write<std::uint64_t>(camera + offsets::CameraSubject, humanoid);
    }

    uintptr_t utils::get_parent(uintptr_t instance) {
        return read<uintptr_t>(instance + offsets::Parent);
    }

    bool utils::is_descendant_of(uintptr_t child, uintptr_t parent) {
        if (!child || !parent) return false;
        uintptr_t current = child;
        while (current) {
            if (current == parent)
                return true;
            current = utils::get_parent(current);
        }
        return false;
    }

    bool utils::get_attribute_bool(uintptr_t instance, const std::string& attr_name) {
        if (!instance || !offsets::AttributeList || !offsets::AttributeToNext || !offsets::AttributeToValue) return false;
        uintptr_t attr_list = read<uintptr_t>(instance + offsets::AttributeList);
        while (attr_list) {
            uintptr_t name_ptr = read<uintptr_t>(attr_list + 0x0); // Usually the first field is the name pointer
            std::string name = utils::read_string(name_ptr);
            if (name == attr_name) {
                uintptr_t value_ptr = read<uintptr_t>(attr_list + offsets::AttributeToValue);
                // Assuming boolean is stored as a byte at value_ptr
                return read<uint8_t>(value_ptr) != 0;
            }
            attr_list = read<uintptr_t>(attr_list + offsets::AttributeToNext);
        }
        return false;
    }

    ImVec3 utils::get_position(uintptr_t instance) {
        if (!instance || !offsets::Primitive || !offsets::Position) return ImVec3(0, 0, 0);
        uintptr_t primitive = read<uintptr_t>(instance + offsets::Primitive);
        if (!primitive) return ImVec3(0, 0, 0);
        Vector pos = read<Vector>(primitive + offsets::Position);
        return ImVec3(pos.x, pos.y, pos.z);
    }

    uintptr_t utils::find_child_by_name(uintptr_t parent, const std::string& name) {
        for (auto child : utils::children(parent)) {
            if (utils::get_instance_name(child) == name)
                return child;
        }
        return 0;
    }

    ImVec3 get_camera_focus() {
        uintptr_t workspace = utils::find_first_child_byclass(globals::datamodel, "Workspace");
        if (!workspace) return ImVec3(0, 0, 0);
        uintptr_t camera = utils::find_first_child(workspace, "Camera");
        if (!camera || !offsets::CameraPos) return ImVec3(0, 0, 0);
        Vector pos = read<Vector>(camera + offsets::CameraPos);
        return ImVec3(pos.x, pos.y, pos.z);
    }

    std::string utils::get_class_name(uintptr_t instance_address)
    {
        return utils::get_instance_classname(instance_address);
    }

    uintptr_t utils::find_first_child(uintptr_t instance_address, int index)
    {
        auto children_vec = utils::children(instance_address);
        if (index < 0 || index >= static_cast<int>(children_vec.size()))
            return 0;
        return children_vec[index];
    }

    std::string utils::get_decal_id(uintptr_t decal_instance)
    {
        if (!decal_instance) return "";
        uintptr_t id_ptr = read<uintptr_t>(decal_instance + offsets::Name);
        if (!id_ptr) return "";
        return utils::length_read_string(id_ptr);
    }

    std::string get_instance_displayname(uintptr_t player) {
            if (!player) return "Unknown";
            uintptr_t displayname_ptr = read<uintptr_t>(player + offsets::DisplayName);
            if (!displayname_ptr) return "Unknown";
            return utils::length_read_string(displayname_ptr);
    }
}