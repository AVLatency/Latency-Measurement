#pragma once

#include "GuiState.h"
#include "Resources.h"
#include <vector>
#include <AudioEndpoint.h>
#include "imgui.h"

class Gui
{
public:
	static float DpiScale;
	static bool DpiScaleChanged;
	static float PreviousDpiScale;

	Gui(Resources& loadedResources) : resources(loadedResources) {};
	~Gui();
	bool DoGui();
	void Finish();

private:
	Resources& resources;
	GuiState state = GuiState::GettingStarted;
	int outputDeviceIndex = 0;
	int inputDeviceIndex = 0;
	std::vector<AudioEndpoint> outputAudioEndpoints;
	std::vector<AudioEndpoint> inputAudioEndpoints;

	void HelpMarker(const char* desc);
	void OptionallyBoldText(const char* text, bool bold);
	void VerifiedMarker(bool verified);
	void LeoBodnarNote(const AudioFormat* format);
	void FormatDescriptions();

	void RefreshAudioEndpoints();
};
