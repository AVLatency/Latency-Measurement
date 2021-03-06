#include "TestManager.h"
#include <thread>
#include <Mmdeviceapi.h>
#include "WasapiOutput.h"
#include "WasapiInput.h"
#include "GeneratedSamples.h"
#include "RecordingAnalyzer.h"
#include "StringHelper.h"

TestManager::TestManager(AudioEndpoint& outputEndpoint, const AudioEndpoint& inputEndpoint, std::vector<AudioFormat*> selectedFormats, std::string fileString, std::string appDirectory, IResultsWriter& resultsWriter, OutputOffsetProfile* currentProfile, DacLatencyProfile* referenceDacLatency)
	: outputEndpoint(outputEndpoint), inputEndpoint(inputEndpoint), SelectedFormats(selectedFormats), AppDirectory(appDirectory), resultsWriter(resultsWriter), Time(time(0)), outputOffsetProfile(currentProfile), referenceDacLatency(referenceDacLatency)
{
	TestFileString = std::format("{} {}", StringHelper::GetTimeString(Time, true), fileString);

	// Removes all spaces from the beginning of the string
	while (TestFileString.size() > 0 && isspace(TestFileString.front()))
	{
		TestFileString.erase(TestFileString.begin());
	}
	// Remove all spaces from the end of the string.
	while (TestFileString.size() > 0 && isspace(TestFileString.back()))
	{
		TestFileString.pop_back();
	}

	managerThread = new std::thread([this] { this->StartTest(); });
}

TestManager::~TestManager()
{
	if (managerThread != NULL)
	{
		CleanUp();
	}
}

void TestManager::CleanUp()
{
	managerThread->join();
	delete(managerThread);
	managerThread = NULL;
}

void TestManager::StartTest()
{
	SetThreadExecutionState(ES_DISPLAY_REQUIRED); // Prevent display from turning off while running this tool.

	TotalPasses = TestConfiguration::NumMeasurements > 0 ? TestConfiguration::NumMeasurements : 1;
	TotalRecordingsPerPass = SelectedFormats.size() > 0 ? SelectedFormats.size() : 1;


	for (int i = 0; i < TestConfiguration::NumMeasurements; i++)
	{
		PassCount = i;
		RecordingCount = 0;
		AudioFormat* lastPlayedFormat = nullptr;

		// for determining if a format switch tone is necessary:
		int firstSampleRate = -1;
		bool switchedSampleRates = false;

		for (AudioFormat* audioFormat : SelectedFormats)
		{
			if (!StopRequested && std::find(FailedFormats.begin(), FailedFormats.end(), audioFormat) == FailedFormats.end())
			{
				bool valid = PerformRecording(audioFormat);
				if (!valid)
				{
					FailedFormats.push_back(audioFormat);
				}
				RecordingCount++;
				lastPlayedFormat = audioFormat;
				if (firstSampleRate == -1)
				{
					firstSampleRate = audioFormat->WaveFormat->nSamplesPerSec;
				}
				else
				{
					if (firstSampleRate != audioFormat->WaveFormat->nSamplesPerSec)
					{
						switchedSampleRates = true;
					}
				}
			}
		}
		TotalRecordingsPerPass = RecordingCount;

		// Inject a dummy format when we're down to a single recording to force the HDMI Audio Device to re-sync to a new signal format
		if (TestConfiguration::InsertFormatSwitch && !switchedSampleRates && !StopRequested)
		{
			bool formatSwitchResult = PlayFormatSwitch(lastPlayedFormat);
			if (!formatSwitchResult)
			{
				// something has gone very wrong, so bail early since there's no point in doing the same test over and over without switching formats.
				StopRequested = true;
			}
		}
	}

	AveragedResults = RecordingAnalyzer::AnalyzeResults(Results, Time, outputEndpoint);
	PopulateSummaryResults();

	if (outputOffsetProfile->isNoOffset)
	{
		// in this case, it's fine if there is a negative latency, because the output offset profile for the daul-out device is unknown.
	}
	else
	{
		for (auto result : AveragedResults)
		{
			if (result.AverageLatency() <= -0.5)
			{
				ShouldShowNegativeLatencyError = true;
			}
		}
	}

	try
	{
		RecordingAnalyzer::SaveFinalResults(resultsWriter, AveragedResults, StringHelper::GetRootPath(AppDirectory), std::format("{}.csv", TestFileString));
	}
	catch (...)
	{
		ShouldShowFilesystemError = true;
	}

	SetThreadExecutionState(0); // Reset prevent display from turning off while running this tool.

	IsFinished = true;
}

