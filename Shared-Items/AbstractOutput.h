#pragma once
#include <atomic>

class AbstractOutput
{
public:
	std::atomic<bool> playbackInProgress = false;

	std::atomic<bool> Mute = false;

	virtual void StartPlayback() = 0;
	virtual void StopPlayback() = 0;
};
