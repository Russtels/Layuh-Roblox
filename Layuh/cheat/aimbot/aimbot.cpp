#include "aimbot.h"
#include <algorithm>
#include <Windows.h>
#include <vector>
#include <cmath>
#include <thread>
#include <chrono>
#include <cfloat>
#include <random>

uintptr_t FindMouseService(uintptr_t datamodel) {
    uintptr_t mouse_service = utils::find_first_child_byclass(datamodel, "MouseService");
    return mouse_service;
}

static Matrix3 LookAtToMatrix(const Vector& cameraPosition, const Vector& targetPosition) {
    Vector forward = (targetPosition - cameraPosition).Normalized();
    Vector right = Vector(0, 1, 0).Cross(forward).Normalized();
    Vector up = forward.Cross(right);

    Matrix3 lookAtMatrix{};
    lookAtMatrix.data[0] = -right.x;  lookAtMatrix.data[1] = up.x;  lookAtMatrix.data[2] = -forward.x;
    lookAtMatrix.data[3] = right.y;   lookAtMatrix.data[4] = up.y;  lookAtMatrix.data[5] = -forward.y;
    lookAtMatrix.data[6] = -right.z;  lookAtMatrix.data[7] = up.z;  lookAtMatrix.data[8] = -forward.z;

    return lookAtMatrix;
}

static Matrix3 lerp_matrix3(const Matrix3& a, const Matrix3& b, float t) {
    Matrix3 result{};
    for (int i = 0; i < 9; ++i) {
        result.data[i] = a.data[i] + (b.data[i] - a.data[i]) * t;
    }
    return result;
}

uintptr_t GetCachedMouseService() {
    static uintptr_t cached_mouse_service = 0;
    cached_mouse_service = FindMouseService(globals::datamodel);
    return cached_mouse_service;
}

std::uint64_t GetInputObject(std::uint64_t base_address) {

    std::uint64_t object_address = read<std::uint64_t>(base_address + 0x118);

    return object_address;
}

void WriteMousePosition(std::uint64_t address, float x, float y) {
  std::uint64_t input_object = GetInputObject(address);
  write<Vector2D>(input_object + offsets::MousePosition, { x, y });
}

struct AimInstances {
    uintptr_t aim = 0;
    uintptr_t kh_aim = 0;
    uintptr_t hc_aim = 0;
    uintptr_t rh_aim = 0;
};

void ClipMouse(bool clip)
{
    if (clip)
    {
        RECT window_rect;
        HWND hwnd = FindWindowA(NULL, "Roblox");
        if (hwnd)
        {
            GetWindowRect(hwnd, &window_rect);
            ClipCursor(&window_rect);
        }
    }
    else
    {
        ClipCursor(NULL);
    }
}

AimInstances GetAimInstances(uintptr_t player) {
    AimInstances aims{};

    auto playergui = utils::find_first_child(player, "PlayerGui");
    if (!playergui) return aims;

    auto mainScreenGui = utils::find_first_child(playergui, "MainScreenGui");
    if (mainScreenGui)
        aims.aim = utils::find_first_child(mainScreenGui, "Aim");

    auto intellectUi = utils::find_first_child(playergui, "IntellectUi");
    if (intellectUi)
        aims.kh_aim = utils::find_first_child(intellectUi, "Aim");

    auto mainScreen = utils::find_first_child(playergui, "Main Screen");
    if (mainScreen)
        aims.hc_aim = utils::find_first_child(mainScreen, "Aim");

    auto interfaceGui = utils::find_first_child(playergui, "Interface");
    if (interfaceGui) {
        auto main = utils::find_first_child(interfaceGui, "Main");
        if (main)
            aims.rh_aim = utils::find_first_child(main, "Aim");
    }

    return aims;
}

bool CAimbot::circle_target()
{
    auto local_character = utils::get_model_instance(globals::local_player);
    if (!local_character)
        return false;

    auto local_root = utils::find_first_child(local_character, "HumanoidRootPart");
    if (!local_root)
        return false;

    uintptr_t local_primitive = read<uintptr_t>(local_root + offsets::Primitive);
    if (!local_primitive)
        return false;

    static Vector original_pos;
    static bool saved_original = false;
    static bool teleport_back_done = false;
    static float angle = 0.0f;
    static auto last_time = std::chrono::steady_clock::now();

    auto now = std::chrono::steady_clock::now();
    float dt = std::chrono::duration<float>(now - last_time).count();
    last_time = now;

    if (this->locked_target)
    {
        if (vars::aimbot::circle_target::teleport_back && !saved_original)
        {
            original_pos = read<Vector>(local_primitive + offsets::Position);
            saved_original = true;
            teleport_back_done = false;
        }

        auto target_character = utils::get_model_instance(this->locked_target);
        if (!target_character)
            return false;

        auto target_root = utils::find_first_child(target_character, "HumanoidRootPart");
        if (!target_root)
            return false;

        uintptr_t target_primitive = read<uintptr_t>(target_root + offsets::Primitive);
        if (!target_primitive)
            return false;

        Vector target_pos = read<Vector>(target_primitive + offsets::Position);

        angle += vars::aimbot::circle_target::speed * dt;
        Vector new_local_pos = target_pos;
        new_local_pos.x += vars::aimbot::circle_target::radius * std::cos(angle);
        new_local_pos.z += vars::aimbot::circle_target::radius * std::sin(angle);
        new_local_pos.y += vars::aimbot::circle_target::height_offset;

        write<Vector>(local_primitive + offsets::Position, new_local_pos);
        return true;
    }
    else
    {
        if (vars::aimbot::circle_target::teleport_back && saved_original && !teleport_back_done)
        {
            for (int i = 0; i < 10; i++)
            {
                write<Vector>(local_primitive + offsets::Position, original_pos);
                _sleep_for(1);
                write<Vector>(local_primitive + offsets::Position, original_pos);
            }
            teleport_back_done = true;
            saved_original = false;
            angle = 0.0f;

            original_pos = read<Vector>(local_primitive + offsets::Position);
        }
        return false;
    }
}

