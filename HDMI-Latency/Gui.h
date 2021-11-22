#pragma once

#include "GuiState.h"
#include "Resources.h"

class Gui
{
public:
	Gui(Resources& loadedResources) : resources(loadedResources) {};
	bool DoGui();

private:
	Resources& resources;
	GuiState state = GuiState::GettingStarted;

	void HelpMarker(const char* desc);

};
