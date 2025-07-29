#include "misc.h"
#include <cmath>
#include <iostream>

double SetDoubleValue(uintptr_t self, double value)
{
    write<double>(self + offsets::Value, value);
    return read<double>(self + offsets::Value);
}

std::vector<uintptr_t> GetChildren(uintptr_t self)
{
    std::vector<uintptr_t> container;
    if (!self)
        return container;
    auto start = read<std::uint64_t>(self + offsets::Children);
    if (!start)
        return container;
    auto end = read<std::uint64_t>(start + offsets::ChildrenEnd);
    if (!end)
        return container;
    for (auto instances = read<std::uint64_t>(start); instances != end; instances += 16) {
        if (instances > 1099511627776 && instances < 3298534883328) {
            container.emplace_back(read<uintptr_t>(instances));
        }
    }
    return container;
}

uintptr_t FindFirstChild(uintptr_t self, const std::string& Name)
{
    if (!self || Name.empty())
        return 0;
    std::vector<uintptr_t> children = GetChildren(self);
    for (auto& object : children)
    {
        std::string objName = utils::get_instance_name(object);
        if (objName == Name)
        {
            return object;
        }
    }
    return 0;
}

uintptr_t FindFirstChildOfClass(uintptr_t self, const std::string& Name)
{
    if (!self || Name.empty())
        return 0;
    std::vector<uintptr_t> children = GetChildren(self);
    for (auto& object : children)
    {
        std::string objClass = utils::get_instance_classname(object);
        if (objClass == Name)
        {
            return object;
        }
    }
    return 0;
}

uintptr_t GetLocalPlayer(uintptr_t self)
{
    if (!self)
        return 0;
    return read<uintptr_t>(self + offsets::LocalPlayer);
}

uintptr_t GetCharacter(uintptr_t self)
{
    if (!self)
        return 0;
    return read<uintptr_t>(self + offsets::ModelInstance);
}

class Vector3 {
public:
    float x, y, z;

    Vector3() : x(0), y(0), z(0) {}
    Vector3(float x_, float y_, float z_) : x(x_), y(y_), z(z_) {}

    Vector3 operator+(const Vector3& other) const {
        return Vector3(x + other.x, y + other.y, z + other.z);
    }

    Vector3 operator-(const Vector3& other) const {
        return Vector3(x - other.x, y - other.y, z - other.z);
    }

    Vector3 operator*(float scalar) const {
        return Vector3(x * scalar, y * scalar, z * scalar);
    }

    Vector3 operator/(float scalar) const {
        return Vector3(x / scalar, y / scalar, z / scalar);
    }

    float dot(const Vector3& other) const {
        return x * other.x + y * other.y + z * other.z;
    }

    Vector3 cross(const Vector3& other) const {
        return Vector3(
            y * other.z - z * other.y,
            z * other.x - x * other.z,
            x * other.y - y * other.x
        );
    }

    float length() const {
        return std::sqrt(x * x + y * y + z * z);
    }

    Vector3 normalized() const {
        float len = length();
        return (len == 0) ? Vector3() : (*this) / len;
    }

    friend std::ostream& operator<<(std::ostream& os, const Vector3& v) {
        os << "(" << v.x << ", " << v.y << ", " << v.z << ")";
        return os;
    }
};