bool TestManager::PerformRecording(AudioFormat* audioFormat)
{
	GeneratedSamples* generatedSamples = new GeneratedSamples(audioFormat->WaveFormat, GeneratedSamples::WaveType::LatencyMeasurement);

	bool validResult = false;
	for (int i = 0; i < TestConfiguration::AttemptsBeforeFail; i++)
	{
		WasapiOutput* output = new WasapiOutput(outputEndpoint, false, true, generatedSamples->samples, generatedSamples->samplesLength, audioFormat->WaveFormat);
		std::thread outputThread{ [output] { output->StartPlayback(); } };

		WasapiInput* input = new WasapiInput(inputEndpoint, false, generatedSamples->TestWaveDurationInSeconds());
		std::thread inputThread{ [input] { input->StartRecording(); } };

		outputThread.join();
		inputThread.join();

		RecordingResult result = RecordingAnalyzer::AnalyzeRecording(*generatedSamples, *input, audioFormat, outputOffsetProfile, referenceDacLatency);
		Results.push_back(result);
		
		if (TestConfiguration::SaveIndividualWavFiles)
		{
			std::string recordingFolder = format("{}/{}/{}", StringHelper::GetRootPath(AppDirectory), TestFileString, audioFormat->FormatString);
			try
			{
				RecordingAnalyzer::SaveRecording(*input, recordingFolder, std::format("{}.wav", result.GUID));
			}
			catch (...)
			{
				ShouldShowFilesystemError = true;
			}
		}
		if (TestConfiguration::SaveIndividualRecordingResults)
		{
			try
			{
				std::string inputFormat = AudioFormat((WAVEFORMATEX*)&input->waveFormat).FormatString;
				RecordingAnalyzer::SaveIndividualResult(resultsWriter, outputEndpoint, inputEndpoint, result, std::format("{}/{}", StringHelper::GetRootPath(AppDirectory), TestFileString), inputFormat);
			}
			catch (...)
			{
				ShouldShowFilesystemError = true;
			}
		}

		validResult = result.Channel1.ValidResult && result.Channel2.ValidResult;

		delete output;
		delete input;
		
		if (validResult)
		{
			break;
		}
	}

	delete generatedSamples;
	return validResult;
}

