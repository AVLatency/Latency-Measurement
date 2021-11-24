#pragma once

#include "GuiState.h"
#include "Resources.h"
#include <vector>
#include <AudioEndpoint.h>
#include "AdjustVolumeManager.h"

class Gui
{
public:
	Gui(Resources& loadedResources) : resources(loadedResources) {};
	~Gui();
	bool DoGui();

private:
	Resources& resources;
	GuiState state = GuiState::GettingStarted;
	int outputDeviceIndex = 0;
	int inputDeviceIndex = 0;
	std::vector<AudioEndpoint> outputAudioEndpoints;
	std::vector<AudioEndpoint> inputAudioEndpoints;

	AdjustVolumeManager* adjustVolumeManager = nullptr;

	void HelpMarker(const char* desc);
	void RefreshAudioEndpoints();

	void StartAjdustVolumeAudio();
};
