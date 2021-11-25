#include "AdjustVolumeManager.h"
#include <WinBase.h>


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

		output = new WasapiOutput(outputEndpoint, true, generatedSamples->samples, generatedSamples->samplesLength, waveFormat);
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

	SafeDeleteMonitorSamples();

	if (generatedSamples != nullptr)
	{
		delete generatedSamples;
	}
}

void AdjustVolumeManager::SafeDeleteMonitorSamples()
{
	if (leftChannelTickMonitorSamples != nullptr)
	{
		delete[] leftChannelTickMonitorSamples;
		leftChannelTickMonitorSamples = nullptr;
	}
	if (rightChannelTickMonitorSamples != nullptr)
	{
		delete[] rightChannelTickMonitorSamples;
		rightChannelTickMonitorSamples = nullptr;
	}
	if (rightChannelNormalizedTickMonitorSamples != nullptr)
	{
		delete[] rightChannelNormalizedTickMonitorSamples;
		rightChannelNormalizedTickMonitorSamples = nullptr;
	}
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

	// Get some info about the ticks that were generated and create the sample buffers
	int tickSamplesLength = input->waveFormat.Format.nSamplesPerSec / generatedSamples->GetTickFrequency(generatedSamples->WaveFormat->nSamplesPerSec);
	tickMonitorSamplesLength = 3 * tickSamplesLength;
	SafeDeleteMonitorSamples();
	leftChannelTickMonitorSamples = new float[tickMonitorSamplesLength];
	rightChannelTickMonitorSamples = new float[tickMonitorSamplesLength];
	rightChannelNormalizedTickMonitorSamples = new float[tickMonitorSamplesLength];

	// Find the peaks in the sourceBuffer, excluding some padding on the left and right of the source buffer:
	int padding = tickMonitorSamplesLength * 2;
	int leftMaxAbsIndex = padding;
	int rightMaxAbsIndex = padding;
	for (int i = padding; i < perChannelSourceBufferLength - padding; i++)
	{
		if (abs(leftSourceBuffer[i]) > abs(leftSourceBuffer[leftMaxAbsIndex]))
		{
			leftMaxAbsIndex = i;
		}
		if (abs(rightSourceBuffer[i]) > abs(rightSourceBuffer[rightMaxAbsIndex]))
		{
			rightMaxAbsIndex = i;
		}
	}

	// Find the middle of the detected ticks:
	// This math hackery depends on all indexing of sourceBuffer to be divisible by 8 to make sure
	// that we're always indexing the right channel.
	int leftChannelTickStart;
	if (abs(leftSourceBuffer[leftMaxAbsIndex + (tickSamplesLength / 2)]) > abs(leftSourceBuffer[leftMaxAbsIndex - (tickSamplesLength / 2)]))
	{
		// Middle of tick is to the right of MaxAbsIndex
		leftChannelTickStart = leftMaxAbsIndex - (tickSamplesLength / 4);
	}
	else
	{
		// Middle of tick is to the left of MaxAbsIndex
		leftChannelTickStart = leftMaxAbsIndex - (tickSamplesLength * 3 / 4);
	}
	int rightChannelTickStart;
	if (abs(rightSourceBuffer[rightMaxAbsIndex + (tickSamplesLength / 2)]) > abs(rightSourceBuffer[rightMaxAbsIndex - (tickSamplesLength / 2)]))
	{
		// Middle of tick is to the right of MaxAbsIndex
		rightChannelTickStart = rightMaxAbsIndex - (tickSamplesLength / 4);
	}
	else
	{
		// Middle of tick is to the left of MaxAbsIndex
		rightChannelTickStart = rightMaxAbsIndex - (tickSamplesLength * 3 / 4);
	}

	// Fill in the sample buffers
	for (int i = 0; i < tickMonitorSamplesLength; i++)
	{
		leftChannelTickMonitorSamples[i] = leftSourceBuffer[leftChannelTickStart - tickSamplesLength + i];
		float rightSample = rightSourceBuffer[rightChannelTickStart - tickSamplesLength + i];
		rightChannelTickMonitorSamples[i] = rightSample;
		float normalizedRightSample = rightSample / abs(rightSourceBuffer[rightMaxAbsIndex]);
		rightChannelNormalizedTickMonitorSamples[i] = normalizedRightSample;
	}

	delete[] leftSourceBuffer;
	delete[] rightSourceBuffer;
}

void AdjustVolumeManager::Stop()
{
	input->StopRecording();
	output->StopPlayback();
}
