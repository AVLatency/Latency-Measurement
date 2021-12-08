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
	static RecordingResult AnalyzeRecording(const GeneratedSamples& generatedSamples, const WasapiInput& input);

	static void SaveRecording(const WasapiInput& input, std::string path);
	static void SaveIndividualResult(IResultsWriter& writer, AudioFormat* audioFormat, const AudioEndpoint& outputEndpoint, const AudioEndpoint& inputEndpoint, RecordingResult& result, std::string testRootPath);

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
	static RecordingSingleChannelResult AnalyzeSingleChannel(const GeneratedSamples& config, float* recordedSamples, int recordedSamplesLength, int inputSampleRate);
	static int CountLinesInFile(std::string filePath);
	static void GetMinMaxOffset(std::string filePath, float& min, float& max);
};
