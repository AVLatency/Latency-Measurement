#include "AdjustVolumeManager.h"
#include <WinBase.h>


AdjustVolumeManager::AdjustVolumeManager(const AudioEndpoint& outputEndpoint, const AudioEndpoint& inputEndpoint)
{
	SetThreadExecutionState(ES_DISPLAY_REQUIRED); // Prevent display from turning off while running this tool.
	working = true;

	int samplesLength = 1000;
	float* samples = new float[samplesLength];
	for (int i = 0; i < samplesLength; i++)
	{
		samples[i] = sin(i / 200);
	}

	output = new WasapiOutput(outputEndpoint, true, samples, samplesLength, NULL); // TODO: provide wave format!
	outputThread = new std::thread([this] { output->StartPlayback(); });

	input = new WasapiInput(inputEndpoint, true, recordBufferDurationInSeconds);
	inputThread = new std::thread([this] { input->StartRecording(); });
}

AdjustVolumeManager::~AdjustVolumeManager()
{
	delete output;
	delete input;

	delete outputThread;
	delete inputThread;

	if (lastInputBufferCopy != nullptr)
	{
		delete[] lastInputBufferCopy;
	}
	if (lastRecordedSegment != nullptr)
	{
		delete[] lastRecordedSegment;
	}
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
	// TODO: Get rid of all of this and do a direct analysis on the sourceBuffer to find the
	// loudest part of each channnel of the wave and then copy them, centered around this loudest
	// part. Because it's centered on the loudest part, this point needs to be be a certain
	// distance from the beginning or end, based on the tick frequency.

	if (lastInputBufferCopy == nullptr)
	{
		lastInputBufferCopy = new float[sourceBufferLength];
	}
	std::copy(sourceBuffer, &sourceBuffer[sourceBufferLength - 1], lastInputBufferCopy);

	if (lastRecordedSegment != nullptr)
	{
		delete[] lastRecordedSegment;
	}

	//Temp test stuff:
	lastRecordedSegmentLength = 2 * 1000;
	lastRecordedSegment = new float[lastRecordedSegmentLength];
	std::copy(lastInputBufferCopy, &lastInputBufferCopy[lastRecordedSegmentLength - 1], lastRecordedSegment);
}

void AdjustVolumeManager::Stop()
{
	input->StopRecording();
	output->StopPlayback();
}
