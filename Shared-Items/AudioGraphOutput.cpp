#include "AudioGraphOutput.h"

AudioGraphOutput::AudioGraphOutput(bool loop, bool firstChannelOnly, float* audioSamples, int audioSamplesLength)
	: loop(loop), firstChannelOnly(firstChannelOnly), audioSamples(audioSamples), audioSamplesLength(audioSamplesLength)
{
}

AudioGraphOutput::~AudioGraphOutput()
{
}

void AudioGraphOutput::StartPlayback()
{
    // TODO
}

void AudioGraphOutput::StopPlayback()
{
	stopRequested = true;
}

bool AudioGraphOutput::FinishedPlayback(bool loopIfNeeded)
{
    bool endOfSamplesReached = sampleIndex >= audioSamplesLength;
    if (loop)
    {
        if (loopIfNeeded && endOfSamplesReached)
        {
            sampleIndex = 0;
        }
        return false;
    }
    else
    {
        return endOfSamplesReached;
    }
}

int AudioGraphOutput::CurrentWindowsSampleRate()
{
    return 4800; // TODO
}
