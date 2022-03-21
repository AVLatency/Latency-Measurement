#pragma once
#include "imgui.h"
#include "AdjustVolumeManager.h"
#include "AudioFormat.h"

class GuiHelper
{
public:
	static void HelpMarker(const char* desc);
	static void FormatDescriptions();
	static void ChannelDescriptions();
	static void OtherCombo(const char* comboName, const char* inputTextName, int* index, const char** options, int optionsLength, char* otherText, int otherTextLength);
	static void OptionallyBoldText(const char* text, bool bold);
	static void AdjustVolumeDisplay(const char* imGuiID, const AdjustVolumeManager::VolumeAnalysis& analysis, float DpiScale, float tickMonitorWidth, float fullMonitorWidth, const char* title, bool* useAutoThreshold, float* manualThreshold, bool* cableCrosstalkDetection);
	static void PeakLevel(AdjustVolumeManager::PeakLevelGrade grade, const char* helpText);
	static void AdjustVolumeInstructionsTroubleshooting(int lastCheckedInputSampleRate, float* outputVolume, void* exampleTexture, int exampleTextureWidth, int exampleTextureHeight, float DpiScale);
	static void VerifiedHelp();
	static void DearImGuiLegal();

	static int CsvInputFilter(ImGuiInputTextCallbackData* data);

	static void DialogVolumeAdjustDisabledAutoThreshold(bool openDialog, ImVec2 center);
	static void DialogVolumeAdjustDisabledCrosstalk(bool openDialog, ImVec2 center);
};
