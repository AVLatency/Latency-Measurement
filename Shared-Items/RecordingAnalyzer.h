#pragma once
#include <Audioclient.h>
#include "WasapiInput.h"
#include "WasapiOutput.h"
#include "GeneratedSamples.h"
#include "RecordingSingleChannelResult.h"
#include "RecordingResult.h"
#include <string>
#include "IResultsWriter.h"
#include <vector>
#include <map>
#include "AveragedResult.h"

class RecordingAnalyzer
{
public:
	static RecordingResult AnalyzeRecording(const GeneratedSamples& generatedSamples, const WasapiInput& input, const AudioFormat& format);
	static std::map<const AudioFormat*, AveragedResult> AnalyzeResults(std::vector<RecordingResult> results, std::string testGuid, time_t tTime, const AudioEndpoint& outputEndpoint);

	static void SaveRecording(const WasapiInput& input, std::string path);
	static void SaveIndividualResult(IResultsWriter& writer, const AudioEndpoint& outputEndpoint, const AudioEndpoint& inputEndpoint, RecordingResult& result, std::string testRootPath);
	static void SaveFinalResults(IResultsWriter& writer, std::map<const AudioFormat*, AveragedResult>, std::string testRootPath, std::string csvFilename);

private:
	struct TickPosition
	{
		int index;
		bool tickInverted;
	};

	static const std::string validRecordingsFilename;
	static const std::string invalidRecordingsFilename;

	static RecordingSingleChannelResult AnalyzeSingleChannel(const GeneratedSamples& config, float* recordedSamples, int recordedSamplesLength, int inputSampleRate);
};
