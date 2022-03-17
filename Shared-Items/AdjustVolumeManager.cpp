#include "AdjustVolumeManager.h"
#include <WinBase.h>
// for M_PI:
#define _USE_MATH_DEFINES
#include <math.h>
#include "TestConfiguration.h"


#define SAFE_RELEASE(punk)  \
              if ((punk) != NULL)  \
                { (punk)->Release(); (punk) = NULL; }

AdjustVolumeManager::AdjustVolumeManager(const AudioEndpoint& outputEndpoint, const AudioEndpoint& inputEndpoint)
{
	SetThreadExecutionState(ES_DISPLAY_REQUIRED); // Prevent display from turning off while running this tool.
	working = true;

	WAVEFORMATEX* waveFormat = GetWaveFormat(outputEndpoint);

	if (waveFormat != NULL)
	{
		generatedSamples = new GeneratedSamples(waveFormat, GeneratedSamples::WaveType::VolumeAdjustment);

		output = new WasapiOutput(outputEndpoint, true, true, generatedSamples->samples, generatedSamples->samplesLength, waveFormat);
		outputThread = new std::thread([this] { output->StartPlayback(); });

		input = new WasapiInput(inputEndpoint, true, recordBufferDurationInSeconds);
		inputThread = new std::thread([this] { input->StartRecording(); });
	}
	else
	{
		throw "Could not find a suitable wave format for adjusting volume."; // TODO: error handling
	}
}

AdjustVolumeManager::~AdjustVolumeManager()
{
	delete output;
	delete input;

	delete outputThread;
	delete inputThread;

	SafeResetVolumeAnalysis(LeftVolumeAnalysis);
	SafeResetVolumeAnalysis(RightVolumeAnalysis);

	if (generatedSamples != nullptr)
	{
		delete generatedSamples;
	}
}

void AdjustVolumeManager::SafeResetVolumeAnalysis(VolumeAnalysis& analysis)
{
	analysis.PeakValue = 0;
	if (analysis.TickMonitorSamples != nullptr)
	{
		delete[] analysis.TickMonitorSamples;
		analysis.TickMonitorSamples = nullptr;
	}
	analysis.TickMonitorSamplesLength = 0;
	if (analysis.FullMonitorSamples != nullptr)
	{
		delete[] analysis.FullMonitorSamples;
		analysis.FullMonitorSamples = nullptr;
	}
	analysis.FullMonitorSamplesLength = 0;
	analysis.TickPosition = 0;
	analysis.MaxPlotValue = 1;
	analysis.AutoThreshold = 0.1;
	analysis.Grade = PeakLevelGrade::Quiet;
}

/// <summary>
/// Attempts to find a 2 channel, 48 kHz, 16-bit PCM wave format that the driver supports in exclusive mode.
/// </summary>
WAVEFORMATEX* AdjustVolumeManager::GetWaveFormat(const AudioEndpoint& endpoint)
{
	WAVEFORMATEX* result = NULL;

	const IID IID_IAudioClient = __uuidof(IAudioClient);
	IAudioClient* pAudioClient = NULL;
	HRESULT hr = endpoint.Device->Activate(IID_IAudioClient, CLSCTX_ALL, NULL, (void**)&pAudioClient);
	if (!FAILED(hr))
	{
		WAVEFORMATEXTENSIBLE* extensibleFormat = new WAVEFORMATEXTENSIBLE();
		memset(extensibleFormat, 0, sizeof(WAVEFORMATEXTENSIBLE));
		extensibleFormat->Format.wFormatTag = WAVE_FORMAT_EXTENSIBLE;
		extensibleFormat->Format.nChannels = 2;
		extensibleFormat->Format.nSamplesPerSec = 48000;
		SetBitsPerSample(&extensibleFormat->Format, 16);
		extensibleFormat->Format.cbSize = 22;
		extensibleFormat->Samples.wValidBitsPerSample = (WORD)extensibleFormat->Format.wBitsPerSample;
		extensibleFormat->dwChannelMask = 0;
		extensibleFormat->SubFormat = KSDATAFORMAT_SUBTYPE_PCM;

		WAVEFORMATEX* closestMatch = NULL;
		hr = pAudioClient->IsFormatSupported(AUDCLNT_SHAREMODE_EXCLUSIVE, (WAVEFORMATEX*)extensibleFormat, &closestMatch);
		if (hr == S_OK)
		{
			// this format is supported!
			result = (WAVEFORMATEX*)extensibleFormat; // TODO: memory management for extensibleFormat, which is now leaking.
		}
		else if (closestMatch != NULL)
		{
			result = closestMatch; // TODO: memory management for closestMatch, which is now leaking.
		}
		else
		{
			extensibleFormat->dwChannelMask = SPEAKER_FRONT_LEFT | SPEAKER_FRONT_RIGHT;
			hr = pAudioClient->IsFormatSupported(AUDCLNT_SHAREMODE_EXCLUSIVE, (WAVEFORMATEX*)extensibleFormat, &closestMatch);
			if (hr == S_OK)
			{
				// this format is supported!
				result = (WAVEFORMATEX*)extensibleFormat; // TODO: memory management for extensibleFormat, which is now leaking.
			}
			else if (closestMatch != NULL)
			{
				result = closestMatch; // TODO: memory management for closestMatch, which is now leaking.
			}
			else
			{
				WAVEFORMATEX* exFormat = new WAVEFORMATEX();
				memset(exFormat, 0, sizeof(WAVEFORMATEX));
				exFormat->wFormatTag = WAVE_FORMAT_PCM;
				exFormat->nChannels = 2;
				exFormat->nSamplesPerSec = 48000;
				SetBitsPerSample(exFormat, 16);
				exFormat->cbSize = 0;

				hr = pAudioClient->IsFormatSupported(AUDCLNT_SHAREMODE_EXCLUSIVE, exFormat, &closestMatch);
				if (hr == S_OK)
				{
					// this format is supported!
					result = exFormat; // TODO: memory management for exFormat, which is now leaking.
				}
				else if (closestMatch != NULL)
				{
					result = closestMatch; // TODO: memory management for closestMatch, which is now leaking.
				}
				else
				{
					// Couldn't find a suitable format. TODO: error handling.
				}

				if (result != exFormat)
				{
					delete exFormat;
				}
			}
		}
		if (result != (WAVEFORMATEX*)extensibleFormat)
		{
			delete extensibleFormat;
		}
	}
	SAFE_RELEASE(pAudioClient)
	return result;
}

