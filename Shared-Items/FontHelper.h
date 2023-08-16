#pragma once
#include "imgui.h"
#include <wtypes.h>

class FontHelper
{
public:
	static ImFont* RegularFont;
	static ImFont* BoldFont;
	static ImFont* HeaderFont;

	static ImVec4 WarningColour;

	static void LoadFonts(HINSTANCE hInstance, ImGuiIO& io, int regularFontId, int boldFontId, float dpiScale);

private:
	static bool initialized;
	static ImFont* LoadFont(HINSTANCE hInstance, ImGuiIO& io, int fontResourceId, float fontSize);
};
