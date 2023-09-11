#include "TestManager.h"
#include <thread>
#include <Mmdeviceapi.h>
#include "AudioGraphOutput.h"
#include "WasapiOutput.h"
#include "WasapiInput.h"
#include "GeneratedSamples.h"
#include "RecordingAnalyzer.h"
#include "StringHelper.h"
#include <format>
#include "WavHelper.h"
#include "ExternalMediaPlayerOutput.h"

TestManager::TestManager(AudioEndpoint* outputEndpoint,
	AudioEndpoint* outputEndpoint2,
	AudioEndpoint* inputEndpoint,
	std::vector<SupportedAudioFormat*> selectedFormats,
	std::string fileString,
	std::string appDirectory,
	IResultsWriter& resultsWriter,
	OutputOffsetProfile* currentProfile,
	DacLatencyProfile* referenceDacLatency)
	: outputEndpoint(outputEndpoint),
	outputEndpoint2(outputEndpoint2),
	inputEndpoint(inputEndpoint),
	SelectedFormats(selectedFormats),
	AppDirectory(appDirectory),
	resultsWriter(resultsWriter),
	Time(time(0)),
	outputOffsetProfile(currentProfile),
	referenceDacLatency(referenceDacLatency)
{
	TestFileString = std::format("{}~{}~{}", StringHelper::GetTimeString(Time, true), OutputOffsetProfile::OutputTypeNameFileSafe(currentProfile->OutType) , fileString);

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

	TotalPasses = (TestConfiguration::NumMeasurements > 0
		&& TestConfiguration::MeasureAverageLatency) ? TestConfiguration::NumMeasurements : 1;
	TotalRecordingsPerPass = SelectedFormats.size() > 0 ? SelectedFormats.size() : 1;

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

	for (int i = 0; i < TotalPasses; i++)
	{
		PassCount = i;
		RecordingCount = 0;
		SupportedAudioFormat* lastPlayedFormat = nullptr;

		// for determining if a format switch tone is necessary:
		int firstSampleRate = -1;
		bool switchedSampleRates = false;

		for (SupportedAudioFormat* audioFormat : SelectedFormats)
		{
			if (!StopRequested.load(std::memory_order_acquire) && std::find(FailedFormats.begin(), FailedFormats.end(), audioFormat) == FailedFormats.end())
			{
				bool valid = PerformRecording(audioFormat);
				if (!valid)
				{
					FailedFormats.push_back(audioFormat);
				}
				RecordingCount++;
				lastPlayedFormat = audioFormat;
				if (audioFormat != nullptr)
				{
					if (firstSampleRate == -1)
					{
						firstSampleRate = audioFormat->Format->GetSamplesPerSec();
					}
					else
					{
						if (firstSampleRate != audioFormat->Format->GetSamplesPerSec())
						{
							switchedSampleRates = true;
						}
					}
				}
				else
				{
					// we are playing the current audio format
				}
			}
		}
		TotalRecordingsPerPass = RecordingCount;

		// Inject a dummy format when we're down to a single recording to force the HDMI Audio Device to re-sync to a new signal format
		if (TestConfiguration::MeasureAverageLatency && !switchedSampleRates && !StopRequested.load(std::memory_order_acquire))
		{
			bool formatSwitchResult = PlayFormatSwitch(lastPlayedFormat);
			if (!formatSwitchResult)
			{
				// something has gone very wrong, so bail early since there's no point in doing the same test over and over without switching formats.
				StopRequested.store(true, std::memory_order_release);
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
				ShouldShowNegativeLatencyError.store(true, std::memory_order_release);
			}
		}
	}

	try
	{
		RecordingAnalyzer::SaveFinalResults(resultsWriter, AveragedResults, StringHelper::GetRootPath(AppDirectory), std::format("{}.csv", TestFileString));
	}
	catch (...)
	{
		ShouldShowFilesystemError.store(true, std::memory_order_release);
	}

	SetThreadExecutionState(0); // Reset prevent display from turning off while running this tool.

	IsFinished.store(true, std::memory_order_release);
}