bool CAimbot::sex_target()
{
    auto local_character = utils::get_model_instance(globals::local_player);
    if (!local_character)
        return false;

    auto local_root = utils::find_first_child(local_character, "HumanoidRootPart");
    if (!local_root)
        return false;

    uintptr_t local_primitive = read<uintptr_t>(local_root + offsets::Primitive);
    if (!local_primitive)
        return false;

    static Vector original_pos;
    static Matrix3 original_rotation;
    static bool saved_original = false;
    static bool teleport_back_done = false;
    static float oscillation = 0.0f;
    static auto last_time = std::chrono::steady_clock::now();

    auto now = std::chrono::steady_clock::now();
    float dt = std::chrono::duration<float>(now - last_time).count();
    last_time = now;

    if (this->locked_target)
    {
        if (vars::misc::teleport_back && !saved_original)
        {
            original_pos = read<Vector>(local_primitive + offsets::Position);
            original_rotation = read<Matrix3>(local_primitive + offsets::CFrame);
            saved_original = true;
            teleport_back_done = false;
        }

        auto target_character = utils::get_model_instance(this->locked_target);
        if (!target_character)
            return false;

        auto target_root = utils::find_first_child(target_character, "HumanoidRootPart");
        if (!target_root)
            return false;

        uintptr_t target_primitive = read<uintptr_t>(target_root + offsets::Primitive);
        if (!target_primitive)
            return false;

        Vector target_pos = read<Vector>(target_primitive + offsets::Position);
        Matrix3 target_rotation = read<Matrix3>(target_primitive + offsets::CFrame);

        oscillation += vars::misc::sex_speed * dt * 10.0f;
        if (oscillation > 2.0f * 3.14159f)
            oscillation -= 2.0f * 3.14159f;

        Vector new_local_pos = target_pos;
        Matrix3 new_local_rotation = original_rotation;

        std::vector<std::string> targetNames;
        if (utils::find_first_child(target_character, "Torso"))
        {
            targetNames = { "Head", "HumanoidRootPart", "Torso" };
        }
        else if (utils::find_first_child(target_character, "UpperTorso"))
        {
            targetNames = { "Head", "HumanoidRootPart", "UpperTorso", "LowerTorso" };
        }
        else
        {
            targetNames = { "Head", "HumanoidRootPart", "Torso" };
        }

        for (const auto& partName : targetNames)
        {
            uintptr_t child_instance = utils::find_first_child(target_character, partName);
            if (child_instance)
            {
                std::string classname = utils::get_instance_classname(child_instance);
                if (classname == "Part" || classname == "MeshPart")
                {
                    uint64_t part_primitive = read<uintptr_t>(child_instance + offsets::Primitive);
                    write<bool>(part_primitive + offsets::CanCollide, true);
                }
            }
        }

        if (vars::misc::sex_mode == 0)
        {
            Vector direction;
            direction.x = -target_rotation.data[2];
            direction.y = -target_rotation.data[5];
            direction.z = -target_rotation.data[8];

            if (direction.IsZero())
            {
                auto workspace = utils::find_first_child_byclass(globals::datamodel, "Workspace");
                if (workspace)
                {
                    auto camera = read<uintptr_t>(workspace + offsets::Camera);
                    if (camera)
                    {
                        auto camera_rotation = read<Matrix3>(camera + offsets::CameraRotation);
                        float yaw = atan2f(camera_rotation.data[2], camera_rotation.data[8]);
                        direction.x = sinf(yaw);
                        direction.z = cosf(yaw);
                        direction.y = 0.0f;
                    }
                }
            }

            if (direction.IsZero())
            {
                direction.x = 0.0f;
                direction.z = 1.0f;
                direction.y = 0.0f;
            }

            float distance = vars::misc::sex_distance;
            new_local_pos.x = target_pos.x - direction.x * distance;
            new_local_pos.z = target_pos.z - direction.z * distance;
            new_local_pos.y = target_pos.y + vars::misc::sex_height;

            float thrust = sinf(oscillation);
            float thrust_power = distance * 0.5f;
            new_local_pos.x += direction.x * thrust * thrust_power;
            new_local_pos.z += direction.z * thrust * thrust_power;

            Vector facing = Vector(-direction.x, 0, -direction.z);
            facing.Normalize();

            Vector right = Vector(facing.z, 0, -facing.x);
            right.Normalize();

            Vector up(0, 1, 0);

            new_local_rotation.data[0] = right.x;
            new_local_rotation.data[1] = up.x;
            new_local_rotation.data[2] = facing.x;

            new_local_rotation.data[3] = right.y;
            new_local_rotation.data[4] = up.y;
            new_local_rotation.data[5] = facing.y;

            new_local_rotation.data[6] = right.z;
            new_local_rotation.data[7] = up.z;
            new_local_rotation.data[8] = facing.z;
        }
        else if (vars::misc::sex_mode == 1)
        {
            auto target_head = utils::find_first_child(target_character, "Head");
            if (target_head)
            {
                uintptr_t head_primitive = read<uintptr_t>(target_head + offsets::Primitive);
                if (head_primitive)
                {
                    Vector head_pos = read<Vector>(head_primitive + offsets::Position);
                    Vector direction = Vector(target_rotation.data[2], target_rotation.data[5], target_rotation.data[8]);

                    new_local_pos = head_pos;
                    new_local_pos.y += 1.8f - vars::misc::sex_height;

                    float face_distance = -vars::misc::sex_distance;
                    new_local_pos.x += direction.x * face_distance;
                    new_local_pos.z += direction.z * face_distance;

                    float thrust = sinf(oscillation) * 1.5f;
                    new_local_pos.x += direction.x * thrust;
                    new_local_pos.z += direction.z * thrust;

                    Vector facing = Vector(-direction.x, 0, -direction.z);
                    facing.Normalize();
                    Vector right = Vector(facing.z, 0, -facing.x);
                    right.Normalize();
                    Vector up(0, 1, 0);

                    new_local_rotation.data[0] = right.x;
                    new_local_rotation.data[1] = up.x;
                    new_local_rotation.data[2] = facing.x;
                    new_local_rotation.data[3] = right.y;
                    new_local_rotation.data[4] = up.y;
                    new_local_rotation.data[5] = facing.y;
                    new_local_rotation.data[6] = right.z;
                    new_local_rotation.data[7] = up.z;
                    new_local_rotation.data[8] = facing.z;
                }
            }
        }
        else if (vars::misc::sex_mode == 2)
        {
            Vector direction;
            direction.x = -target_rotation.data[2];
            direction.y = -target_rotation.data[5];
            direction.z = -target_rotation.data[8];

            if (direction.IsZero())
            {
                auto workspace = utils::find_first_child_byclass(globals::datamodel, "Workspace");
                if (workspace)
                {
                    auto camera = read<uintptr_t>(workspace + offsets::Camera);
                    if (camera)
                    {
                        auto camera_rotation = read<Matrix3>(camera + offsets::CameraRotation);
                        float yaw = atan2f(camera_rotation.data[2], camera_rotation.data[8]);
                        direction.x = sinf(yaw);
                        direction.z = cosf(yaw);
                        direction.y = 0.0f;
                    }
                }
            }

            if (direction.IsZero())
            {
                direction.x = 0.0f;
                direction.z = 1.0f;
                direction.y = 0.0f;
            }

            float distance = vars::misc::sex_distance;
            new_local_pos.x = target_pos.x + direction.x * distance;
            new_local_pos.z = target_pos.z + direction.z * distance;
            new_local_pos.y = target_pos.y + vars::misc::sex_height;

            float thrust = sinf(oscillation);
            float thrust_power = distance * 0.5f;
            new_local_pos.x -= direction.x * thrust * thrust_power;
            new_local_pos.z -= direction.z * thrust * thrust_power;

            Vector facing = Vector(-direction.x, 0, -direction.z);
            facing.Normalize();

            Vector right = Vector(facing.z, 0, -facing.x);
            right.Normalize();

            Vector up(0, 1, 0);

            new_local_rotation.data[0] = right.x;
            new_local_rotation.data[1] = up.x;
            new_local_rotation.data[2] = facing.x;

            new_local_rotation.data[3] = right.y;
            new_local_rotation.data[4] = up.y;
            new_local_rotation.data[5] = facing.y;

            new_local_rotation.data[6] = right.z;
            new_local_rotation.data[7] = up.z;
            new_local_rotation.data[8] = facing.z;
        }
        else if (vars::misc::sex_mode == 3)
        {
            Vector direction = Vector(target_rotation.data[2], target_rotation.data[5], target_rotation.data[8]);
            direction.Normalize();

            new_local_pos = target_pos;
            new_local_pos.y = target_pos.y - 2.0f;

            float base_distance = -vars::misc::sex_distance * 0.5f;
            new_local_pos.x += direction.x * base_distance;
            new_local_pos.z += direction.z * base_distance;

            float thrust_wave = 0.5f * (sinf(oscillation) - 1.0f);
            float back_power = vars::misc::sex_distance;
            float forward_power = -2.0f;

            if (thrust_wave < 0)
                thrust_wave *= back_power;
            else
                thrust_wave *= forward_power;

            new_local_pos.x += direction.x * thrust_wave;
            new_local_pos.z += direction.z * thrust_wave;

            Vector facing = Vector(-direction.x, 0, -direction.z);
            facing.Normalize();
            Vector right = Vector(facing.z, 0, -facing.x);
            right.Normalize();
            Vector up(0, 1, 0);

            Matrix3 look_rotation;
            look_rotation.data[0] = right.x;
            look_rotation.data[1] = up.x;
            look_rotation.data[2] = facing.x;
            look_rotation.data[3] = right.y;
            look_rotation.data[4] = up.y;
            look_rotation.data[5] = facing.y;
            look_rotation.data[6] = right.z;
            look_rotation.data[7] = up.z;
            look_rotation.data[8] = facing.z;

            new_local_rotation = look_rotation;
        }

        write<Vector>(local_primitive + offsets::Position, new_local_pos);
        write<Matrix3>(local_primitive + offsets::CFrame, new_local_rotation);

        return true;
    }
    else
    {
        if (vars::misc::teleport_back && saved_original && !teleport_back_done)
        {
            for (int i = 0; i < 10; i++)
            {
                write<Vector>(local_primitive + offsets::Position, original_pos);
                write<Matrix3>(local_primitive + offsets::CFrame, original_rotation);
                _sleep_for(1);
            }
            teleport_back_done = true;
            saved_original = false;
            oscillation = 0.0f;
        }

        static uintptr_t last_target = 0;

        if (last_target)
        {
            auto target_character = utils::get_model_instance(last_target);
            if (target_character)
            {
                std::vector<std::string> targetNames;
                if (utils::find_first_child(target_character, "Torso"))
                {
                    targetNames = { "Head", "HumanoidRootPart", "Torso" };
                }
                else if (utils::find_first_child(target_character, "UpperTorso"))
                {
                    targetNames = { "Head", "HumanoidRootPart", "UpperTorso", "LowerTorso" };
                }
                else
                {
                    targetNames = { "Head", "HumanoidRootPart", "Torso" };
                }

                for (const auto& partName : targetNames)
                {
                    uintptr_t child_instance = utils::find_first_child(target_character, partName);
                    if (child_instance)
                    {
                        std::string classname = utils::get_instance_classname(child_instance);
                        if (classname == "Part" || classname == "MeshPart")
                        {
                            uint64_t part_primitive = read<uintptr_t>(child_instance + offsets::Primitive);
                            write<bool>(part_primitive + offsets::CanCollide, false);
                        }
                    }
                }
            }

            last_target = 0;
        }

        if (this->locked_target)
        {
            last_target = this->locked_target;
        }

        return false;
    }
}