void AdjustVolumeManager::SetBitsPerSample(WAVEFORMATEX* wfx, WORD bitsPerSample)
{
	wfx->wBitsPerSample = bitsPerSample;
	wfx->nBlockAlign = wfx->wBitsPerSample / 8 * wfx->nChannels;
	wfx->nAvgBytesPerSec = wfx->nBlockAlign * wfx->nSamplesPerSec;
}

void AdjustVolumeManager::Tick()
{
	if (working)
	{
		if (!input->recordingInProgress && !output->playbackInProgress)
		{
			inputThread->join();
			outputThread->join();
			working = false;
			SetThreadExecutionState(0); // Reset prevent display from turning off while running this tool.
		}
		else
		{
			// Read buffers and get things in a nice state for the GUI
			if (lastBufferFlipWasTo1 && !input->recordingToBuffer1)
			{
				// just flipped to buffer 2; Buffer 1 is now ready.
				lastBufferFlipWasTo1 = !lastBufferFlipWasTo1;
				CopyBuffer(input->recordingBuffer1, input->recordingBufferLength);
			}
			else if (!lastBufferFlipWasTo1 && input->recordingToBuffer1)
			{
				// just flipped to buffer 1; Buffer 2 is now ready.
				lastBufferFlipWasTo1 = !lastBufferFlipWasTo1;
				CopyBuffer(input->recordingBuffer2, input->recordingBufferLength);
			}
		}
	}
}

void AdjustVolumeManager::CopyBuffer(float* sourceBuffer, int sourceBufferLength)
{
	int perChannelSourceBufferLength = sourceBufferLength / input->recordedAudioNumChannels;
	// Making math a lot simpler by copying to one sample array per recorded channel:
	float* leftSourceBuffer = new float[perChannelSourceBufferLength];
	float* rightSourceBuffer = new float[perChannelSourceBufferLength];
	int channelIndex = 0;
	for (int i = 0; i < sourceBufferLength; i += input->recordedAudioNumChannels)
	{
		leftSourceBuffer[channelIndex] = sourceBuffer[i];
		rightSourceBuffer[channelIndex] = sourceBuffer[i + 1];
		channelIndex++;
	}
	int sampleRate = input->waveFormat.Format.nSamplesPerSec;
	AnalyseChannel(LeftVolumeAnalysis, leftSourceBuffer, perChannelSourceBufferLength);
	AnalyseChannel(RightVolumeAnalysis, rightSourceBuffer, perChannelSourceBufferLength);

	delete[] leftSourceBuffer;
	delete[] rightSourceBuffer;
}

