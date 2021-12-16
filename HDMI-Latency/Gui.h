#pragma once

#include "GuiState.h"
#include "Resources.h"
#include <vector>
#include <AudioEndpoint.h>
#include "AdjustVolumeManager.h"
#include "imgui.h"
#include "TestManager.h"

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
	GuiState state = GuiState::GettingStarted;
	int outputDeviceIndex = 0;
	int inputDeviceIndex = 0;
	std::vector<AudioEndpoint> outputAudioEndpoints; // TODO: refs and pointers to these are used here and there, which isn't safe.
	std::vector<AudioEndpoint> inputAudioEndpoints; // TODO: refs and pointers to these are used here and there, which isn't safe.

	int resultFormatIndex;

	AdjustVolumeManager* adjustVolumeManager = nullptr;
	TestManager* testManager = nullptr;

	void HelpMarker(const char* desc);
	void OtherCombo(const char* comboName, const char* inputTextName, int* index, const char** options, int optionsLength, char* otherText, int otherTextLength);
	void OptionallyBoldText(const char* text, bool bold);
	void PeakLevel(AdjustVolumeManager::PeakLevelGrade grade, const char* helpText);
	void VerifiedMarker(bool verified);
	void LeoBodnarNote(const AudioFormat* format);
	void FormatDescriptions();

	static int CsvInputFilter(ImGuiInputTextCallbackData* data);

	void RefreshAudioEndpoints();

	void StartAjdustVolumeAudio();
	void StartTest();
};