bool got_aim_instances = false;
AimInstances GetInstances(uintptr_t player) {
    if (!got_aim_instances)
	got_aim_instances = true;
    AimInstances aims = GetAimInstances(player);
	return aims;
}

bool CAimbot::aim_at_closest_player(matrix view_matrix)
{
    static std::string locked_bone_name;
    static bool last_aimbot_active = false;
    static bool toggle_key_down = false;
    static std::chrono::steady_clock::time_point last_target_switch = std::chrono::steady_clock::now();
    static Vector last_target_pos;
    static auto last_tp_check_time = std::chrono::steady_clock::now();
    static Vector last_good_camera_pos;
    static Matrix3 last_good_camera_rot;
    static bool has_last_good_camera = false;

    if (!vars::aimbot::aimbot_enabled)
        return false;

    bool key_pressed = (GetAsyncKeyState(vars::aimbot::aimbot_key) & 0x8000) != 0;

    if (vars::aimbot::aimbot_mode == 1)
    {
        if (key_pressed && !toggle_key_down)
        {
            globals::key_info::aimbot_active = !globals::key_info::aimbot_active;
            toggle_key_down = true;
            if (!globals::key_info::aimbot_active)
            {
                locked_target = 0;
                locked_bone_name.clear();
            }
        }
        else if (!key_pressed)
        {
            toggle_key_down = false;
        }
    }
    else
    {
        globals::key_info::aimbot_active = key_pressed;
        if (!key_pressed)
        {
            locked_target = 0;
            locked_bone_name.clear();
        }
    }

    if (!globals::key_info::aimbot_active)
    {
        locked_target = 0;
        locked_bone_name.clear();
        last_aimbot_active = false;
        return false;
    }

    auto local_character = utils::get_model_instance(globals::local_player);
    if (!local_character) return false;
    auto local_root = utils::find_first_child(local_character, "HumanoidRootPart");
    if (!local_root) return false;
    uintptr_t local_primitive = read<uintptr_t>(local_root + offsets::Primitive);
    if (!local_primitive) return false;
    Vector local_pos = read<Vector>(local_primitive + offsets::Position);

    static auto last_time = std::chrono::steady_clock::now();
    auto now = std::chrono::steady_clock::now();
    std::chrono::duration<float> elapsed = now - last_time;
    static std::vector<uintptr_t> player_instances;
    if (elapsed.count() >= 0.5f)
    {
        last_time = now;
        player_instances = utils::get_players(globals::datamodel);
    }

    POINT cursor;
    if (!GetCursorPos(&cursor)) return false;

    const char* selected_bone_name = nullptr;

    if (locked_target)
    {
        auto character = utils::get_model_instance(locked_target);
        if (!character)
        {
            locked_target = 0;
            locked_bone_name.clear();
        }
        else
        {
            std::string locked_name = utils::get_instance_name(locked_target);
            if (vars::whitelist::users.count(locked_name) > 0)
            {
                locked_target = 0;
                locked_bone_name.clear();
            }
            else
            {
                bool skip = false;
                if (vars::aimbot::grab_check || vars::aimbot::knock_check)
                {
                    auto workspace = utils::find_first_child_byclass(globals::datamodel, "Workspace");
                    if (workspace)
                    {
                        auto players_folder = utils::find_first_child(workspace, "Players");
                        if (players_folder)
                        {
                            auto player_folder = utils::find_first_child(players_folder, locked_name.c_str());
                            if (player_folder)
                            {
                                auto body_effects = utils::find_first_child(player_folder, "BodyEffects");
                                if (body_effects)
                                {
                                    if (vars::aimbot::grab_check)
                                    {
                                        auto grabbed = utils::find_first_child(body_effects, "Grabbed");
                                        if (grabbed && read<bool>(grabbed + offsets::Value))
                                            skip = true;
                                    }
                                    if (!skip && vars::aimbot::knock_check)
                                    {
                                        auto ko = utils::find_first_child(body_effects, "K.O");
                                        if (ko && read<bool>(ko + offsets::Value))
                                            skip = true;
                                    }
                                }
                            }
                        }
                    }
                }
                if (skip)
                {
                    locked_target = 0;
                    locked_bone_name.clear();
                }
                else if (vars::aimbot::team_check && read<int>(locked_target + offsets::Team) == read<int>(globals::local_player + offsets::Team))
                {
                    locked_target = 0;
                    locked_bone_name.clear();
                }
            }
        }
    }

    if (vars::aimbot::aimbot_method == 1)
    {
        auto workspace = utils::find_first_child_byclass(globals::datamodel, "Workspace");
        if (workspace)
        {
            auto camera = read<uintptr_t>(workspace + offsets::Camera);
            Vector camera_position = read<Vector>(camera + offsets::CameraPos);
            Matrix3 camera_rotation = read<Matrix3>(camera + offsets::CameraRotation);
            if (!vars::aimbot::arsenal_tp_fix || (locked_target && last_target_pos.y >= -100.0f))
            {
                last_good_camera_pos = camera_position;
                last_good_camera_rot = camera_rotation;
                has_last_good_camera = true;
            }
        }
    }

    if (vars::aimbot::arsenal_tp_fix && locked_target)
    {
        auto character = utils::get_model_instance(locked_target);
        if (character)
        {
            auto root = utils::find_first_child(character, "HumanoidRootPart");
            if (root)
            {
                uintptr_t primitive = read<uintptr_t>(root + offsets::Primitive);
                if (primitive)
                {
                    Vector pos = read<Vector>(primitive + offsets::Position);
                    auto now_tp = std::chrono::steady_clock::now();
                    float dist = (pos - last_target_pos).Length();
                    float time_diff = std::chrono::duration<float>(now_tp - last_tp_check_time).count();
                    if ((time_diff > 0.01f && dist > 600.0f) || pos.y < -100.0f)
                    {
                        locked_target = 0;
                        locked_bone_name.clear();
                        if (has_last_good_camera)
                        {
                            auto workspace = utils::find_first_child_byclass(globals::datamodel, "Workspace");
                            if (workspace)
                            {
                                auto camera = read<uintptr_t>(workspace + offsets::Camera);
                                write<Vector>(camera + offsets::CameraPos, last_good_camera_pos);
                                write<Matrix3>(camera + offsets::CameraRotation, last_good_camera_rot);
                            }
                        }
                    }
                    last_target_pos = pos;
                    last_tp_check_time = now_tp;
                }
            }
        }
    }

    bool should_find_new_target = false;

    if (vars::aimbot::aimbot_mode == 1)
    {
        if (globals::key_info::aimbot_active)
        {
            if (!locked_target)
            {
                should_find_new_target = true;
            }
            else if (key_pressed && !toggle_key_down)
            {
                should_find_new_target = true;
                locked_target = 0;
            }
        }
    }
    else
    {
        should_find_new_target = !locked_target;
    }

    if (should_find_new_target)
    {
        float best_distance = FLT_MAX;
        float fov_radius = vars::aimbot::fov_value;
        uintptr_t best_target = 0;
        const char* best_bone_name = nullptr;

        for (auto& player_instance : player_instances)
        {
            if (!player_instance || player_instance == globals::local_player)
                continue;

            if (vars::aimbot::aimbot_mode == 1 && player_instance == locked_target)
                continue;

            std::string name = utils::get_instance_name(player_instance);
            if (vars::whitelist::users.count(name) > 0)
                continue;
            if (globals::local_player)
            {
                int player_team = read<int>(player_instance + offsets::Team);
                int local_team = read<int>(globals::local_player + offsets::Team);
                if (player_team == local_team && vars::aimbot::team_check)
                    continue;
            }
            auto character = utils::get_model_instance(player_instance);
            if (!character)
                continue;
            auto humanoid = utils::find_first_child_byclass(character, "Humanoid");
            if (!humanoid)
                continue;

            bool skip = false;
            if (vars::aimbot::grab_check || vars::aimbot::knock_check)
            {
                auto workspace = utils::find_first_child_byclass(globals::datamodel, "Workspace");
                if (workspace)
                {
                    auto players_folder = utils::find_first_child(workspace, "Players");
                    if (players_folder)
                    {
                        auto player_folder = utils::find_first_child(players_folder, name.c_str());
                        if (player_folder)
                        {
                            auto body_effects = utils::find_first_child(player_folder, "BodyEffects");
                            if (body_effects)
                            {
                                if (vars::aimbot::grab_check)
                                {
                                    auto grabbed = utils::find_first_child(body_effects, "Grabbed");
                                    if (grabbed && read<bool>(grabbed + offsets::Value))
                                        skip = true;
                                }
                                if (!skip && vars::aimbot::knock_check)
                                {
                                    auto ko = utils::find_first_child(body_effects, "K.O");
                                    if (ko && read<bool>(ko + offsets::Value))
                                        skip = true;
                                }
                            }
                        }
                    }
                }
            }
            if (skip)
                continue;

            const char* bone_name = nullptr;
            if (vars::aimbot::target_hitbox == 0)
            {
                bone_name = "Head";
            }
            else if (vars::aimbot::target_hitbox == 1)
            {
                bone_name = "HumanoidRootPart";
            }
            else if (vars::aimbot::target_hitbox == 2)
            {
                static const char* random_parts[] = {
                    "Head", "Torso", "HumanoidRootPart", "UpperTorso", "LowerTorso",
                    "LeftArm", "RightArm", "LeftLeg", "RightLeg",
                    "LeftUpperArm", "LeftLowerArm", "LeftHand",
                    "RightUpperArm", "RightLowerArm", "RightHand",
                    "LeftUpperLeg", "LeftLowerLeg", "LeftFoot",
                    "RightUpperLeg", "RightLowerLeg", "RightFoot"
                };
                static std::random_device rd;
                static std::mt19937 g(rd());
                std::uniform_int_distribution<int> dist(0, std::size(random_parts) - 1);
                bone_name = random_parts[dist(g)];
            }

            auto target_bone = utils::find_first_child(character, bone_name);
            if (!target_bone)
            {
                bone_name = "HumanoidRootPart";
                target_bone = utils::find_first_child(character, bone_name);
                if (!target_bone) continue;
            }

            auto primitive = read<uintptr_t>(target_bone + offsets::Primitive);
            if (!primitive) continue;
            Vector pos = read<Vector>(primitive + offsets::Position);
            if (vars::aimbot::arsenal_tp_fix && pos.y < -100.0f) continue;
            float world_distance = (pos - local_pos).Length();
            if (world_distance > vars::aimbot::max_distance) continue;
            Vector2D sp;
            if (!utils::world_to_screen(pos, sp, view_matrix, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN))) continue;
            float dx = static_cast<float>(cursor.x) - sp.x;
            float dy = static_cast<float>(cursor.y) - sp.y;
            float dist = sqrtf(dx * dx + dy * dy);
            if (dist > fov_radius) continue;
            if (world_distance < best_distance)
            {
                best_distance = world_distance;
                best_target = player_instance;
                best_bone_name = bone_name;
            }
        }

        if (best_target)
        {
            locked_target = best_target;
            locked_bone_name = best_bone_name;
            auto character = utils::get_model_instance(locked_target);
            if (character)
            {
                auto root = utils::find_first_child(character, "HumanoidRootPart");
                if (root)
                {
                    uintptr_t primitive = read<uintptr_t>(root + offsets::Primitive);
                    if (primitive)
                    {
                        last_target_pos = read<Vector>(primitive + offsets::Position);
                        last_tp_check_time = std::chrono::steady_clock::now();
                    }
                }
            }
        }
    }

    last_aimbot_active = globals::key_info::aimbot_active;

    if (!locked_target)
        return false;

    auto character = utils::get_model_instance(locked_target);
    if (!character) return false;

    const char* bone_name = nullptr;

    if (!locked_bone_name.empty())
    {
        bone_name = locked_bone_name.c_str();
    }
    else if (vars::aimbot::target_hitbox == 0)
    {
        bone_name = "Head";
    }
    else if (vars::aimbot::target_hitbox == 1)
    {
        bone_name = "HumanoidRootPart";
    }
    else if (vars::aimbot::target_hitbox == 2)
    {
        static const char* random_parts[] = {
            "Head", "Torso", "HumanoidRootPart", "UpperTorso", "LowerTorso",
            "LeftArm", "RightArm", "LeftLeg", "RightLeg",
            "LeftUpperArm", "LeftLowerArm", "LeftHand",
            "RightUpperArm", "RightLowerArm", "RightHand",
            "LeftUpperLeg", "LeftLowerLeg", "LeftFoot",
            "RightUpperLeg", "RightLowerLeg", "RightFoot"
        };
        static std::random_device rd;
        static std::mt19937 g(rd());
        std::uniform_int_distribution<int> dist(0, std::size(random_parts) - 1);
        bone_name = random_parts[dist(g)];
    }

    auto target_bone = utils::find_first_child(character, bone_name);
    if (!target_bone)
    {
        bone_name = "HumanoidRootPart";
        target_bone = utils::find_first_child(character, bone_name);
        if (!target_bone) return false;
    }

    auto primitive = read<uintptr_t>(target_bone + offsets::Primitive);
    if (!primitive) return false;
    Vector raw_pos = read<Vector>(primitive + offsets::Position);
    Vector pos = raw_pos;

    if (vars::aimbot::prediction)
    {
        auto humanoid_root_part = utils::find_first_child(character, "HumanoidRootPart");
        auto h_r_p_primtive = read<uintptr_t>(humanoid_root_part + offsets::Primitive);
        if (humanoid_root_part && h_r_p_primtive)
        {
            auto velocity = read<Vector>(h_r_p_primtive + offsets::Velocity);
            if (vars::aimbot::prediction_distance_based)
            {
                float distance = (raw_pos - local_pos).Length();
                const float min_distance = vars::aimbot::min_pred_distance;
                float normalized_factor = (std::min)(1.0f, (std::max)(0.0f, (distance - min_distance) / (vars::aimbot::max_distance - min_distance)));
                float scaled_prediction = vars::aimbot::prediction_factor * normalized_factor;
                pos = raw_pos + velocity * scaled_prediction;
            }
            else
            {
                pos = raw_pos + velocity * vars::aimbot::prediction_factor;
            }
        }
    }

    Vector2D screen_pos;
    if (!utils::world_to_screen(pos, screen_pos, view_matrix, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN))) return false;

    if (vars::aimbot::target_line) {
        ImDrawList* draw_list = ImGui::GetOverlayDrawList();
        ImVec2 cursor_pos(static_cast<float>(cursor.x), static_cast<float>(cursor.y));
        ImVec2 target_pos(screen_pos.x, screen_pos.y);
        draw_list->AddLine(cursor_pos, target_pos, vars::aimbot::target_line_color, 1.5f);
    }

    if (vars::aimbot::aimbot_method == 0)
    {
        static auto last_mouse_aim_time = std::chrono::steady_clock::now();
        auto current_time = std::chrono::steady_clock::now();
        auto elapsed_ms = std::chrono::duration_cast<std::chrono::milliseconds>(current_time - last_mouse_aim_time).count();
        if (elapsed_ms >= 5)
        {
            POINT current_cursor;
            if (GetCursorPos(&current_cursor))
            {
                float move_x = screen_pos.x - static_cast<float>(current_cursor.x);
                float move_y = screen_pos.y - static_cast<float>(current_cursor.y);
                float smoothing = vars::aimbot::aimbot_smoothing;
                smoothing = (std::max)(1.0f, smoothing);
                move_x /= smoothing;
                move_y /= smoothing;
                mouse_event(MOUSEEVENTF_MOVE, static_cast<DWORD>(move_x), static_cast<DWORD>(move_y), 0, 0);
            }
            last_mouse_aim_time = current_time;
        }
    }
    if (vars::aimbot::aimbot_method == 1)
    {
        auto workspace = utils::find_first_child_byclass(globals::datamodel, "Workspace");
        auto camera = read<uintptr_t>(workspace + offsets::Camera);
        Vector camera_position = read<Vector>(camera + offsets::CameraPos);
        Matrix3 camera_rotation = read<Matrix3>(camera + offsets::CameraRotation);
        if (!vars::aimbot::arsenal_tp_fix || pos.y >= -100.0f)
        {
            last_good_camera_pos = camera_position;
            last_good_camera_rot = camera_rotation;
            has_last_good_camera = true;
        }
        Matrix3 hit_matrix = LookAtToMatrix(camera_position, pos);
        float lerp_t = 1.0f - (std::clamp(vars::aimbot::aimbot_smoothing, 0.0f, 20.0f) / 20.0f);
        Matrix3 relative_matrix = lerp_matrix3(camera_rotation, hit_matrix, lerp_t);
        write<Matrix3>(camera + offsets::CameraRotation, relative_matrix);
    }
    else if (vars::aimbot::aimbot_method == 2)
    {
        POINT current_cursor;
        if (GetCursorPos(&current_cursor))
        {
            float move_x = screen_pos.x - static_cast<float>(current_cursor.x);
            float move_y = screen_pos.y - static_cast<float>(current_cursor.y);
            INPUT input{};
            input.mi.time = 0;
            input.type = INPUT_MOUSE;
            input.mi.mouseData = 0;
            input.mi.dx = static_cast<LONG>(move_x);
            input.mi.dy = static_cast<LONG>(move_y);
            input.mi.dwFlags = MOUSEEVENTF_MOVE;
            SendInput(1, &input, sizeof(input));
            ClipMouse(true);
        }
    }
    else if (vars::aimbot::aimbot_method == 3)
    {
        AimInstances aims = GetAimInstances(globals::local_player);
        Vector2D dimensions(GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN));
        float clamped_x = screen_pos.x;
        float clamped_y = screen_pos.y;
        uint64_t new_position_x = static_cast<uint64_t>(std::lround(clamped_x));
        uint64_t new_position_y = static_cast<uint64_t>(std::lround(clamped_y));
        if (aims.aim) {
            write<uint64_t>(aims.aim + offsets::FramePositionX, new_position_x);
            write<uint64_t>(aims.aim + offsets::FramePositionY, new_position_y);
        }
        if (aims.hc_aim) {
            write<uint64_t>(aims.hc_aim + offsets::FramePositionX, new_position_x);
            write<uint64_t>(aims.hc_aim + offsets::FramePositionY, new_position_y);
        }
        if (aims.rh_aim) {
            write<uint64_t>(aims.rh_aim + offsets::FramePositionX, new_position_x);
            write<uint64_t>(aims.rh_aim + offsets::FramePositionY, new_position_y);
        }
        if (aims.kh_aim) {
            write<uint64_t>(aims.kh_aim + offsets::FramePositionX, new_position_x);
            write<uint64_t>(aims.kh_aim + offsets::FramePositionY, new_position_y);
        }
        uintptr_t mouse_service = GetCachedMouseService();
        if (mouse_service) {
            WriteMousePosition(mouse_service, clamped_x, clamped_y);
        }
    }
    return true;
}

