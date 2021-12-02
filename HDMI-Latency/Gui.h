#pragma once

#include "GuiState.h"
#include "Resources.h"
#include <vector>
#include <AudioEndpoint.h>
#include "AdjustVolumeManager.h"
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

private:
	Resources& resources;
	GuiState state = GuiState::GettingStarted;
	int outputDeviceIndex = 0;
	int inputDeviceIndex = 0;
	std::vector<AudioEndpoint> outputAudioEndpoints;
	std::vector<AudioEndpoint> inputAudioEndpoints;

	int outputOffsetProfileIndex = 0;

	AdjustVolumeManager* adjustVolumeManager = nullptr;

	void HelpMarker(const char* desc);
	void OtherCombo(const char* comboName, const char* inputTextName, int* index, const char** options, int optionsLength, char* otherText, int otherTextLength);
	void OptionallyBoldText(const char* text, bool bold);
	static int CsvInputFilter(ImGuiInputTextCallbackData* data);

	void RefreshAudioEndpoints();

	void StartAjdustVolumeAudio();
};
