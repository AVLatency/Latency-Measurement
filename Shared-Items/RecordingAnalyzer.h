#pragma once
#include <Audioclient.h>
#include "WasapiInput.h"
#include "WasapiOutput.h"
#include "GeneratedSamples.h"
#include "RecordingSingleChannelResult.h"
#include "RecordingResult.h"
#include <string>
#include "IResultsWriter.h"

class RecordingAnalyzer
{
public:
	/// <summary>
	/// Analyzes recording and writes results to files.
	/// </summary>
	static RecordingResult AnalyzeRecording(IResultsWriter& writer, const GeneratedSamples& generatedSamples, const WasapiOutput& output, const WasapiInput& input, const AudioEndpoint& outputEndpoint, const AudioEndpoint& inputEndpoint, AudioFormat* audioFormat, std::string testFileString);

private:
	struct TickPosition
	{
		int index;
		bool tickInverted;
	};

	static const std::string validRecordingsFilename;
	static const std::string invalidRecordingsFilename;

	static WORD GetFormatID(WAVEFORMATEX* waveFormat);
	static std::string GetAudioDataFormatString(WAVEFORMATEX* waveFormat);
	static std::string GetChannelMaskString(WAVEFORMATEX* waveFormat);
	static void SaveRecording(const WasapiInput& input, std::string path);
	static RecordingSingleChannelResult AnalyzeSingleChannel(const GeneratedSamples& config, float* recordedSamples, int recordedSamplesLength, int inputSampleRate);
	static void SaveResult(IResultsWriter& writer, const GeneratedSamples& config, AudioFormat* audioFormat, int inputSampleRate, const AudioEndpoint& outputEndpoint, const AudioEndpoint& inputEndpoint, RecordingResult& result, std::string testRootPath);
	static int CountLinesInFile(std::string filePath);
	static void GetMinMaxOffset(std::string filePath, float& min, float& max);
};
