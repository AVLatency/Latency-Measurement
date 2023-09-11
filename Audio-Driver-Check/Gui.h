#pragma once

#include "GuiState.h"
#include "Resources.h"
#include <vector>
#include <AudioEndpoint.h>
#include "imgui.h"
#include "AbstractOutput.h"
#include "GeneratedSamples.h"
#include <thread>
#include "OutputAPI.h"

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
	OutputAPI outputType = OutputAPI::WasapiExclusive;
	int outputDeviceIndex = 0;
	std::vector<AudioEndpoint*> outputAudioEndpoints;

	AbstractOutput* output = nullptr;
	std::thread* outputThread = nullptr;
	GeneratedSamples* currentSamples = nullptr;
	bool firstChannelOnly = false;
	float blipFrequency = 1;
	int blipSampleLength = 24;
	float onOffFrequency = 1;
	GeneratedSamples::WaveType waveType = GeneratedSamples::WaveType::TestPattern_TonePlusHighFreq;

	int numChannelsForSavedFile = 2;
	int numLoopsForSavedFile = 1;

	void AppDescriptionText();
	void HelpMarker(const char* desc);
	void OptionallyBoldText(const char* text, bool bold);

	void RefreshAudioEndpoints();
	void ClearFormatSelection();

	std::string WaveTypeSelection(int samplesPerSec);
};
