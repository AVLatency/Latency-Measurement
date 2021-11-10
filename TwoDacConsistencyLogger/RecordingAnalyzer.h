#pragma once
#include <Audioclient.h>
#include "WasapiInput.h"
#include "WasapiOutput.h"
#include "RecordingConfiguration.h"
#include "SystemInfo.h"
#include "RecordingSingleChannelResult.h"
#include "RecordingResult.h"
#include <string>

struct TickPosition
{
	int index;
	bool tickInverted;
};
class RecordingAnalyzer
{
public:

	/// <summary>
	/// Analyzes recording and writes results to files.
	/// </summary>
	static RecordingResult AnalyzeRecording(const RecordingConfiguration& config, const SystemInfo& sysInfo, const WasapiOutput& output, const WasapiInput& input, float detectionThresholdMultiplier);
	static void UpdateSummary(const SystemInfo& sysInfo);

private:
	static const std::string validResultsFilename;
	static const std::string invalidResultsFilename;

	static std::string GetTestRootPath(const SystemInfo& sysInfo);
	static std::string GetGuidString();
	static WORD GetFormatID(WAVEFORMATEX* waveFormat);
	static std::string GetAudioDataFormatString(WAVEFORMATEX* waveFormat);
	static std::string GetChannelMaskString(WAVEFORMATEX* waveFormat);
	static void SaveRecording(const WasapiInput& input, std::string guid);
	static RecordingSingleChannelResult AnalyzeSingleChannel(const RecordingConfiguration& config, float* recordedSamples, int recordedSamplesLength, int inputSampleRate, float detectionThresholdMultiplier);
	static void SaveResult(const RecordingConfiguration& config, const SystemInfo& sysInfo, int inputSampleRate, RecordingResult result, std::string testRootPath, std::string recordingRootPath, std::string outputDeviceName, std::string inputDeviceName);
	static int CountLinesInFile(std::string filePath);
	static void GetMinMaxOffset(std::string filePath, float& min, float& max);
};
