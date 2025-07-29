#include "image_handler.h"
#include <d3dx9.h>

LPDIRECT3DTEXTURE9 LoadTextureFromMemory(LPDIRECT3DDEVICE9 pDevice, const std::vector<uint8_t>& data) {
    if (data.empty()) return nullptr; 
    LPDIRECT3DTEXTURE9 texture = nullptr;
    HRESULT result = D3DXCreateTextureFromFileInMemory(pDevice, data.data(), data.size(), &texture);
    if (FAILED(result)) {
        return nullptr; 
    }
    return texture;
}

void CImageHandler::create_images() {
    if (globals::images_init) return;

    workspace_image = LoadTextureFromMemory(dx_device, workspace_image_data);
    part_image = LoadTextureFromMemory(dx_device, part_image_data);
    model_image = LoadTextureFromMemory(dx_device, model_image_data);
    folder_image = LoadTextureFromMemory(dx_device, folder_image_data);
    camera_image = LoadTextureFromMemory(dx_device, camera_image_data);
    script_image = LoadTextureFromMemory(dx_device, script_image_data);
    local_script_image = LoadTextureFromMemory(dx_device, local_script_image_data);
    players_image = LoadTextureFromMemory(dx_device, players_image_data);
    humanoid_image = LoadTextureFromMemory(dx_device, humanoid_image_data);
    accessory_image = LoadTextureFromMemory(dx_device, accessory_image_data);
    sound_image = LoadTextureFromMemory(dx_device, sound_image_data);
    replicated_storage_image = LoadTextureFromMemory(dx_device, replicated_storage_image_data);
    hat_image = LoadTextureFromMemory(dx_device, hat_image_data);
    player_image = LoadTextureFromMemory(dx_device, player_image_data);
    module_script_image = LoadTextureFromMemory(dx_device, module_script_image_data);
    run_service_image = LoadTextureFromMemory(dx_device, run_service_image_data);
    spawn_location_image = LoadTextureFromMemory(dx_device, spawn_location_image_data);
    stats_image = LoadTextureFromMemory(dx_device, stats_image_data);
    starter_gui_image = LoadTextureFromMemory(dx_device, starter_gui_image_data);
    replicated_first_image = LoadTextureFromMemory(dx_device, replicated_first_image_data);
    chat_image = LoadTextureFromMemory(dx_device, chat_image_data);
    starter_pack_image = LoadTextureFromMemory(dx_device, starter_pack_image_data);
    gui_service_image = LoadTextureFromMemory(dx_device, gui_service_image_data);
    core_gui_image = LoadTextureFromMemory(dx_device, core_gui_image_data);
	ui_list_layout_image = LoadTextureFromMemory(dx_device, ui_list_layout_data);
	remote_event_image = LoadTextureFromMemory(dx_device, remote_event_data);
	remote_function_image = LoadTextureFromMemory(dx_device, remote_function_data);
	data_store_image = LoadTextureFromMemory(dx_device, data_store_data);
	text_label_image = LoadTextureFromMemory(dx_device, text_label_data);
	text_button_image = LoadTextureFromMemory(dx_device, text_button_data);
	image_label_image = LoadTextureFromMemory(dx_device, image_label_data);
	frame_image = LoadTextureFromMemory(dx_device, frame_data);
	billboard_gui_image = LoadTextureFromMemory(dx_device, billboard_gui_data);
	surface_gui_image = LoadTextureFromMemory(dx_device, surface_gui_data);
	http_rbx_api_service_image = LoadTextureFromMemory(dx_device, http_rbx_api_service_data);
	insert_service_image = LoadTextureFromMemory(dx_device, insert_service_data);

    globals::images_init = true;
}
