#pragma once

#include "MeasurementToolGuiState.h"
#include "Resources.h"
#include <vector>
#include <AudioEndpoint.h>
#include "AdjustVolumeManager.h"
#include "imgui.h"
#include "TestManager.h"
#include "OutputOffsetProfile.h"

class Gui
{
public:
	static float DpiScale;
	static bool DpiScaleChanged;
	static float PreviousDpiScale;

	bool ShowInitialFilesystemError = false;

	Gui(Resources& loadedResources) : resources(loadedResources) {};
	~Gui();
	bool DoGui();
	void Finish();

private:
	enum struct FileSystemErrorType { Initial, MidTest };
	FileSystemErrorType fileSystemErrorType = FileSystemErrorType::Initial;

	Resources& resources;
	MeasurementToolGuiState state = MeasurementToolGuiState::GettingStarted;
	
	int outputTypeIndex = 0;
	int outputDeviceIndex = 0;
	int outputDevice2Index = 0;
	int inputDeviceIndex = 0;
	std::vector<AudioEndpoint*> outputAudioEndpoints;
	std::vector<AudioEndpoint*> inputAudioEndpoints;
	AudioEndpoint* defaultAudioOutputEndpoint = nullptr;

	int lastCheckedInputSampleRate = 0;

	int resultFormatIndex;

	AdjustVolumeManager* adjustVolumeManager = nullptr;
	TestManager* testManager = nullptr;

	void RefreshAudioEndpoints();
	AudioEndpoint* SelectedAudioOutputEndpoint();

	void StartSelectAudioDevices();
	void StartAdjustVolume();
	void StartTest();

	void SetDutPassthroughOutputType();
	bool IncludeSurroundAsDefault();
};
