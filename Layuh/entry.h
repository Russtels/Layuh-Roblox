#pragma once

#include < stdio.h >
#include < wtypes.h >
#include < algorithm >
#include "oxorany/oxorany_include.h"
#include < random >
#include < chrono >
#include < thread >
#include < dxgi.h >
#include < string >
#include < filesystem >
#include < fstream >
#include < tlhelp32.h >
#include < list >
#include < unordered_map >
#include < cstdint >
#include < iomanip >
#include < iostream >
#include < cmath >
#include "d3dx9.h"
#include <d3d9.h>
#include "d3dx9tex.h"
#include "kernel/driver.h"

#include "cheat/utils/utils.h"
#include "imgui.h"
#include "imgui_impl_dx9.h"
#include "imgui_impl_win32.h"
#include "cheat/menu/ui/render/drawing/drawing.h"
#include "cheat/menu/ui/render/overlay.h"

#include "cheat/globals/globals.h"
#include "cheat/globals/roblox.h"
#include <unordered_set>

#include "imgui_internal.h"
#define IMGUI_DEFINE_MATH_OPERATORS

#define _sleep_for( ms ) std::this_thread::sleep_for( std::chrono::milliseconds( ms ) );
#define return_null( ) return 0;
#define return_false( ) return false;

#define M_PI   3.14159265358979323846264338327950288

inline IDirect3DDevice9* dx_device = nullptr;

inline ImFont* verdanaFont;
inline ImFont* pixelFont;
inline ImFont* menuSmallFont;
inline ImFont* menuBigFont;

inline std::unordered_set<uintptr_t> flagged_instances;

void render_notifications();
void notify(const std::string & message, ImVec4 color);

namespace vars
{
    namespace aimbot
    {
        inline bool aimbot_enabled = true;
        inline bool prediction = false;
        inline int aimbot_prediction_mode = 1;
        inline float prediction_factor = 0.155;
        inline bool prediction_distance_based = false;
        inline float min_pred_distance = 5.0f;
        inline bool team_check = false;
        inline bool fov_circle = true;
        inline bool auto_hitbox_select = false;
        inline bool lock_on_target = true;
        inline float fov_value = 100.0f;
        inline auto aimbot_key = VK_RBUTTON;
        inline auto aimbot_prediction_key = VK_RBUTTON;
        inline int aimbot_mode = 0;
        inline float aimbot_smoothing = 1.5f;
        inline float max_distance = 800.f;
        inline int target_hitbox = 0;
        inline int aimbot_method = 0;
		inline bool sticky_aim = false;
        inline bool knock_check = false;
        inline float minimum_knock_hp = 14.0f;

        inline bool target_line = false;
        inline ImColor target_line_color = ImColor(160, 10, 40, 255);

        inline bool triggerbot = false;
		inline float triggerbot_delay = 1.0f; // in milliseconds
        inline float triggerbot_max_distance = 50.0f;
        inline auto triggerbot_key = VK_RBUTTON;
		inline int triggerbot_key_mode = 0; // 0 = hold, 1 = toggle

        inline bool arsenal_tp_fix = false;
        inline bool grab_check = false;

        namespace circle_target {
            inline bool enabled = false;
            inline bool teleport_back = false;
            inline float speed = 10.0f;
            inline float radius = 15.0f;
            inline float height_offset = 1.0f;
        }
    }

    namespace esp
    {
        inline bool esp_enabled = true;
        inline bool esp_local = false;
        inline float esp_max_distance = 1000.0f;
        inline bool team_check = false;

        inline bool esp_box = true;
        inline bool esp_fill_box = true;
        inline bool box_glow = false;

        inline bool esp_skeleton = false;
        inline float esp_skeleton_thickness = 1.0f;

        inline bool esp_name = true;

        inline bool esp_health_bar = true;
        inline bool esp_health_text = true;

        inline bool esp_distance = true;

        inline bool esp_tracer = false;
        inline int tracer_start = 0;
        inline int tracer_end = 0;
        inline float tracer_thickness = 2.0f;

        inline bool esp_mode = 0;

        inline bool esp_radar = false;
		inline float radar_pos_x = 100.0f;
		inline float radar_pos_y = 100.0f;
		inline float radar_radius = 500.0f;
		inline bool radar_show_names = false;
		inline bool radar_show_distance = false;
		inline bool radar_show_debug = false;

