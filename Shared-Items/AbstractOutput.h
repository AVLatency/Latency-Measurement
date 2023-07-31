#pragma once
class AbstractOutput
{
public:
	bool playbackInProgress = false;

	bool Mute = false;

	virtual void StartPlayback() = 0;
	virtual void StopPlayback() = 0;
};
