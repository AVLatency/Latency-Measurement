#pragma once
#include "imgui.h"
#include <wtypes.h>

class FontHelper
{
public:
	static ImFont* BoldFont;
	static ImFont* HeaderFont;

	static void LoadFonts(HINSTANCE hInstance, ImGuiIO& io, int regularFontId, int boldFontId, float dpiScale);

private:
	static ImFont* LoadFont(HINSTANCE hInstance, ImGuiIO& io, int fontResourceId, float fontSize);
};
