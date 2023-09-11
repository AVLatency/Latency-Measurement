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
#include "AveragedResult.h"
#include "OutputOffsetProfile.h"
#include "DacLatencyProfile.h"

class RecordingAnalyzer
{
public:
	struct TickPosition
	{
		/// <summary>
		/// This is the index of the start of the rising or falling edge.
		/// </summary>
		int index;
		/// <summary>
		/// This is the index of the end of the rising or falling edge
		/// </summary>
		int endIndex;
		/// <summary>
		/// This is the magnitude of the following rising or falling edge.
		/// </summary>
		float magnitude;
	};

	static RecordingResult AnalyzeRecording(const GeneratedSamples& generatedSamples, const WasapiInput& input, SupportedAudioFormat* format, OutputOffsetProfile* currentProfile, DacLatencyProfile* referenceDacLatency);
	static std::vector<AveragedResult> AnalyzeResults(std::vector<RecordingResult> results, time_t tTime, const AudioEndpoint& outputEndpoint);
	static std::vector<TickPosition> GetTicks(float* recordedSamples, int recordedSamplesLength, int sampleRate, double expectedTickFrequency, int numTicks, float threshold);

	static void SaveRecording(const WasapiInput& input, std::string path, std::string filename);
	static void SaveIndividualResult(IResultsWriter& writer, const AudioEndpoint& outputEndpoint, const AudioEndpoint& inputEndpoint, RecordingResult& result, std::string testRootPath, std::string inputFormat);
	static void SaveFinalResults(IResultsWriter& writer, std::vector<AveragedResult>, std::string testRootPath, std::string csvFilename);

private:
	static const std::string validRecordingsFilename;
	static const std::string invalidRecordingsFilename;

	static RecordingSingleChannelResult AnalyzeSingleChannel(const GeneratedSamples& config, float* recordedSamples, int recordedSamplesLength, int inputSampleRate, float threshold);
};