void CMisc::teleport_to_nearest(matrix viewmatrix)
{
    if (!vars::misc::teleport_to_nearest)
        return;

    POINT cursor;
    if (!GetCursorPos(&cursor))
        return;

    std::vector<uintptr_t> player_instances = utils::get_players(globals::datamodel);

    uintptr_t closest_target = 0;
    float best_distance = FLT_MAX;

    for (auto& player_instance : player_instances)
    {
        if (!player_instance || player_instance == globals::local_player)
            continue;

        auto character = utils::get_model_instance(player_instance);
        if (!character)
            continue;

        uintptr_t targetPrimitive = 0;
        auto headBone = utils::find_first_child(character, "Head");
        if (headBone)
            targetPrimitive = read<uintptr_t>(headBone + offsets::Primitive);
        if (!targetPrimitive)
        {
            auto rootBone = utils::find_first_child(character, "HumanoidRootPart");
            if (rootBone)
                targetPrimitive = read<uintptr_t>(rootBone + offsets::Primitive);
        }
        if (!targetPrimitive)
            continue;

        Vector partPos = read<Vector>(targetPrimitive + offsets::Position);
        Vector2D screenPos;
        if (!utils::world_to_screen(partPos, screenPos, viewmatrix, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN)))
            continue;

        float dx = static_cast<float>(cursor.x) - screenPos.x;
        float dy = static_cast<float>(cursor.y) - screenPos.y;
        float distance = sqrtf(dx * dx + dy * dy);

        if (distance < best_distance)
        {
            best_distance = distance;
            closest_target = player_instance;
            globals::teleport_target = player_instance;
        }
    }

    if (closest_target)
    {
        auto character = utils::get_model_instance(closest_target);
        if (!character)
            return;

        uintptr_t chosen_part = utils::find_first_child(character, "Head");
        if (!chosen_part) return;

        static bool xButtonPressed = false;

        if (GetAsyncKeyState(vars::misc::teleport_key) & 0x8000)
        {
            if (!xButtonPressed)
            {
                utils::console_print_color(__FILE__, "CMISC::TELEPORT_TO_NEAREST call");
                utils::teleport_to_part(globals::local_player, chosen_part);
                xButtonPressed = true;
            }
        }
        else
        {
            xButtonPressed = false;
        }
    }
}

void CMisc::noclip ( )
{
    if ( !globals::local_player )
        return;
    
    if ( !vars::misc::noclip_enabled )
        return;

    static bool noclip_toggled = false;
    bool should_noclip = false;
    
    if ( vars::misc::noclip_mode == 0 )
    {
        should_noclip = GetAsyncKeyState( vars::misc::noclip_key );
    }

    else if ( vars::misc::noclip_mode == 1 )
    {
        if ( GetAsyncKeyState ( vars::misc::noclip_key ) & 0x1 )
        {
            noclip_toggled = !noclip_toggled;
        }
        should_noclip = noclip_toggled;
    }
    
    globals::key_info::noclip_active = should_noclip;
    
    uintptr_t character = utils::get_model_instance ( globals::local_player );
    if ( !character )
        return;
    
    std::vector< std::string > targetNames;
    if ( utils::find_first_child ( character, "Torso" ) )
    {
        targetNames = { "Head", "HumanoidRootPart", "Torso" };
    }
    else if ( utils::find_first_child ( character, "UpperTorso" ) )
    {
        targetNames = { "Head", "HumanoidRootPart", "UpperTorso", "LowerTorso" };
    }
    else
    {
        targetNames = { "Head", "HumanoidRootPart", "Torso" };
    }
    
    for ( const auto & partName : targetNames )
    {
        uintptr_t child_instance = utils::find_first_child ( character, partName );
        if ( child_instance )
        {
            std::string classname = utils::get_instance_classname ( child_instance );
            if ( classname == "Part" || classname == "MeshPart" )
            {
                uint64_t part_primitive = read< uintptr_t > ( child_instance + offsets::Primitive );
                write< bool > ( part_primitive + offsets::CanCollide, !should_noclip );
            }
        }
    }
}

