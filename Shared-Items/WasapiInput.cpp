#include "WasapiInput.h"



WasapiInput::WasapiInput(bool loop, double bufferDurationInSeconds)
{
    this->loop = loop;
    this->bufferDurationInSeconds = bufferDurationInSeconds;
}

WasapiInput::~WasapiInput()
{
    if (recordingBuffer1 != nullptr)
    {
        delete[] recordingBuffer1;
    }
    if (recordingBuffer2 != nullptr)
    {
        delete[] recordingBuffer2;
    }
}

void WasapiInput::StartRecording()
{
    recordingInProgress = true;
}

void WasapiInput::StopRecording()
{
    stopRequested = true;
}