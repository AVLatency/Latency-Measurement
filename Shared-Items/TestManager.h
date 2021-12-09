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
#include <map>
#include "OutputOffsetProfile.h"

class TestManager
{
public:
	bool IsFinished = false;
	bool StopRequested = false;
	int TotalPasses = 1;
	int TotalRecordingsPerPass = 1;
	int PassCount = 0;
	int RecordingCount = 0;

	std::vector<AudioFormat*> SelectedFormats;
	std::vector<AudioFormat*> FailedFormats;

	std::vector<RecordingResult> Results;
	std::map<const AudioFormat*, AveragedResult> AveragedResults;

	time_t Time;
	std::string TestFileString;
	std::string AppDirectory;
	
	/// <summary>
	/// Will start the test on a new thread
	/// </summary>
	TestManager(AudioEndpoint& outputEndpoint, const AudioEndpoint& inputEndpoint, std::vector<AudioFormat*> selectedFormats, std::string fileString, std::string appDirectory, IResultsWriter& resultsWriter, OutputOffsetProfile* currentProfile);
	~TestManager();
	/// <summary>
	/// Should be called from the originating thread when IsFinished == true. This will join and delete the test thread.
	/// </summary>
	void CleanUp();

private:
	std::string timeString;

	std::thread* managerThread = NULL;
	AudioEndpoint& outputEndpoint;
	const AudioEndpoint& inputEndpoint;

	IResultsWriter& resultsWriter;

	OutputOffsetProfile* outputOffsetProfile;

	void StartTest();
	bool PerformRecording(AudioFormat* audioFormat);
	bool PlayFormatSwitch(AudioFormat* lastPlayedFormat);
	WAVEFORMATEX* FindFormatSwitchFormat(std::vector<AudioFormat*> formats, AudioFormat* lastPlayedFormat);
};

