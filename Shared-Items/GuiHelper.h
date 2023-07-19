#pragma once
#include "imgui.h"
#include "AdjustVolumeManager.h"
#include "AudioFormat.h"
#include "OutputOffsetProfile.h"

class GuiHelper
{
public:
	enum struct Tool { AudioLatency, HdmiToDigitalAudio };

	static void HelpMarker(const char* desc);
	static void DeveloperOptions();
	static void FormatDescriptions();
	static void ChannelDescriptions();
	static void AMDSpeakersNote();
	static void OtherCombo(const char* comboName, const char* inputTextName, int* index, const char** options, int optionsLength, char* otherText, int otherTextLength);
	static void OptionallyBoldText(const char* text, bool bold);
	static void AdjustVolumeDisplay(const char* imGuiID, const AdjustVolumeManager::VolumeAnalysis& analysis, float DpiScale, float tickMonitorWidth, float fullMonitorWidth, const char* title, bool* useAutoThreshold, float* manualThreshold, bool* cableCrosstalkDetection, bool setDefaultState);
	static void PeakLevel(AdjustVolumeManager::PeakLevelGrade grade, const char* helpText);
	static std::string CableHelpText(Tool tool, OutputOffsetProfile::OutputType outType);
	static void AdjustVolumeInstructionsTroubleshooting(Tool tool, OutputOffsetProfile::OutputType outType, int lastCheckedInputSampleRate, float* outputVolume, bool* overrideNoisyQuiet, void* exampleTexture, int exampleTextureWidth, int exampleTextureHeight, float DpiScale);
	static void TestConfiguration(float DpiScale);
	static void VerifiedHelp();
	static void DearImGuiLegal();

	static int CsvInputFilter(ImGuiInputTextCallbackData* data);

	static void DialogVolumeAdjustDisabledCrosstalk(bool openDialog, ImVec2 center);
	static void DialogNegativeLatency(bool openDialog, ImVec2 center);
};
