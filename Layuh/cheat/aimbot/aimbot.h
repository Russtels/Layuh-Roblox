#pragma once

#include "entry.h"

class CAimbot {
public:
	bool circle_target();
	bool aim_at_closest_player(matrix viewmatrix);
	bool triggerbot(matrix view_matrix);
	bool sex_target();
private:
	uintptr_t locked_target = 0;
};

namespace fs { inline CAimbot aimbot; }
