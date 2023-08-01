#pragma once
#include "AbstractOutput.h"

// Note: I think this output class requires the user to have this installed:
// https://support.microsoft.com/en-us/topic/media-feature-pack-list-for-windows-n-editions-c1c6fffa-d052-8338-7a79-a4bb980a700a

class AudioGraphOutput : public AbstractOutput
{
public:
	///<param name="firstChannelOnly">If true, audio will only be output to the first channel. Otherwise audio will be output to all channels</param>
	///<param name="audioSamples">These provided samples must remain in memory so long as WasapiOutput might be reading them. They will not be deleted by WasapiOutput.</param>
	AudioGraphOutput(bool loop, bool firstChannelOnly, float* audioSamples, int audioSamplesLength);
	~AudioGraphOutput();

	void StartPlayback();
	void StopPlayback();

	static int CurrentWindowsSampleRate();

private:
	bool loop;
	bool firstChannelOnly;
	float* audioSamples;
	int audioSamplesLength;
	int sampleIndex = 0;

	bool stopRequested = false;

	bool FinishedPlayback(bool loopIfNeeded);
};
