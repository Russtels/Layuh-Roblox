#pragma once

#include "entry.h"
#include <cmath>

struct ImVec3 {
    float x, y, z;
    ImVec3() : x(0), y(0), z(0) {}
    ImVec3(float _x, float _y, float _z) : x(_x), y(_y), z(_z) {}

    ImVec3 operator-(const ImVec3& v) const { return ImVec3(x - v.x, y - v.y, z - v.z); }
    float Length() const { return std::sqrt(x * x + y * y + z * z); }
};

class Vector {
public:
    float x, y, z;

    Vector(float x = 0, float y = 0, float z = 0) : x(x), y(y), z(z) { }

    Vector operator+(const Vector& v) const { return Vector(x + v.x, y + v.y, z + v.z); }
    Vector operator-(const Vector& v) const { return Vector(x - v.x, y - v.y, z - v.z); }
    Vector operator*(float scalar) const { return Vector(x * scalar, y * scalar, z * scalar); }
    Vector operator/(float scalar) const { return Vector(x / scalar, y / scalar, z / scalar); }

    Vector& operator+=(const Vector& v) { x += v.x; y += v.y; z += v.z; return *this; }
    Vector& operator-=(const Vector& v) { x -= v.x; y -= v.y; z -= v.z; return *this; }
    Vector& operator*=(float scalar) { x *= scalar; y *= scalar; z *= scalar; return *this; }
    Vector& operator/=(float scalar) { x /= scalar; y /= scalar; z /= scalar; return *this; }

    float Length() const { return std::sqrt(x * x + y * y + z * z); }
    float LengthSqr() const { return x * x + y * y + z * z; }
    float Length2D() const { return std::sqrt(x * x + y * y); }

    Vector Normalized() const {
        float len = Length();
        if (len == 0) return Vector();
        return Vector(x / len, y / len, z / len);
    }

    void Normalize() {
        float len = Length();
        if (len == 0) return;
        x /= len; y /= len; z /= len;
    }

    float Dot(const Vector& v) const {
        return x * v.x + y * v.y + z * v.z;
    }

    Vector Cross(const Vector& v) const {
        return Vector(
            y * v.z - z * v.y,
            z * v.x - x * v.z,
            x * v.y - y * v.x
        );
    }

    float DistTo(const Vector& v) const {
        return (*this - v).Length();
    }

    float DistToSqr(const Vector& v) const {
        return (*this - v).LengthSqr();
    }

    bool IsZero() const {
        return x == 0 && y == 0 && z == 0;
    }
};

static struct Matrix3 final {
    float data[9];
};

class Vector2D {
public:
    float x, y;

    Vector2D(float x = 0, float y = 0) : x(x), y(y) {}

    Vector2D operator+(const Vector2D& v) const { return Vector2D(x + v.x, y + v.y); }
    Vector2D operator-(const Vector2D& v) const { return Vector2D(x - v.x, y - v.y); }
    Vector2D operator*(float scalar) const { return Vector2D(x * scalar, y * scalar); }
    Vector2D operator/(float scalar) const { return Vector2D(x / scalar, y / scalar); }

    Vector2D& operator+=(const Vector2D& v) { x += v.x; y += v.y; return *this; }
    Vector2D& operator-=(const Vector2D& v) { x -= v.x; y -= v.y; return *this; }
    Vector2D& operator*=(float scalar) { x *= scalar; y *= scalar; return *this; }
    Vector2D& operator/=(float scalar) { x /= scalar; y /= scalar; return *this; }

    float Length() const { return std::sqrt(x * x + y * y); }
    float LengthSqr() const { return x * x + y * y; }

    Vector2D Normalized() const {
        float len = Length();
        if (len == 0) return Vector2D();
        return Vector2D(x / len, y / len);
    }

    void Normalize() {
        float len = Length();
        if (len == 0) return;
        x /= len; y /= len;
    }

    float Dot(const Vector2D& v) const {
        return x * v.x + y * v.y;
    }

    float DistTo(const Vector2D& v) const {
        return (*this - v).Length();
    }

    float DistToSqr(const Vector2D& v) const {
        return (*this - v).LengthSqr();
    }

    bool IsZero() const {
        return x == 0 && y == 0;
    }

    float AngleBetween(const Vector2D& v) const {
        float dot = Dot(v);
        float lengths = Length() * v.Length();
        if (lengths == 0) return 0;
        return std::acos(dot / lengths);
    }
};

struct matrix {
	float m[4][4];
};

inline matrix view_matrix;

namespace utils
{
	extern bool initialize_console(...);

	extern void return_message_box(LPCSTR string, ...);

	extern void console_print_success(const char* file, const char* string, ...);

	extern void console_print_color(const char* file, const char* string, ...);

	extern void console_print_rescan(const char* file, const char* string, ...);

    void console_print_logo(const char* file, const char* string, ...);

    extern void teleport_to_part(uintptr_t localplayer, uintptr_t part);

	extern std::vector< uintptr_t > get_players(uintptr_t datamodel_address);

	extern uintptr_t find_first_child_byclass(uintptr_t instance_address, const std::string& child_name);

	extern uintptr_t get_model_instance(uintptr_t instance_address);

	extern uintptr_t __visual_engine_addy();

	extern uintptr_t find_first_child(uintptr_t instance_address, const std::string& child_name);

	extern std::string get_instance_name(uintptr_t instance_address);

    extern std::string get_instance_classname(uintptr_t instance_address);

	extern bool world_to_screen(const Vector& world_pos, Vector2D& screen_pos, const matrix& view_matrix, int screen_width, int screen_height);
    
    extern void change_fov(uint64_t _datamodel, float _fov);
    
    extern void change_viewangles(uint64_t _datamodel, Vector shit);

    extern std::vector< uintptr_t > children(uintptr_t instance_address);

    extern std::vector<uintptr_t> get_descendants(uintptr_t instance_address);

    extern Matrix3 look_at(const Vector& cam_pos, const Vector& target_pos);

    extern std::vector<uintptr_t> get_all_players();

    extern std::string get_instance_name(uintptr_t player);

    extern float get_player_health(uintptr_t player);

    extern ImVec3 get_player_position(uintptr_t player);

    extern void teleport_to_player(uintptr_t local_player, uintptr_t target_player);

    extern void Spectate(uintptr_t player);

    extern void UnSpectate();

    extern uintptr_t find_child_by_name(uintptr_t parent, const std::string& name);

    extern ImVec3 get_position(uintptr_t instance);

    extern bool get_attribute_bool(uintptr_t instance, const std::string& attr_name);

    extern uintptr_t get_parent(uintptr_t instance);
    
    extern bool is_descendant_of(uintptr_t child, uintptr_t parent);

    extern ImVec3 get_camera_focus();

    extern uintptr_t find_first_child(uintptr_t instance_address, int index);

    extern std::string get_class_name(uintptr_t instance_address);

    extern std::string get_decal_id(uintptr_t decal_instance);

    extern std::string get_instance_displayname(uintptr_t instance_address);
};
