#pragma once

static int keystatus = 0;
static int keystatus2 = 0;

namespace hotkeys
{
	int aimkey;
	int trigkey;
}

void ChangeKeyAim(void* blank)
{
	keystatus = 1;

	while (true)
	{
		for (int i = 0; i < 0x87; i++)
		{
			if (GetAsyncKeyState(i) & 0x8000)
			{
				var::aim::Key = i;
				keystatus = 0;

				return;
			}
		}
	}
}


static const char* keyNames[] =
{
	(""),
	("Left Mouse"),
	("Right Mouse"),
	("Cancel"),
	("Middle Mouse"),
	("Mouse 5"),
	("Mouse 4"),
	(""),
	("Backspace"),
	("Tab"),
	(""),
	(""),
	("Clear"),
	("Enter"),
	(""),
	(""),
	("Shift"),
	("Control"),
	("Alt"),
	("Pause"),
	("Caps"),
	(""),
	(""),
	(""),
	(""),
	(""),
	(""),
	("Escape"),
	(""),
	(""),
	(""),
	(""),
	("Space"),
	("Page Up"),
	("Page Down"),
	("End"),
	("Home"),
	("Left"),
	("Up"),
	("Right"),
	("Down"),
	(""),
	(""),
	(""),
	("Print"),
	("Insert"),
	("Delete"),
	(""),
	("0"),
	("1"),
	("2"),
	("3"),
	("4"),
	("5"),
	("6"),
	("7"),
	("8"),
	("9"),
	(""),
	(""),
	(""),
	(""),
	(""),
	(""),
	(""),
	("A"),
	("B"),
	("C"),
	("D"),
	("E"),
	("F"),
	("G"),
	("H"),
	("I"),
	("J"),
	("K"),
	("L"),
	("M"),
	("N"),
	("O"),
	("P"),
	("Q"),
	("R"),
	("S"),
	("T"),
	("U"),
	("V"),
	("W"),
	("X"),
	("Y"),
	("Z"),
	(""),
	(""),
	(""),
	(""),
	(""),
	("Numpad 0"),
	("Numpad 1"),
	("Numpad 2"),
	("Numpad 3"),
	("Numpad 4"),
	("Numpad 5"),
	("Numpad 6"),
	("Numpad 7"),
	("Numpad 8"),
	("Numpad 9"),
	("Multiply"),
	("Add"),
	(""),
	("Subtract"),
	("Decimal"),
	("Divide"),
	("F1"),
	("F2"),
	("F3"),
	("F4"),
	("F5"),
	("F6"),
	("F7"),
	("F8"),
	("F9"),
	("F10"),
	("F11"),
	("F12")
};

static bool Items_ArrayGetter(void* data, int idx, const char** out_text)
{
	const char* const* items = (const char* const*)data;
	if (out_text)
	{
		*out_text = items[idx];
	}

	return true;
}

void hotkey_button(int aimkey, void* changekey, int status)
{
	const char* preview_value = NULL;
	if (aimkey >= 0 && aimkey < IM_ARRAYSIZE(keyNames))
	{
		Items_ArrayGetter(keyNames, aimkey, &preview_value);
	}

	std::string aimkeys;
	if (preview_value == NULL)
	{
		aimkeys = "Select any key";
	}
	else
	{
		aimkeys = preview_value;
	}

	if (status == 1)
	{
		aimkeys = "...";
	}

	ImVec2 text_size = ImGui::CalcTextSize(aimkeys.c_str());

	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
	if (ImGui::Button(aimkeys.c_str(), ImVec2(90, 25)))
	{
		if (status == 0)
		{
			CreateThread(0, 0, (LPTHREAD_START_ROUTINE)changekey, nullptr, 0, nullptr);
		}
	}
	ImGui::PopStyleVar();
}