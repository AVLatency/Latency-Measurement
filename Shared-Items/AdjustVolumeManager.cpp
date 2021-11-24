#include "AdjustVolumeManager.h"
#include <WinBase.h>


AdjustVolumeManager::AdjustVolumeManager(const AudioEndpoint& inputEndpoint)
{
	SetThreadExecutionState(ES_DISPLAY_REQUIRED); // Prevent display from turning off while running this tool.
	working = true;

	input = new WasapiInput(inputEndpoint, true, recordBufferDurationInSeconds);
	inputThread = new std::thread([this] { input->StartRecording(); });

	output = new WasapiOutput();
	outputThread = new std::thread([this] { /* TODO: output->StartPlayback(); */});
}

AdjustVolumeManager::~AdjustVolumeManager()
{
	delete input;
	delete output;

	delete inputThread;
	delete outputThread;

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
	// TODO: output->StopPlayback();
}
