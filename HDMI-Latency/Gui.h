#pragma once

#include "GuiState.h"
#include "Resources.h"
#include <vector>
#include <AudioEndpoint.h>

class Gui
{
public:
	Gui(Resources& loadedResources) : resources(loadedResources) {};
	bool DoGui();

private:
	Resources& resources;
	GuiState state = GuiState::GettingStarted;
	int outputDeviceIndex = 0;
	int inputDeviceIndex = 0;
	std::vector<AudioEndpoint> outputAudioEndpoints;
	std::vector<AudioEndpoint> inputAudioEndpoints;

	void HelpMarker(const char* desc);
	void RefreshAudioEndpoints();
};
