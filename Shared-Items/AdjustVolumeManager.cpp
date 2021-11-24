#include "AdjustVolumeManager.h"
#include <WinBase.h>


AdjustVolumeManager::AdjustVolumeManager()
{
	SetThreadExecutionState(ES_DISPLAY_REQUIRED); // Prevent display from turning off while running this tool.
	working = true;

	input = new WasapiInput(true, 0.5);
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
}

void AdjustVolumeManager::Tick()
{
	if (working)
	{
		if (!input->recordingInProgress && !output->playbackInProgress)
		{
			outputThread->join();
			outputThread->join();
			working = false;
			SetThreadExecutionState(0); // Reset prevent display from turning off while running this tool.
		}
		else
		{
			// TODO: read buffers and get things in a nice state for the GUI
		}
	}
}

void AdjustVolumeManager::Stop()
{
	input->StopRecording();
	// TODO: output->StopPlayback();
}