bool CAimbot::triggerbot(matrix view_matrix)
{
    static bool trigger_active = false;
    static bool toggle_key_down = false;
    static auto last_trigger_time = std::chrono::steady_clock::now();
    static uintptr_t last_target = 0;

    static const char* all_parts[] = {
        "Head", "Torso", "HumanoidRootPart",
        "UpperTorso", "LowerTorso",
        "LeftArm", "RightArm", "LeftLeg", "RightLeg",
        "LeftUpperArm", "LeftLowerArm", "LeftHand",
        "RightUpperArm", "RightLowerArm", "RightHand",
        "LeftUpperLeg", "LeftLowerLeg", "LeftFoot",
        "RightUpperLeg", "RightLowerLeg", "RightFoot"
    };
    constexpr int all_count = sizeof(all_parts) / sizeof(all_parts[0]);

    if (!vars::aimbot::triggerbot)
        return false;

    bool key_pressed = (GetAsyncKeyState(vars::aimbot::triggerbot_key) & 0x8000) != 0;

    if (vars::aimbot::triggerbot_key_mode == 1)
    {
        if (key_pressed && !toggle_key_down)
        {
            trigger_active = !trigger_active;
            toggle_key_down = true;

            if (!trigger_active)
                last_target = 0;
        }
        else if (!key_pressed)
        {
            toggle_key_down = false;
        }
    }
    else
    {
        trigger_active = key_pressed;
        if (!key_pressed)
            last_target = 0;
    }

    if (!trigger_active)
        return false;

    auto local_character = utils::get_model_instance(globals::local_player);
    if (!local_character)
        return false;

    auto local_root = utils::find_first_child(local_character, "HumanoidRootPart");
    if (!local_root)
        return false;

    uintptr_t local_primitive = read<uintptr_t>(local_root + offsets::Primitive);
    if (!local_primitive)
        return false;

    Vector local_pos = read<Vector>(local_primitive + offsets::Position);

    static auto last_player_scan = std::chrono::steady_clock::now();
    static std::vector<uintptr_t> player_instances;
    auto now = std::chrono::steady_clock::now();

    if (std::chrono::duration<float>(now - last_player_scan).count() >= 0.5f)
    {
        last_player_scan = now;
        player_instances = utils::get_players(globals::datamodel);
    }

    POINT cursor;
    if (!GetCursorPos(&cursor))
        return false;

    float best_distance = FLT_MAX;
    uintptr_t best_target = 0;
    const char* best_bone = nullptr;
    Vector best_pos;

    for (auto& player_instance : player_instances)
    {
        if (!player_instance || player_instance == globals::local_player)
            continue;

        std::string name = utils::get_instance_name(player_instance);
        if (vars::whitelist::users.count(name) > 0)
            continue;

        if (globals::local_player && vars::aimbot::team_check)
        {
            int player_team = read<int>(player_instance + offsets::Team);
            int local_team = read<int>(globals::local_player + offsets::Team);
            if (player_team == local_team)
                continue;
        }

        auto character = utils::get_model_instance(player_instance);
        if (!character)
            continue;

        auto humanoid = utils::find_first_child_byclass(character, "Humanoid");
        if (!humanoid)
            continue;

        float health = read<float>(humanoid + offsets::Health);
        if ((vars::aimbot::knock_check && health < vars::aimbot::minimum_knock_hp) || (health && health <= 0))
            continue;

        if (vars::aimbot::aimbot_method == 2)
        {
            int indices[all_count];
            for (int i = 0; i < all_count; ++i) indices[i] = i;
            static std::random_device rd;
            static std::mt19937 g(rd());
            std::shuffle(indices, indices + all_count, g);

            for (int i = 0; i < all_count; ++i) {
                const char* bone_name = all_parts[indices[i]];
                auto target_bone = utils::find_first_child(character, bone_name);
                if (!target_bone)
                    continue;

                auto primitive = read<uintptr_t>(target_bone + offsets::Primitive);
                if (!primitive)
                    continue;

                Vector pos = read<Vector>(primitive + offsets::Position);
                float world_distance = (pos - local_pos).Length();
                if (world_distance > vars::aimbot::triggerbot_max_distance)
                    continue;

                Vector2D screen_pos;
                if (!utils::world_to_screen(pos, screen_pos, view_matrix, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN)))
                    continue;

                float dx = static_cast<float>(cursor.x) - screen_pos.x;
                float dy = static_cast<float>(cursor.y) - screen_pos.y;
                float distance = sqrt(dx * dx + dy * dy);
                float trigger_radius = 10.0f;

                if (distance <= trigger_radius && world_distance < best_distance)
                {
                    best_distance = world_distance;
                    best_target = player_instance;
                    best_bone = bone_name;
                    best_pos = pos;
                }
                break;
            }
        }
        else
        {
            const char* bone_name = (vars::aimbot::target_hitbox == 0) ? "Head" : "HumanoidRootPart";
            auto target_bone = utils::find_first_child(character, bone_name);
            if (!target_bone)
                continue;

            auto primitive = read<uintptr_t>(target_bone + offsets::Primitive);
            if (!primitive)
                continue;

            Vector pos = read<Vector>(primitive + offsets::Position);
            float world_distance = (pos - local_pos).Length();
            if (world_distance > vars::aimbot::triggerbot_max_distance)
                continue;

            Vector2D screen_pos;
            if (!utils::world_to_screen(pos, screen_pos, view_matrix, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN)))
                continue;

            float dx = static_cast<float>(cursor.x) - screen_pos.x;
            float dy = static_cast<float>(cursor.y) - screen_pos.y;
            float distance = sqrt(dx * dx + dy * dy);
            float trigger_radius = 10.0f;

            if (distance <= trigger_radius && world_distance < best_distance)
            {
                best_distance = world_distance;
                best_target = player_instance;
                best_bone = bone_name;
                best_pos = pos;
            }
        }
    }

    if (best_target)
    {
        bool should_fire = false;

        if (vars::aimbot::triggerbot_key_mode == 1)
        {
            if (best_target != last_target ||
                std::chrono::duration<float, std::milli>(now - last_trigger_time).count() >= vars::aimbot::triggerbot_delay + 500.0f)
            {
                should_fire = true;
                last_target = best_target;
            }
        }
        else
        {
            should_fire = true;
        }

        if (should_fire && std::chrono::duration<float, std::milli>(now - last_trigger_time).count() >= vars::aimbot::triggerbot_delay)
        {
            mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0);
            Sleep(5);
            mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
            last_trigger_time = now;
            return true;
        }
    }
    else
    {
        last_target = 0;
    }

    return false;
}
