#pragma once

#include "TestConfiguration.h"
#include <vector>
#include <audioclient.h>
#include <thread>
#include "AudioFormat.h"
#include "AudioEndpoint.h"
#include <string>
#include "IResultsWriter.h"
#include "AveragedResult.h"
#include "OutputOffsetProfile.h"
#include "DacLatencyProfile.h"

class TestManager
{
public:
	std::atomic<bool> IsFinished = false;
	std::atomic<bool> StopRequested = false;

	int TotalPasses = 1; // written on manager thread, read by the GUI on its thread. If GUI reads out of date value it's fine.
	int TotalRecordingsPerPass = 1; // written on manager thread, read by the GUI on its thread. If GUI reads out of date value it's fine.
	int PassCount = 0; // written on manager thread, read by the GUI on its thread. If GUI reads out of date value it's fine.
	int RecordingCount = 0; // written on manager thread, read by the GUI on its thread. If GUI reads out of date value it's fine.

	std::atomic<bool> ShouldShowFilesystemError = false;
	bool HasShownFilesystemError = false; // only accessed by the GUI thread

	std::atomic<bool> ShouldShowNegativeLatencyError = false;
	bool HasShownNegativeLatencyError = false; // only accessed by the GUI thread

	std::vector<SupportedAudioFormat*> SelectedFormats;
	std::vector<SupportedAudioFormat*> FailedFormats;

	std::vector<RecordingResult> Results;
	std::vector<AveragedResult> AveragedResults;
	std::vector<AveragedResult> SummaryResults;

	time_t Time;
	std::string TestFileString;
	std::string AppDirectory;
	
	/// <summary>
	/// Will start the test on a new thread
	/// </summary>
	TestManager(AudioEndpoint* outputEndpoint, AudioEndpoint* outputEndpoint2, AudioEndpoint* inputEndpoint, std::vector<SupportedAudioFormat*> selectedFormats, std::string fileString, std::string appDirectory, IResultsWriter& resultsWriter, OutputOffsetProfile* currentProfile, DacLatencyProfile* referenceDacLatency);
	~TestManager();
	/// <summary>
	/// Should be called from the originating thread when IsFinished == true. This will join and delete the test thread.
	/// </summary>
	void CleanUp();

private:
	std::string timeString;

	std::thread* managerThread = NULL;
	AudioEndpoint* outputEndpoint;
	AudioEndpoint* outputEndpoint2; // Second output endpoint for Relative Windows Audio output type
	AudioEndpoint* inputEndpoint;

	IResultsWriter& resultsWriter;

	OutputOffsetProfile* outputOffsetProfile;
	DacLatencyProfile* referenceDacLatency;

	void StartTest();
	bool PerformRecording(SupportedAudioFormat* audioFormat);
	bool PlayFormatSwitch(SupportedAudioFormat* lastPlayedFormat);
	WAVEFORMATEX* FindFormatSwitchFormat(std::vector<SupportedAudioFormat*> formats, SupportedAudioFormat* lastPlayedFormat);
	void PopulateSummaryResults();
};

