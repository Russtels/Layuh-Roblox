#pragma once

#include "entry.h"

class CMisc {
public:
	void teleport_to_nearest(matrix viewmatrix);
	void fix_position();
	void noclip();
	void fly();
	void spinbot();
	void infinite_jump();
	void speed_hack();
	void antistomp_realud();
	//void instant_proximity_prompt();
	void custom_fov();
	void jump_power();
	void star_count();
	//void anti_afk();
	void gravity_modifier();
	void sky_customizer();
	void print_game_info();
	//void autoparry();
	void press_left_click();
	void rapid_fire();
	//void no_jump_cooldown();
	void headless();
	void NoJumpCoolDown();
	//void korblox();
};

namespace fs { inline CMisc misc; }