        inline bool esp_oof_arrows = false;

        inline bool watermark = true; 
		inline int watermarkmodes = 0;

        inline bool keybindlist = false;

        inline ImColor name_color = ImColor(200, 20, 50, 255);    // Brighter red for names
        inline ImColor box_color = ImColor(160, 10, 40, 255);    // Deep red box
        inline ImColor fill_box_color = ImColor(160, 10, 40, 100);    // Semi-transparent fill
        inline ImColor skeleton_color = ImColor(255, 60, 80, 255);    // Lighter for skeleton
        inline ImColor health_bar_color = ImColor(220, 30, 60, 255);    // Bright red health
        inline ImColor radar_color = ImColor(160, 10, 40, 255);    // Match box color
        inline ImColor radar_names_color = ImColor(255, 255, 255, 255);    // Match box color
        inline ImColor radar_distance_color = ImColor(255, 255, 255, 255);    // Match box color
        inline ImColor oof_arrow_color = ImColor(255, 70, 90, 255);    // Brighter arrow
        inline ImColor health_text_color = ImColor(255, 70, 90, 255);    // Brighter arrow
        inline ImColor distance_color = ImColor(255, 70, 90, 255);    // Brighter arrow
        inline ImColor tracer_color = ImColor(160, 10, 40, 255);    // Deep red box

        inline bool esp_preview = false;

        namespace instances
        {
            inline bool esp_box = true;
            inline bool esp_filled_box = true;
            inline bool esp_name = true;
            inline bool esp_health_bar = true;
            inline bool esp_health_number = true;
            inline bool esp_distance = true;
            inline ImColor esp_part_color = ImColor(255, 255, 255, 255);
            inline ImColor esp_model_color = ImColor(0, 255, 0, 255);
            inline ImColor esp_npc_color = ImColor(3, 140, 252, 255);
        }
    }

    namespace visuals
    {
        inline ImColor fov_circle_color = ImColor(160, 10, 40, 255);   
    }

    namespace misc
    {
        inline bool teleport_to_nearest = false;
        inline auto teleport_key = VK_XBUTTON1;
        inline bool teleport_arrow = false;
        inline ImColor teleport_arrow_color = ImColor(209, 112, 250, 255);

        inline bool noclip_enabled = false;
        inline int noclip_mode = 1;
        inline auto noclip_key = VK_F1;

        inline bool fly_enabled = false;
		inline auto fly_toggle_key = VK_TAB;
		inline int fly_mode = 0;
        inline int fly_method = 0;
		inline float fly_speed = 5.0f; 

		inline bool spinbot_enabled = false; 
        inline int spinbot_mode = 0; //0 = spin 1 = jitter

        inline bool speed_hack_enabled = false;
		inline int speed_hack_mode = 0; // 0 = normal 1 = CFrame cheeks
        inline float speed_multiplier = 2.0f;
        inline int speed_hack_keybind_mode = 0; // 0 = hold , 1 = toggle
        inline auto speed_hack_toggle_key = VK_LSHIFT;

        inline bool instant_prompt_enabled = false;

        inline bool custom_fov_enabled = false;
        inline float custom_fov_value = 90.0f;

        inline bool jump_power_enabled = false;
        inline float jump_power_value = 50.0f;

        inline bool teleport_back = false;
        inline bool sex_enabled = false;
        inline float sex_speed = 1.0f;
        inline float sex_distance = 2.0f;
        inline float sex_height = 0.0f;
        inline int sex_mode = 0;

		inline bool fps_unlocker_enabled = false;
		inline int fps_limit = 60;

        inline bool star_count_enabled = false;
        inline int star_count_value = 5000;

        inline bool anti_afk_enabled = true;

        inline bool gravity_modifier_enabled = false;
        inline float gravity_value = 196.2f;

        inline bool sky_customizer_enabled = false;
        inline bool custom_ambient = false;
        inline float ambient_r = 0.5f;
        inline float ambient_g = 0.5f;
        inline float ambient_b = 0.5f;
        inline bool custom_fog = false;
        inline float fog_r = 0.7f;
        inline float fog_g = 0.7f;
        inline float fog_b = 0.7f;
        inline float fog_start = 0.0f;
        inline float fog_end = 100000.0f;
        inline bool custom_time = false;
        inline float clock_time = 14.0f;

        inline bool streamproof = false;