void CMisc::spinbot()
{
    if (!globals::local_player)
        return;

    if (!vars::misc::spinbot_enabled)
        return;

    uintptr_t character = utils::get_model_instance(globals::local_player);
    if (!character)
        return;

    uintptr_t humanoidRootPart = utils::find_first_child(character, "HumanoidRootPart");
    if (!humanoidRootPart)
        return;

    uintptr_t primitive = read<uintptr_t>(humanoidRootPart + offsets::Primitive);
    if (!primitive)
        return;

    static float spinAngle = 0.0f;

    Matrix3 spinMatrix;

    if (vars::misc::spinbot_mode == 0)
    {
        spinAngle += 0.3f;
        if (spinAngle > 2 * M_PI)
            spinAngle -= 2 * M_PI;

        float cosAngle = cos(spinAngle);
        float sinAngle = sin(spinAngle);

        spinMatrix.data[0] = cosAngle;    spinMatrix.data[1] = 0.0f;  spinMatrix.data[2] = -sinAngle;
        spinMatrix.data[3] = 0.0f;        spinMatrix.data[4] = 1.0f;  spinMatrix.data[5] = 0.0f;
        spinMatrix.data[6] = sinAngle;    spinMatrix.data[7] = 0.0f;  spinMatrix.data[8] = cosAngle;
        write<Matrix3>(primitive + offsets::CFrame, spinMatrix);

    }
    else if (vars::misc::spinbot_mode == 1)
    {
        static int jitterDirection = 1;
        static int jitterCount = 0;

        jitterCount++;
        if (jitterCount > 5)
        {
            jitterDirection = -jitterDirection;
            jitterCount = 0;
        }

        float jitterAngle = jitterDirection * (M_PI / 2);
        float cosAngle = cos(jitterAngle);
        float sinAngle = sin(jitterAngle);

        spinMatrix.data[0] = cosAngle;    spinMatrix.data[1] = 0.0f;  spinMatrix.data[2] = -sinAngle;
        spinMatrix.data[3] = 0.0f;        spinMatrix.data[4] = 1.0f;  spinMatrix.data[5] = 0.0f;
        spinMatrix.data[6] = sinAngle;    spinMatrix.data[7] = 0.0f;  spinMatrix.data[8] = cosAngle;
        write<Matrix3>(primitive + offsets::CFrame, spinMatrix);
     }
    else if (vars::misc::spinbot_mode == 2)
    {
        static std::random_device rd;
        static std::mt19937 gen(rd());
        static std::uniform_int_distribution<> distr(0, 5);

        Vector oldvel = read<Vector>(primitive + offsets::Position);

        Vector newvec(oldvel.x + distr(gen), oldvel.y + distr(gen), oldvel.z + distr(gen));

        write<Vector>(primitive + offsets::Position, newvec);
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        write<Vector>(primitive + offsets::Position, oldvel);
    }
}

void CMisc::fly()
{
    if (!globals::local_player)
        return;

    if (!vars::misc::fly_enabled)
        return;

    static bool fly_toggled = false;
    bool should_fly = false;

    if (vars::misc::fly_mode == 0)
    {
        should_fly = GetAsyncKeyState(vars::misc::fly_toggle_key);
    }
    else if (vars::misc::fly_mode == 1)
    {
        if (GetAsyncKeyState(vars::misc::fly_toggle_key) & 0x1)
        {
            fly_toggled = !fly_toggled;
        }
        should_fly = fly_toggled;
    }

    if (!should_fly)
        return;

    uintptr_t character = utils::get_model_instance(globals::local_player);
    if (!character)
        return;

    uintptr_t humanoidRootPart = utils::find_first_child(character, "HumanoidRootPart");
    if (!humanoidRootPart)
        return;

    uintptr_t primitive = read<uintptr_t>(humanoidRootPart + offsets::Primitive);
    if (!primitive)
        return;

    uintptr_t workspace = read<uintptr_t>(globals::datamodel + offsets::Workspace);
    if (!workspace)
        return;

    uintptr_t Camera = read<uintptr_t>(workspace + offsets::Camera);
    if (!Camera)
        return;

    Vector camPos = read<Vector>(Camera + offsets::CameraPos);
    Matrix3 camCFrame = read<Matrix3>(Camera + offsets::CameraRotation);
    Vector currentPos = read<Vector>(primitive + offsets::Position);

    Vector lookVector = Vector(-camCFrame.data[2], -camCFrame.data[5], -camCFrame.data[8]);
    Vector rightVector = Vector(camCFrame.data[0], camCFrame.data[3], camCFrame.data[6]);

    Vector moveDirection(0, 0, 0);

    if (vars::misc::fly_method == 0)
    {
        Vector flatLook = lookVector;
        if (flatLook.y != 0)
        {
            flatLook.y = 0;
            if (!flatLook.IsZero())
                flatLook.Normalize();
        }

        if (GetAsyncKeyState('W') & 0x8000)
        {
            moveDirection = moveDirection + lookVector;
        }
        if (GetAsyncKeyState('S') & 0x8000)
        {
            moveDirection = moveDirection - lookVector;
        }
        if (GetAsyncKeyState('A') & 0x8000)
        {
            moveDirection = moveDirection - rightVector;
        }
        if (GetAsyncKeyState('D') & 0x8000)
        {
            moveDirection = moveDirection + rightVector;
        }
        if (GetAsyncKeyState(VK_SPACE) & 0x8000)
        {
            moveDirection.y += 1.0f;
        }

        if (!moveDirection.IsZero())
        {
            moveDirection.Normalize();
            moveDirection = moveDirection * vars::misc::fly_speed;
        }

        Vector newPos = currentPos + moveDirection;
        write<Vector>(primitive + offsets::Position, newPos);
        write<bool>(primitive + offsets::CanCollide, false);
        write<Vector>(primitive + offsets::Velocity, Vector(0, 0, 0));
    }
    else if (vars::misc::fly_method == 1)
    {
        if (GetAsyncKeyState('W') & 0x8000)
        {
            moveDirection = moveDirection + lookVector;
        }
        if (GetAsyncKeyState('S') & 0x8000)
        {
            moveDirection = moveDirection - lookVector;
        }
        if (GetAsyncKeyState('A') & 0x8000)
        {
            moveDirection = moveDirection - rightVector;
        }
        if (GetAsyncKeyState('D') & 0x8000)
        {
            moveDirection = moveDirection + rightVector;
        }
        if (GetAsyncKeyState(VK_SPACE) & 0x8000)
        {
            moveDirection.y += 1.0f;
        }

        if (!moveDirection.IsZero())
        {
            moveDirection.Normalize();
            moveDirection = moveDirection * vars::misc::fly_speed;
        }

        write<Vector>(primitive + offsets::Velocity, moveDirection * vars::misc::fly_speed);
        write<bool>(primitive + offsets::CanCollide, false);
    }
}