bool TestManager::PerformRecording(SupportedAudioFormat* audioFormat)
{
	int sampleRate = audioFormat == nullptr ? AudioGraphOutput::CurrentWindowsSampleRate() : audioFormat->Format->GetSamplesPerSec();
	GeneratedSamples* generatedSamples = new GeneratedSamples(sampleRate, GeneratedSamples::WaveType::LatencyMeasurement);

	bool validResult = false;
	for (int i = 0; i < TestConfiguration::AttemptsBeforeFail && !StopRequested.load(std::memory_order_acquire); i++)
	{
		AbstractOutput* output;
		if (audioFormat == nullptr)
		{
			output = new AudioGraphOutput(false, true, generatedSamples->samples, generatedSamples->samplesLength);
		}
		else if (audioFormat->Format->type == AudioFormat::FormatType::WaveFormatEx)
		{
			output = new WasapiOutput(outputEndpoint, false, true, generatedSamples->samples, generatedSamples->samplesLength, audioFormat->Format->GetWaveFormat());
		}
		else if (audioFormat->Format->type == AudioFormat::FormatType::File)
		{
			output = new ExternalMediaPlayerOutput(audioFormat->Format->FileName, false);
		}
		std::thread outputThread{ [output] { output->StartPlayback(); } };

		AbstractOutput* output2 = nullptr;
		std::thread* output2Thread = nullptr;
		if (outputEndpoint2 != nullptr)
		{
			output2 = new WasapiOutput(outputEndpoint2, false, true, generatedSamples->samples, generatedSamples->samplesLength, audioFormat->Format->GetWaveFormat());
			output2Thread = new std::thread ([output2] { output2->StartPlayback(); });
		}

		float recordingDuration = generatedSamples->TestWaveDurationInSeconds();
		if (audioFormat->Format->type == AudioFormat::FormatType::File)
		{
			recordingDuration = audioFormat->Format->FileDuration + TestConfiguration::AdditionalRecordingTimeForFilePlayback;
		}
		WasapiInput* input = new WasapiInput(inputEndpoint, false, recordingDuration);
		std::thread inputThread{ [input] { input->StartRecording(); } };

		outputThread.join();
		inputThread.join();
		if (output2Thread != nullptr)
		{
			output2Thread->join();
		}

		RecordingResult result = RecordingAnalyzer::AnalyzeRecording(*generatedSamples, *input, audioFormat, outputOffsetProfile, referenceDacLatency);
		Results.push_back(result);
		
		if (TestConfiguration::SaveIndividualWavFiles)
		{
			std::string audioFormatString = audioFormat == nullptr ? AudioFormat::GetCurrentWinAudioFormatString() : audioFormat->Format->FormatString;
			std::string recordingFolder = format("{}/{}/{}", StringHelper::GetRootPath(AppDirectory), TestFileString, audioFormatString);
			try
			{
				WavHelper::SaveWavFile(
					recordingFolder,
					std::format("{}.wav", result.GUID),
					input->recordingBuffer1,
					input->recordingBufferLength,
					input->waveFormat.Format.nSamplesPerSec,
					input->waveFormat.Format.nChannels,
					input->waveFormat.Format.nChannels,
					false,
					1.0f);
			}
			catch (...)
			{
				ShouldShowFilesystemError.store(true, std::memory_order_release);
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
				ShouldShowFilesystemError.store(true, std::memory_order_release);
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

bool TestManager::PlayFormatSwitch(SupportedAudioFormat* lastPlayedFormat)
{
	WAVEFORMATEX* waveFormat = nullptr;

	// First, see if the stereo 48 kHz 16 bit is an option:
	{
		std::vector<SupportedAudioFormat*> formats = outputEndpoint->GetWaveFormatExFormats(2, 48000, 16);
		waveFormat = FindFormatSwitchFormat(formats, lastPlayedFormat);
	}

	if (waveFormat == nullptr)
	{
		// Next, see if the stereo 44.1 kHz 16 bit is an option:
		std::vector<SupportedAudioFormat*> formats = outputEndpoint->GetWaveFormatExFormats(2, 44100, 16);
		waveFormat = FindFormatSwitchFormat(formats, lastPlayedFormat);
	}

	if (waveFormat == nullptr)
	{
		// Next, see if the stereo 48 kHz is an option:
		std::vector<SupportedAudioFormat*> formats = outputEndpoint->GetWaveFormatExFormats(2, 48000);
		waveFormat = FindFormatSwitchFormat(formats, lastPlayedFormat);
	}

	if (waveFormat == nullptr)
	{
		// Next, see if the stereo 44.1 kHz is an option:
		std::vector<SupportedAudioFormat*> formats = outputEndpoint->GetWaveFormatExFormats(2, 44100);
		waveFormat = FindFormatSwitchFormat(formats, lastPlayedFormat);
	}

	if (waveFormat == nullptr)
	{
		// Next, see if the stereo is an option:
		std::vector<SupportedAudioFormat*> formats = outputEndpoint->GetWaveFormatExFormats(2);
		waveFormat = FindFormatSwitchFormat(formats, lastPlayedFormat);
	}

	if (waveFormat == nullptr)
	{
		// This will probably only happen the measurements are coming back invalid due to a hardware configuraion issue.
		for (const SupportedAudioFormat* f : outputEndpoint->SupportedFormats)
		{
			if (lastPlayedFormat != f && std::find(FailedFormats.begin(), FailedFormats.end(), f) == FailedFormats.end())
			{
				waveFormat = f->Format->GetWaveFormat();
				break;
			}
		}
	}

	if (waveFormat == nullptr)
	{
		// it seems like maybe this endpoint only actually outputs audio for a single format?
		return false;
	}

	GeneratedSamples* generatedSamples = new GeneratedSamples(waveFormat->nSamplesPerSec, GeneratedSamples::WaveType::FormatSwitch);
	AbstractOutput* output = new WasapiOutput(outputEndpoint, false, true, generatedSamples->samples, generatedSamples->samplesLength, waveFormat);
	std::thread outputThread{ [output] { output->StartPlayback(); } };
	outputThread.join();
	delete output;
	delete generatedSamples;

	return true;
}

WAVEFORMATEX* TestManager::FindFormatSwitchFormat(std::vector<SupportedAudioFormat*> formats, SupportedAudioFormat* lastPlayedFormat)
{
	if (formats.size() > 0)
	{
		for (SupportedAudioFormat* f : formats)
		{
			if (lastPlayedFormat != f && std::find(FailedFormats.begin(), FailedFormats.end(), f) == FailedFormats.end())
			{
				if (f->Format->type == AudioFormat::FormatType::File)
				{
#if _DEBUG
					throw "TestManager::FindFormatSwitchFormat formats vector incrrectly contains AudioFormat::FormatType::File types";
#endif
				}
				else
				{
					return f->Format->GetWaveFormat();
				}
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
		if (avgResult.Format == nullptr)
		{
			// Current windows audio format
			SummaryResults.push_back(avgResult);
		}
		else
		{
			if (avgResult.Format->DefaultSelection
				&& avgResult.Format->Format->type == AudioFormat::FormatType::WaveFormatEx
				&& AudioFormat::GetFormatID(avgResult.Format->Format->GetWaveFormat()) == WAVE_FORMAT_PCM)
			{
				SummaryResults.push_back(avgResult);
				if (avgResult.Format->Format->GetNumChannels() == 2)
				{
					foundStereo = true;
				}
				else if (avgResult.Format->Format->GetNumChannels() == 6)
				{
					foundFiveOne = true;
				}
				else if (avgResult.Format->Format->GetNumChannels() == 8)
				{
					foundSevenOne = true;
				}
			}
		}
	}

	for (AveragedResult avgResult : AveragedResults)
	{
		if (avgResult.Format != nullptr
			&& avgResult.Format->Format->type == AudioFormat::FormatType::WaveFormatEx
			&& AudioFormat::GetFormatID(avgResult.Format->Format->GetWaveFormat()) == WAVE_FORMAT_PCM)
		{
			if (!foundStereo
				&&avgResult.Format->Format->GetNumChannels() == 2
				&& avgResult.Format->Format->GetSamplesPerSec() == 48000
				&& avgResult.Format->Format->GetBitsPerSample() == 16)
			{
				SummaryResults.push_back(avgResult);
				foundStereo = true;
			}
			if (!foundFiveOne
				&& avgResult.Format->Format->GetNumChannels() == 6
				&& avgResult.Format->Format->GetSamplesPerSec() == 48000
				&& avgResult.Format->Format->GetBitsPerSample() == 16)
			{
				SummaryResults.push_back(avgResult);
				foundFiveOne = true;
			}
			if (!foundSevenOne
				&& avgResult.Format->Format->GetNumChannels() == 8
				&& avgResult.Format->Format->GetSamplesPerSec() == 48000
				&& avgResult.Format->Format->GetBitsPerSample() == 16)
			{
				SummaryResults.push_back(avgResult);
				foundSevenOne = true;
			}
		}
	}
}