        inline bool mines_gamble = false;
        inline bool slots_gamle = false;
		inline bool blackjack_gamble = false;

        inline bool rapid_fire = false;
        inline bool antistomp = false;
        inline bool nojumpcooldown = false;
        inline bool headless = false;

        inline bool spotify = false;

        inline bool roblox_focused_check = false;
    }

    namespace autoparry {
        inline bool enabled = false;
        inline float parry_distance = 15.0f;
    }

    namespace stream_sniper
    {
        inline bool enabled = false;
 
    }

    namespace whitelist
    {
        inline std::unordered_map<std::string, uintptr_t> users;
        inline std::unordered_map<std::string, uintptr_t> targets;
    }


    namespace instance_explorer
    {
        inline bool enabled = false;
        inline float cache_time = 2.f;
    }

    namespace menu 
    {
		inline auto menukey = VK_INSERT;
    }
}

inline LPDIRECT3DTEXTURE9 workspace_image = nullptr;
inline LPDIRECT3DTEXTURE9 part_image = nullptr;
inline LPDIRECT3DTEXTURE9 model_image = nullptr;
inline LPDIRECT3DTEXTURE9 camera_image = nullptr;
inline LPDIRECT3DTEXTURE9 folder_image = nullptr;
inline LPDIRECT3DTEXTURE9 local_script_image = nullptr;
inline LPDIRECT3DTEXTURE9 script_image = nullptr;
inline LPDIRECT3DTEXTURE9 humanoid_image = nullptr;
inline LPDIRECT3DTEXTURE9 players_image = nullptr;
inline LPDIRECT3DTEXTURE9 sound_image = nullptr;
inline LPDIRECT3DTEXTURE9 accessory_image = nullptr;
inline LPDIRECT3DTEXTURE9 hat_image = nullptr;
inline LPDIRECT3DTEXTURE9 player_image = nullptr;
inline LPDIRECT3DTEXTURE9 module_script_image = nullptr;
inline LPDIRECT3DTEXTURE9 replicated_storage_image = nullptr;
inline LPDIRECT3DTEXTURE9 run_service_image = nullptr;
inline LPDIRECT3DTEXTURE9 spawn_location_image = nullptr;
inline LPDIRECT3DTEXTURE9 replicated_first_image = nullptr;
inline LPDIRECT3DTEXTURE9 starter_gui_image = nullptr;
inline LPDIRECT3DTEXTURE9 starter_pack_image = nullptr;
inline LPDIRECT3DTEXTURE9 starter_player_image = nullptr;
inline LPDIRECT3DTEXTURE9 stats_image = nullptr;
inline LPDIRECT3DTEXTURE9 chat_image = nullptr;
inline LPDIRECT3DTEXTURE9 core_gui_image = nullptr;
inline LPDIRECT3DTEXTURE9 gui_service_image = nullptr;
inline LPDIRECT3DTEXTURE9 aim_image = nullptr;
inline LPDIRECT3DTEXTURE9 visuals_image = nullptr;
inline LPDIRECT3DTEXTURE9 misc_image = nullptr;
inline LPDIRECT3DTEXTURE9 players_menu_image = nullptr;
inline LPDIRECT3DTEXTURE9 config_image = nullptr;
inline LPDIRECT3DTEXTURE9 lua_image = nullptr;

inline LPDIRECT3DTEXTURE9 data_store_image = nullptr; // check
inline LPDIRECT3DTEXTURE9 remote_event_image = nullptr; // check
inline LPDIRECT3DTEXTURE9 remote_function_image = nullptr; // check
inline LPDIRECT3DTEXTURE9 ui_list_layout_image = nullptr; // check
inline LPDIRECT3DTEXTURE9 text_label_image = nullptr; // check
inline LPDIRECT3DTEXTURE9 text_button_image = nullptr; // check
inline LPDIRECT3DTEXTURE9 image_label_image = nullptr; // check
inline LPDIRECT3DTEXTURE9 frame_image = nullptr; // check
inline LPDIRECT3DTEXTURE9 billboard_gui_image = nullptr; // check
inline LPDIRECT3DTEXTURE9 surface_gui_image = nullptr; // check
inline LPDIRECT3DTEXTURE9 http_rbx_api_service_image = nullptr; // check
inline LPDIRECT3DTEXTURE9 insert_service_image = nullptr; // check
