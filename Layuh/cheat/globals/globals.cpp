#include "globals.h"

namespace globals
{
	bool has_loaded = false;
	bool images_init = false;

	uintptr_t client;
	uintptr_t datamodel;
	uintptr_t visual_engine;
	uintptr_t local_player;
	uintptr_t teleport_target;	
	uintptr_t mouse_service;

	namespace key_info 
	{
		bool aimbot_active = false;
		bool aimbot_prediction_active = false;
		bool noclip_active = false;
		bool fly_active = false;
	}
}