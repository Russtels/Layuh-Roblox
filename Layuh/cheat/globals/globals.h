#pragma once
#include <cstdint>

namespace globals
{
	extern bool has_loaded;
    extern bool images_init;

	extern uintptr_t client;
    extern uintptr_t datamodel;
    extern uintptr_t visual_engine;
    extern uintptr_t local_player;
	extern uintptr_t teleport_target;
	extern uintptr_t mouse_service;

	namespace key_info
	{
		extern bool aimbot_active;
		extern bool aimbot_prediction_active;
		extern bool noclip_active;
		extern bool fly_active;
	}
}