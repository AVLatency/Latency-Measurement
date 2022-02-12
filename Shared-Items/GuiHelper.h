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
	static void PeakLevel(AdjustVolumeManager::PeakLevelGrade grade, const char* helpText);
	static void VerifiedHelp();
	static void DearImGuiLegal();

	static int CsvInputFilter(ImGuiInputTextCallbackData* data);
};