void CMisc::speed_hack()
{
    static bool speed_toggled = false;
    static bool was_enabled = false;

    uintptr_t character = utils::get_model_instance(globals::local_player);
    if (!character)
        return;

    uintptr_t humanoid = utils::find_first_child_byclass(character, "Humanoid");
    if (!humanoid)
        return;

    uintptr_t humanoidRootPart = utils::find_first_child(character, "HumanoidRootPart");
    if (!humanoidRootPart)
        return;

    uintptr_t primitive = read<uintptr_t>(humanoidRootPart + offsets::Primitive);
    if (!primitive)
        return;

    float baseSpeed = 16.0f;

    bool should_speed = false;
    if (vars::misc::speed_hack_keybind_mode == 0)
    {
        should_speed = (GetAsyncKeyState(vars::misc::speed_hack_toggle_key) & 0x8000);
    }
    else if (vars::misc::speed_hack_keybind_mode == 1)
    {
        if (GetAsyncKeyState(vars::misc::speed_hack_toggle_key) & 0x1)
        {
            speed_toggled = !speed_toggled;
        }
        should_speed = speed_toggled;
    }

    if (vars::misc::speed_hack_enabled && should_speed)
    {
        if (vars::misc::speed_hack_mode == 0)
        {
            float speedMultiplier = vars::misc::speed_multiplier;
            write<float>(humanoid + offsets::WalkSpeed, speedMultiplier);
            write<float>(humanoid + offsets::WalkSpeedCheck, speedMultiplier);
            was_enabled = true;
        }
        else if (vars::misc::speed_hack_mode == 1)
        {
            uintptr_t workspace = read<uintptr_t>(globals::datamodel + offsets::Workspace);
            if (!workspace)
                return;

            uintptr_t Camera = read<uintptr_t>(workspace + offsets::Camera);
            if (!Camera)
                return;

            Matrix3 camCFrame = read<Matrix3>(Camera + offsets::CameraRotation);

            Vector lookVector(-camCFrame.data[2], -camCFrame.data[5], -camCFrame.data[8]);
            Vector rightVector(camCFrame.data[0], camCFrame.data[3], camCFrame.data[6]);
            Vector moveDirection(0, 0, 0);

            if (GetAsyncKeyState('W') & 0x8000)
                moveDirection += lookVector;
            if (GetAsyncKeyState('S') & 0x8000)
                moveDirection -= lookVector;
            if (GetAsyncKeyState('A') & 0x8000)
                moveDirection -= rightVector;
            if (GetAsyncKeyState('D') & 0x8000)
                moveDirection += rightVector;

            moveDirection.y = 0.0f;

            if (!moveDirection.IsZero())
            {
                moveDirection.Normalize();
                float speed = vars::misc::speed_multiplier / 10.0f;
                Vector currentPos = read<Vector>(primitive + offsets::Position);
                Vector targetPos = currentPos + moveDirection * speed;
                float lerpAlpha = 0.2f;
                Vector lerpedPos = currentPos + (targetPos - currentPos) * lerpAlpha;
                write<Vector>(primitive + offsets::Position, lerpedPos);
            }
        }
    }
    else if (was_enabled)
    {
        write<float>(humanoid + offsets::WalkSpeed, baseSpeed);
        write<float>(humanoid + offsets::WalkSpeedCheck, baseSpeed);
        was_enabled = false;
    }
}


