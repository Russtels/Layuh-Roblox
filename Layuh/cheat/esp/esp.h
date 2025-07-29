#pragma once

#include "entry.h"

class CESP {
public:
	bool draw_players(matrix viewmatrix);
	//bool draw_instances(matrix viewmatrix);
	bool draw_radar(matrix viewmatrix);
	bool DrawArrow(ImVec2 center, Vector2D targetScreenPos, float radius, ImColor color, bool outlined = false);
};

namespace fs { inline CESP esp; }