void AdjustVolumeManager::AnalyseChannel(VolumeAnalysis& analysis, float* recordedSamples, int recordedSamplesLength)
{
	// Get some info about the ticks that were generated and create the sample buffers
	int outputSampleRate = generatedSamples->WaveFormat->nSamplesPerSec;
	int inputSampleRate = input->waveFormat.Format.nSamplesPerSec;
	int expectedTickFrequency = generatedSamples->GetTickFrequency(outputSampleRate);
	int tickDurationInSamples = ceil((float)inputSampleRate / expectedTickFrequency);
	int halfTickDurationInSamples = ceil((float)(inputSampleRate / 2) / expectedTickFrequency);

	SafeResetVolumeAnalysis(analysis);

	for (int i = 0; i < recordedSamplesLength; i++)
	{
		if (abs(recordedSamples[i]) > analysis.PeakValue)
		{
			analysis.PeakValue = abs(recordedSamples[i]);
		}
	}

	float* allEdges = new float[recordedSamplesLength];
	int largestEdgeIndex = 0;
	float largestEdge = 0;
	for (int i = 0; i < recordedSamplesLength; i++)
	{
		float highestMagnitude = 0;
		// The highest change in magnitude for a tick will occur over halfTickDurationInSamples, give or take
		// It's possible that more change happens over a slightly longer period of time, but this is not important
		// because the bulk of the change will still happen over this time, which will cause it to exceed the
		// TestConfiguration::DetectionThreshold, which is all that matters.
		for (int j = i; j < recordedSamplesLength && j - i < halfTickDurationInSamples; j++)
		{
			float thisMagnitude = abs(recordedSamples[i] - recordedSamples[j]);
			if (thisMagnitude > highestMagnitude)
			{
				highestMagnitude = thisMagnitude;
			}
		}
		allEdges[i] = highestMagnitude;

		if (highestMagnitude > largestEdge)
		{
			largestEdgeIndex = i;
			largestEdge = highestMagnitude;
		}
	}
	analysis.MaxPlotValue = allEdges[largestEdgeIndex];

	// Here's a story that describes the worst case scenario for a tick:
	// - Largest edge is from high peak to low peak, but doubled because of echo
	// - This results in a value of 1
	// - The leading edge should still be detected. Without the echo, it's edge
	//   magnitude would be 0.5 of the largest edge, but becaues of the echo
	//   it's actually only 0.25 of the largest edge
	// - Worst-case sampling of this leading edge would only reveal half of its
	//   magnitude, which brings it down to 0.125 of the largest edge.
	// - To give just a tad more wiggle-room, I've chosen to round down to 12%
	//   of largest edge:
	analysis.AutoThreshold = allEdges[largestEdgeIndex] * .12f;

	// TODO: tick monitor should act like full monitor to handle higher sample rates.

	// Tick monitor
	analysis.TickMonitorSamplesLength = TickMonitorCycles * tickDurationInSamples;
	analysis.TickMonitorSamples = new float[analysis.TickMonitorSamplesLength];
	analysis.TickMonitorSampleRate = inputSampleRate;
	int tickMonitorStart = largestEdgeIndex - (analysis.TickMonitorSamplesLength / 3);
	if (tickMonitorStart < 0)
	{
		tickMonitorStart = 0;
	}
	if (tickMonitorStart + analysis.TickMonitorSamplesLength > recordedSamplesLength)
	{
		tickMonitorStart = recordedSamplesLength - analysis.TickMonitorSamplesLength;
	}
	for (int i = tickMonitorStart; i < tickMonitorStart + analysis.TickMonitorSamplesLength; i++)
	{
		analysis.TickMonitorSamples[i - tickMonitorStart] = allEdges[i];
	}

	// TODO: Full monitor should scale the divider based on input sample rate.

	// Full monitor
	int sourceSampleCount = ceil(inputSampleRate * 0.08); // Generated samples is 0.10 seconds, 0.02 seconds is the threshold used for looking for echos after the tick.
	int fullMonitorStart = largestEdgeIndex - (sourceSampleCount / 2);
	if (fullMonitorStart < 0)
	{
		fullMonitorStart = 0;
	}
	if (fullMonitorStart + sourceSampleCount > recordedSamplesLength)
	{
		fullMonitorStart = recordedSamplesLength - sourceSampleCount;
	}

	analysis.FullMonitorSamplesLength = sourceSampleCount / FullMonitorDivisions + (sourceSampleCount % FullMonitorDivisions == 0 ? 0 : 1);
	analysis.FullMonitorSamples = new float[analysis.FullMonitorSamplesLength];
	analysis.FullMonitorSampleRate = inputSampleRate / FullMonitorDivisions;
	for (int i = fullMonitorStart; i < fullMonitorStart + sourceSampleCount; i+= FullMonitorDivisions)
	{
		bool foundTick = false;

		float largestEdge = 0;
		for (int j = 0; j < FullMonitorDivisions && i + j < recordedSamplesLength; j++)
		{
			if (allEdges[i + j] > largestEdge)
			{
				largestEdge = allEdges[i + j];
			}
			if (i + j == largestEdgeIndex)
			{
				foundTick = true;
			}
		}

		if ((i - fullMonitorStart) / FullMonitorDivisions < analysis.FullMonitorSamplesLength)
		{
			analysis.FullMonitorSamples[(i - fullMonitorStart) / FullMonitorDivisions] = largestEdge;
		}
		else
		{
			// this should never happen
			throw "My math was wrong on array indices";
		}

		if (foundTick)
		{
			analysis.TickPosition = (i - fullMonitorStart) / FullMonitorDivisions;
		}
	}

	analysis.Grade = PeakLevelGrade::Good; // TODO: look for other ticks. if there are any, then it's quiet. Or, if it lines up with other channel, it means that it's crosstalk.

	delete[] allEdges;
}

void AdjustVolumeManager::Stop()
{
	input->StopRecording();
	output->StopPlayback();
}