//void CMisc::instant_proximity_prompt()
//{
//    if (!globals::local_player || !vars::misc::instant_prompt_enabled)
//        return;
//
//    uintptr_t workspace = read<uintptr_t>(globals::datamodel + offsets::Workspace);
//    if (!workspace)
//        return;
//
//    auto process_instance = [](uintptr_t instance, auto& self) -> void {
//        std::string className = utils::get_instance_classname(instance);
//        if (className == "ProximityPrompt")
//        {
//            write<float>(instance + offsets::ProximityPromptHoldDuraction, 0.0f);
//        }
//
//        std::vector<uintptr_t> children = utils::children(instance);
//        for (auto& child : children)
//        {
//            self(child, self);
//        }
//        };
//
//    process_instance(workspace, process_instance);
//}

void CMisc::jump_power()
{
    static bool was_enabled = false;

    uintptr_t character = utils::get_model_instance(globals::local_player);
    if (!character)
        return;

    uintptr_t humanoid = utils::find_first_child_byclass(character, "Humanoid");
    if (!humanoid)
        return;

    float baseJumpPower = 20.0f;

    if (vars::misc::jump_power_enabled)
    {
        float jumpPower = vars::misc::jump_power_value;
        write<float>(humanoid + offsets::JumpPower, jumpPower);
        was_enabled = true;
    }
    else if (was_enabled)
    {
        write<float>(humanoid + offsets::JumpPower, baseJumpPower);
        was_enabled = false;
    }
}

//void CMisc::anti_afk()
//{
// if (vars::misc::anti_afk_enabled) {
// write<float>(offsets::ForceNewAFKDuration, 999.0f);
// }
//}

//void CMisc::press_left_click()
//{
//    mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0);
//    Sleep(5);
//    mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
//	notify("Parried", ImVec4(0.0f, 1.0f, 0.0f, 1.0f));
//}
//
//void CMisc::autoparry()
//{
//    if (!globals::local_player || !vars::autoparry::enabled)
//        return;
//
//    uintptr_t character = utils::get_model_instance(globals::local_player);
//    if (!character)
//        return;
//
//    uintptr_t highlight = utils::find_first_child(character, "Highlight");
//    if (!highlight)
//        return;
//
//    uintptr_t workspace = utils::find_first_child_byclass(globals::datamodel, "Workspace");
//    if (!workspace)
//        return;
//
//    uintptr_t root_part = utils::find_first_child(character, "HumanoidRootPart");
//    if (!root_part)
//        return;
//
//    uintptr_t player_primitive = read<uintptr_t>(root_part + offsets::Primitive);
//    if (!player_primitive)
//        return;
//
//    Vector player_position = read<Vector>(player_primitive + offsets::Position);
//
//    auto workspace_children = utils::children(workspace);
//
//    for (auto& folder : workspace_children)
//    {
//        if (!folder)
//            continue;
//
//        std::string class_name = utils::get_instance_classname(folder);
//        if (class_name != "Folder")
//            continue;
//
//        auto balls = utils::children(folder);
//        for (auto& ball : balls)
//        {
//            if (!ball)
//                continue;
//
//            uintptr_t primitive = read<uintptr_t>(ball + offsets::Primitive);
//            if (!primitive)
//                continue;
//
//            Vector position = read<Vector>(primitive + offsets::Position);
//            float distance = (position - player_position).Length();
//
//            if (distance <= vars::autoparry::parry_distance)
//            {
//                press_left_click();
//                return;
//            }
//        }
//    }
//}

