#pragma once

#include "GuiState.h"
#include "Resources.h"
#include <vector>
#include <AudioEndpoint.h>
#include "imgui.h"
#include "WasapiOutput.h"
#include "GeneratedSamples.h"
#include <thread>

class Gui
{
public:
	static float DpiScale;
	static bool DpiScaleChanged;
	static float PreviousDpiScale;

	Gui(Resources& loadedResources);
	~Gui();
	bool DoGui();
	void Finish(bool requestStop);

private:
	Resources& resources;
	GuiState state = GuiState::SelectAudioDevice;
	int outputDeviceIndex = 0;
	std::vector<AudioEndpoint> outputAudioEndpoints; // TODO: refs and pointers to these are used here and there, which isn't safe.

	WasapiOutput* output = nullptr;
	std::thread* outputThread = nullptr;
	GeneratedSamples* currentSamples = nullptr;
	bool firstChannelOnly = false;
	float blipFrequency = 1;
	int blipSampleLength = 24;
	float onOffFrequency = 1;
	GeneratedSamples::WaveType waveType = GeneratedSamples::WaveType::TestPattern_TonePlusHighFreq;

	void AppDescriptionText();
	void HelpMarker(const char* desc);
	void OptionallyBoldText(const char* text, bool bold);

	void RefreshAudioEndpoints();
	void ClearFormatSelection();
};
