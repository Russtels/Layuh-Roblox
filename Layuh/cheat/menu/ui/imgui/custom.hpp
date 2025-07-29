#pragma once
#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui.h"
#include "imgui_internal.h"
#include <string>
#include <chrono>

class custom
{
public:
	bool tab(const char* text, bool selected, ImFont* icon_font);
	void begin_child(const char* name, ImVec2 size);
	void end_child();
	bool checkbox(const char* label, bool* v);
	void selector(const char* label, int& value, int max_value, const char* labels[], int label_count);
};

inline custom* imgui_custom;