void CMisc::rapid_fire()
{
    static bool mouseDown = false;
    if (!vars::misc::rapid_fire) return;

    uintptr_t players = utils::find_first_child_byclass(globals::datamodel, "Players");
    std::string local_name = utils::get_instance_name(globals::local_player);
    uintptr_t character = utils::get_model_instance(globals::local_player);
    if (!character)
        return;

    uintptr_t bodyeffects = utils::find_first_child(character, "BodyEffects");
    if (!bodyeffects)
        return;
	//printf("BodyEffects: %p\n", bodyeffects);
    if (!bodyeffects) return;
    uintptr_t gunfiring = FindFirstChild(bodyeffects, "GunFiring");
	//printf("GunFiring: %p\n", gunfiring);
    if (!gunfiring) return;

    bool isFiring = read<bool>(gunfiring + offsets::Value);

    if (isFiring && !mouseDown)
    {
        INPUT input{};
        input.type = INPUT_MOUSE;
        input.mi.dwFlags = MOUSEEVENTF_LEFTDOWN;
        SendInput(1, &input, sizeof(input));
        mouseDown = true;
    }
    else if (!isFiring && mouseDown)
    {
        INPUT input{};
        input.type = INPUT_MOUSE;
        input.mi.dwFlags = MOUSEEVENTF_LEFTUP;
        SendInput(1, &input, sizeof(input));
        mouseDown = false;
    }
}

void CMisc::antistomp_realud()
{
    bool antistomptoggle = vars::misc::antistomp;
    if (!antistomptoggle)
        return;

    uintptr_t character = utils::get_model_instance(globals::local_player);
    if (!character)
        return;

    uintptr_t bodyeffects = utils::find_first_child(character, "BodyEffects");
    if (!bodyeffects)
        return;

    uintptr_t ko = utils::find_first_child(bodyeffects, "K.O");
    if (!ko)
        return;

    bool is_knocked = read<bool>(ko + offsets::Value);
    if (is_knocked)
    {
        uintptr_t humanoid = utils::find_first_child_byclass(character, "Humanoid");
        uintptr_t parent = read<uintptr_t>(character + offsets::Parent);
        write<uintptr_t>(character + offsets::Parent, 0);
        write<float>(humanoid + offsets::Health, 0);
    }
}

void CMisc::headless()
{
    if (!globals::local_player)
        return;

    if (!vars::misc::headless)
        return;

    uintptr_t character = utils::get_model_instance(globals::local_player);
    if (!character)
        return;

    uintptr_t head = utils::find_first_child(character, "Head");
    if (!head)
        return;

    write<float>(head + offsets::Transparency, 1.0f);

    std::vector<uintptr_t> head_children = utils::children(head);
    for (auto& child : head_children) {
        std::string name = utils::get_instance_name(child);
        std::string class_name = utils::get_instance_classname(child);
        if (name == "face" && (class_name == "Decal" || class_name == "Texture")) {
            write<float>(child + offsets::Transparency, 1.0f);
        }
    }
}

void CMisc::NoJumpCoolDown()
{
    if (!globals::local_player)
        return;

    uintptr_t character = utils::get_model_instance(globals::local_player);
    if (!character)
        return;

    uintptr_t humanoid = utils::find_first_child_byclass(character, "Humanoid");
    if (!humanoid)
        return;

    if (vars::misc::nojumpcooldown) {
        write<bool>(humanoid + offsets::JumpPower, false);
    }
    else {
        write<bool>(humanoid + offsets::JumpPower, true);
    }
}
