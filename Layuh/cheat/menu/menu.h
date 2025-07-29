#pragma once

#include "entry.h"

class CMenu {
public:
	void draw_keybinds();
	bool draw_menu();
	void draw_instance_tree(uintptr_t instance);
	void draw_watermark();
	void draw_instance_explorer();
private:
	std::unordered_map<uintptr_t, std::vector<uintptr_t>> cache;
	std::chrono::steady_clock::time_point last_refresh_time;
	const std::chrono::seconds refresh_interval{ 1 };
    std::unordered_map<std::string, LPDIRECT3DTEXTURE9> class_icons = {
   {"Workspace", workspace_image},
   {"Part", part_image},
   {"MeshPart", part_image},
   {"Model", model_image},
   {"Camera", camera_image},
   {"Folder", folder_image},
   {"LocalScript", local_script_image},
   {"Script", script_image},
   {"Humanoid", humanoid_image},
   {"Players", players_image},
   {"Sound", sound_image},
   {"Accessory", accessory_image},
   {"Hat", hat_image},
   {"Player", player_image},
   {"ModuleScript", module_script_image},
   {"ReplicatedStorage", replicated_storage_image},
   {"RunService", run_service_image},
   {"SpawnLocation", spawn_location_image},
   {"ReplicatedFirst", replicated_first_image},
   {"StarterGui", starter_gui_image},
   {"StarterPack", starter_pack_image},
   {"StarterPlayer", starter_player_image},
   {"Stats", stats_image},
   {"Chat", chat_image},
   {"CoreGui", core_gui_image},
   {"GuiService", gui_service_image}
    };
};

namespace d { inline CMenu menu; }

