#pragma once

#include "../../entry.h"

class Drawing {
public:
	void text(std::string text, ImVec2 pos, ImColor color);

	void line(const ImVec2& from, const ImVec2& to, const ImColor& color, float thickness);

	void outlined_line(const ImVec2& from, const ImVec2& to, ImColor line_color, ImColor outline_color, float thickness);

	void box(ImVec2 top_left, ImVec2 bottom_right, ImColor color, float thickness);

	void circle(ImVec2 center, float radius, ImColor color, float thickness);

	void outlined_box(ImVec2 top_left, ImVec2 bottom_right, ImColor box_color, ImColor outline_color, float thickness);

	void filled_box(ImVec2 top_left, ImVec2 bottom_right, ImColor fill_color);

	void hotkey(const char* label, int* key, int* mode, ImVec2 size);

	void hotkey_no_modes(const char* label, int* key, ImVec2 size);

};

namespace drawingapi { inline Drawing drawing; }