bool TestManager::PlayFormatSwitch(AudioFormat* lastPlayedFormat)
{
	WAVEFORMATEX* waveFormat = nullptr;

	// First, see if the stereo 48 kHz 16 bit is an option:
	{
		std::vector<AudioFormat*> formats = outputEndpoint.GetFormats(2, 48000, 16);
		waveFormat = FindFormatSwitchFormat(formats, lastPlayedFormat);
	}

	if (waveFormat == nullptr)
	{
		// Next, see if the stereo 44.1 kHz 16 bit is an option:
		std::vector<AudioFormat*> formats = outputEndpoint.GetFormats(2, 44100, 16);
		waveFormat = FindFormatSwitchFormat(formats, lastPlayedFormat);
	}

	if (waveFormat == nullptr)
	{
		// Next, see if the stereo 48 kHz is an option:
		std::vector<AudioFormat*> formats = outputEndpoint.GetFormats(2, 48000);
		waveFormat = FindFormatSwitchFormat(formats, lastPlayedFormat);
	}

	if (waveFormat == nullptr)
	{
		// Next, see if the stereo 44.1 kHz is an option:
		std::vector<AudioFormat*> formats = outputEndpoint.GetFormats(2, 44100);
		waveFormat = FindFormatSwitchFormat(formats, lastPlayedFormat);
	}

	if (waveFormat == nullptr)
	{
		// Next, see if the stereo is an option:
		std::vector<AudioFormat*> formats = outputEndpoint.GetFormats(2);
		waveFormat = FindFormatSwitchFormat(formats, lastPlayedFormat);
	}

	if (waveFormat == nullptr)
	{
		// This will probably only happen the measurements are coming back invalid due to a hardware configuraion issue.
		for (const AudioFormat& f : outputEndpoint.SupportedFormats)
		{
			if (lastPlayedFormat != &f && std::find(FailedFormats.begin(), FailedFormats.end(), &f) == FailedFormats.end())
			{
				waveFormat = f.WaveFormat;
				break;
			}
		}
	}

	if (waveFormat == nullptr)
	{
		// it seems like maybe this endpoint only actually outputs audio for a single format?
		return false;
	}

	GeneratedSamples* generatedSamples = new GeneratedSamples(waveFormat, GeneratedSamples::WaveType::FormatSwitch);
	WasapiOutput* output = new WasapiOutput(outputEndpoint, false, true, generatedSamples->samples, generatedSamples->samplesLength, waveFormat);
	std::thread outputThread{ [output] { output->StartPlayback(); } };
	outputThread.join();
	delete output;

	return true;
}

WAVEFORMATEX* TestManager::FindFormatSwitchFormat(std::vector<AudioFormat*> formats, AudioFormat* lastPlayedFormat)
{
	if (formats.size() > 0)
	{
		for (AudioFormat* f : formats)
		{
			if (lastPlayedFormat != f && std::find(FailedFormats.begin(), FailedFormats.end(), f) == FailedFormats.end())
			{
				return f->WaveFormat;
			}
		}
	}
	return nullptr;
}

void TestManager::PopulateSummaryResults()
{
	// Find the three default formats and see if they were measured.
	// If they were, add them to the SummaryResults vector
	// If they weren't measured, look through all the AveragedResults and see if there are any that are the same format,
	// but without the same channel masks -- if any are found, add them to the SummaryResults vector
	bool foundStereo = false;
	bool foundFiveOne = false;
	bool foundSevenOne = false;
	for (AveragedResult avgResult : AveragedResults)
	{
		if (avgResult.Format->DefaultSelection)
		{
			SummaryResults.push_back(avgResult);
			if (avgResult.Format->WaveFormat->nChannels == 2)
			{
				foundStereo = true;
			}
			else if (avgResult.Format->WaveFormat->nChannels == 6)
			{
				foundFiveOne = true;
			}
			else if (avgResult.Format->WaveFormat->nChannels == 8)
			{
				foundSevenOne = true;
			}
		}
	}

	for (AveragedResult avgResult : AveragedResults)
	{
		if (!foundStereo
			&& avgResult.Format->WaveFormat->nChannels == 2
			&& avgResult.Format->WaveFormat->nSamplesPerSec == 48000
			&& avgResult.Format->WaveFormat->wBitsPerSample == 16)
		{
			SummaryResults.push_back(avgResult);
			foundStereo = true;
		}
		if (!foundFiveOne
			&& avgResult.Format->WaveFormat->nChannels == 6
			&& avgResult.Format->WaveFormat->nSamplesPerSec == 48000
			&& avgResult.Format->WaveFormat->wBitsPerSample == 16)
		{
			SummaryResults.push_back(avgResult);
			foundFiveOne = true;
		}
		if (!foundSevenOne
			&& avgResult.Format->WaveFormat->nChannels == 8
			&& avgResult.Format->WaveFormat->nSamplesPerSec == 48000
			&& avgResult.Format->WaveFormat->wBitsPerSample == 16)
		{
			SummaryResults.push_back(avgResult);
			foundSevenOne = true;
		}
	}
}
